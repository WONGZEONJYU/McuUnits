#ifndef XMEMORY_HPP
#define XMEMORY_HPP 1

#include <wglobal.hpp>
#include <mutex>
#include <memory>
#include <xatomic.hpp>
#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#endif

inline namespace mem {

    // 计算多维数组中单个元素的总数
    template<typename T>
    constexpr size_t calculate_total_elements() {
        if constexpr (std::is_array_v<T>) {
            constexpr auto current_extent{std::extent_v<T, 0>};
            return current_extent > 0
                ? current_extent * calculate_total_elements<std::remove_extent_t<T>>()
                : calculate_total_elements<std::remove_extent_t<T>>();
        } else {
            return 1;
        }
    }

    template<typename Tp_> struct XAllocator {

        using value_type = Tp_;

        constexpr XAllocator() = default;

        template<typename Other>
        constexpr XAllocator(XAllocator<Other> const &) {}

#if (defined(FREERTOS) || defined(USE_FREERTOS)) && defined(configSUPPORT_DYNAMIC_ALLOCATION)
    #if configSUPPORT_DYNAMIC_ALLOCATION > 0
        static constexpr value_type * allocate(std::size_t const count) noexcept
        { return static_cast<value_type*>( pvPortMalloc(count * sizeof(value_type)) ); }

        static constexpr void deallocate(value_type * const p, size_t) noexcept
        { vPortFree(p); }
    #endif
#else
        static value_type * allocate(std::size_t const count) noexcept
        { return static_cast<value_type*>( malloc(count * sizeof(value_type)) ); }

        static void deallocate(value_type * const p,size_t) noexcept
        { free(p); }
#endif
        friend constexpr bool operator==(const XAllocator &, const XAllocator &) noexcept
        { return true; }
        friend constexpr bool operator!=(const XAllocator &, const XAllocator &) noexcept
        { return false; }
    };

    template<typename Tp_> struct XDeleter {

        using type = Tp_;
        using value_type = type;

        template<typename > friend struct XDeleter;

        constexpr XDeleter() = default;

        constexpr ~XDeleter() = default;

        template<typename U> requires std::is_convertible_v<U * ,Tp_ *>
        constexpr XDeleter(XDeleter<U> const &) {}

        static constexpr void cleanup(value_type * const pointer) noexcept {
            static_assert(sizeof(value_type) > static_cast<std::size_t>(0)
                ,"value_type must be a complete type!");
#if (defined(FREERTOS) || defined(USE_FREERTOS)) && defined(configSUPPORT_DYNAMIC_ALLOCATION)
    #if configSUPPORT_DYNAMIC_ALLOCATION > 0
            vPortFree(pointer);
    #endif
#else
            free(pointer);
#endif
        }

        constexpr void operator()(value_type * const pointer) const noexcept
        { std::ranges::destroy_at(pointer); cleanup(pointer); }
    };

    template<typename Tp_> struct XDeleter<Tp_[]> {
    private:
        mutable XAtomicInteger<std::size_t> m_length {};

        template<typename > friend struct XDeleter;

        template<typename U>
        constexpr void copy_(U const & o) noexcept
        { m_length.storeRelease(o.m_length.loadAcquire()); }

    public:
        using type = Tp_;
        using value_type = std::remove_all_extents_t<type>;

        constexpr XDeleter() = default;

        constexpr ~XDeleter() = default;

        template<typename U> requires std::is_constructible_v<U(*)[],Tp_(*)[]>
        constexpr XDeleter(XDeleter<U[]> const & o) noexcept
        { copy_(o); }

        constexpr XDeleter(XDeleter const & o) noexcept
        { copy_(o); }

        template<typename U> requires std::is_constructible_v<U(*)[],Tp_(*)[]>
        constexpr XDeleter& operator=(XDeleter<U[]> const & o) noexcept
        { copy_(o);return *this; }

        constexpr XDeleter &operator=(XDeleter const & o) noexcept
        { copy_(o);return *this; }

        constexpr void setLength(std::size_t const length) const noexcept
        { m_length.storeRelease(length); }

        static constexpr void cleanup(void * const pointer) noexcept {
            static_assert(!std::is_void_v<value_type>
                ,"can't delete pointer to incomplete type");

            static_assert(sizeof(type) > static_cast<std::size_t>(0)
                ,"can't delete pointer to incomplete type!");

#if (defined(FREERTOS) || defined(USE_FREERTOS)) && defined(configSUPPORT_DYNAMIC_ALLOCATION)
    #if configSUPPORT_DYNAMIC_ALLOCATION > 0
            vPortFree(pointer);
    #endif
#else
            free(pointer);
#endif
        }

        template<typename Up_> requires std::is_constructible_v<Up_(*)[],Tp_(*)[]>
        constexpr void operator()(Up_ * const pointer) const noexcept {
            if constexpr (std::is_array_v<Up_>) {
                std::ranges::destroy_at(pointer);
            }else {
                std::ranges::destroy_n(pointer,m_length.loadAcquire());
            }
            cleanup(pointer);
        }
    };

    template<typename Tp_>
    using XUniquePtr = std::unique_ptr<Tp_, XDeleter< Tp_ > >;

    template<typename Tp_>
    using XSharedPtr = std::shared_ptr<Tp_>;

    template<typename T,typename ...Args> requires (std::extent_v<T> > 0)
    constexpr void makeUnique(Args && ...) noexcept = delete;

    template<typename Tp_,typename ...Args,typename Ret = XUniquePtr<Tp_>>
    requires ( std::negation_v< std::is_array<Tp_> > )
    constexpr auto makeUnique(Args && ...args) noexcept -> Ret {
        auto const rawPtr { static_cast<Tp_ *>(pvPortMalloc(sizeof(Tp_))) };
        if(!rawPtr) { return {}; }
        return XUniquePtr<Tp_>(std::ranges::construct_at(rawPtr, std::forward<Args>(args)...) , XDeleter<Tp_>{});
    }

    template<typename Tp_,typename Ret = XUniquePtr<Tp_> >
    requires ( std::is_array_v<Tp_> && !std::extent_v<Tp_> )
    constexpr auto makeUnique(std::size_t const n) noexcept -> Ret {
        using Up_ = std::remove_all_extents_t<Tp_>;

        auto constexpr elements{ calculate_total_elements< std::remove_extent_t<Tp_> >() };
        auto const needElements{ n * elements };

        auto const rawPtr { static_cast<std::add_pointer_t<Up_>>( pvPortMalloc(needElements * sizeof(Up_))) };
        if(!rawPtr) { return {}; }

        std::ranges::uninitialized_default_construct_n(rawPtr, needElements);

        Ret ret (reinterpret_cast<std::decay_t<Tp_>>(rawPtr),XDeleter<Tp_>{});

        ret.get_deleter().setLength(needElements);

        return ret;
    }

    template<typename Tp_,typename Ret = XSharedPtr<Tp_> ,typename ...Args>
    requires (std::negation_v<std::is_array<Tp_>>)
    constexpr auto makeShared(Args && ...args) noexcept -> Ret
    { return std::allocate_shared<Tp_>(XAllocator<Ret>{}, std::forward<Args>(args)...); }

    template<typename Tp_ ,typename Ret = XSharedPtr<Tp_> >
    requires (std::is_unbounded_array_v<Tp_>)
    constexpr auto makeShared(std::size_t const n) noexcept -> Ret
    { return std::allocate_shared<Tp_>(XAllocator<Tp_>{},n); }

    template<typename Tp_ ,typename Ret = XSharedPtr<Tp_> >
    requires (std::is_unbounded_array_v<Tp_>)
    constexpr auto makeShared(std::size_t const n ,std::remove_all_extents_t<Tp_> const & u) noexcept -> Ret
    { return std::allocate_shared<Tp_>(XAllocator<Tp_>{},n,u); }

    template<typename Tp_ ,typename Ret = XSharedPtr<Tp_> >
    requires (std::is_bounded_array_v<Tp_>)
    constexpr auto makeShared() noexcept -> Ret
    { return std::allocate_shared<Tp_>(XAllocator<Tp_>{}); }

    template<typename Tp_ ,typename Ret = XSharedPtr<Tp_> >
    requires (std::is_bounded_array_v<Tp_>)
    constexpr auto makeShared(std::remove_all_extents_t<Tp_> const & u) noexcept ->Ret
    { return std::allocate_shared<Tp_>(XAllocator<Tp_>{},u); }

    namespace XPrivate {

        template<typename Object>
        struct Has_X_TwoPhaseConstruction_CLASS_Macro {
        private:
            static_assert(std::is_object_v<Object>,"typename Object don't Object type");

            template<typename T>
            static constexpr char test( void (T::*)() ){return {};}

            static constexpr int test( void (Object::*)() ){return {};}
        public:
            enum { value = sizeof(test(&Object::checkFriendXTwoPhaseConstruction_)) == sizeof(int) };
        };

        template<typename Object>
        inline constexpr bool Has_X_TwoPhaseConstruction_CLASS_Macro_v { Has_X_TwoPhaseConstruction_CLASS_Macro<Object>::value };

        template<typename Object,typename ...Args>
        struct Has_construct_Func {
        private:
            static_assert(std::is_object_v<Object>,"typename Object don't Object type");
        #if __cplusplus >= 202002L
            template<typename O,typename ...A>
            static constexpr auto test(int) -> std::true_type
                requires ( ( sizeof( std::declval<O>().construct_( ( std::declval< std::decay_t< A > >() )... ) )
                    > static_cast<std::size_t>(0) ) )
            {return {} ;}
        #else
            #if 0 //只能二选一
                template<typename O,typename ...A>
                static constexpr auto test(int)
                    -> std::enable_if_t< ( sizeof(std::declval<O>().construct_( (std::declval< std::decay_t< A > >())...) )
                        > static_cast<std::size_t>(0) )
                        ,std::true_type >
            {return {};}
            #else
                template<typename O,typename ...A>
                static constexpr auto test(int)
                -> decltype( sizeof( std::declval<O>().construct_( (std::declval< std::decay_t< A > >())...) )
                    > static_cast<std::size_t>(0)
                        , std::true_type{} )
            {return {};}
            #endif
        #endif
            template<typename ...>
            static constexpr auto test(...) -> std::false_type {return {};}
        public:
            enum { value = decltype(test<Object,Args...>(0))::value };
        };

        template<typename ...Args>
        inline constexpr bool Has_construct_Func_v {Has_construct_Func<Args...>::value};

        template<typename Object,typename ...Args>
        struct is_private_mem_func {
            static_assert(std::is_object_v<Object>,"typename Object don't Object type");
        private:
        #if __cplusplus >= 202002L
            template<typename O,typename ...A>
            static constexpr auto test(int) -> std::false_type
                requires (
                    ( sizeof( std::declval<O>().construct_( std::declval< std::decay_t< A > >()...) ) > static_cast<std::size_t>(0) )
                        || std::is_same_v< decltype( std::declval<O>().construct_( std::declval< std::decay_t< A > >()...) ),void >
                )
            {return {} ;}
        #else
            #if 0 //只能二选一
                template<typename O,typename ...A>
                static constexpr auto test(int)
                    -> std::enable_if_t< std::is_same_v< decltype( std::declval<O>().construct_( (std::declval< std::decay_t< A > >())...) ) ,void >
                         || ( sizeof( std::declval<O>().construct_( (std::declval< std::decay_t< A > >())...) ) > static_cast<std::size_t>(0) )
                            ,std::false_type >
            {return {} ;}
            #else
                template<typename O,typename ...A>
                static constexpr auto test(int)
                    -> decltype( std::is_same_v< decltype(std::declval<O>().construct_((std::declval< std::decay_t< A > >())...)), void >
                        || ( sizeof( std::declval<O>().construct_( (std::declval< std::decay_t< A > >())... ) ) > static_cast<std::size_t>(0) )
                            ,std::false_type {} )
            {return {} ;}
            #endif
        #endif

            template<typename ...>
            static constexpr auto test(...) ->std::true_type {return {} ;}
        public:
            enum { value = decltype(test<Object,Args...>(0))::value };
        };

        template<typename ...Args>
        inline constexpr bool is_private_mem_func_v{ is_private_mem_func<Args...>::value };

        template<typename T, typename... Args>
        struct is_default_constructor_accessible {
        private:
            enum {
                result = std::disjunction_v< std::is_constructible< T, std::decay_t< Args >... >
                        ,std::is_nothrow_constructible< T ,std::decay_t<Args>... >
                        ,std::is_trivially_constructible< T ,std::decay_t<Args>... >
                >
            };

            template<typename > struct is_copy_move_constructor {
                enum { value = false };
            };

        #if __cplusplus >= 202002L
            template<typename ...AS> requires(sizeof...(AS) == 1)
            struct is_copy_move_constructor<std::tuple<AS...>> {
                using Tuple_ = std::tuple<AS...>;
                using First_ = std::tuple_element_t<0, Tuple_>;
                enum {
                    value = std::disjunction_v<
                        std::is_same<First_, T &>,
                        std::is_same<First_, const T &>,
                        std::is_same<First_, T &&>,
                        std::is_same<First_, const T &&>
                    >
                };
            };
        #else
            template<> struct is_copy_move_constructor<std::tuple<>> {
                enum { value = false };
            };
            template<typename ...AS>
            struct is_copy_move_constructor<std::tuple<AS...>> {
            private:
                using Tuple_ = std::tuple<AS...>;
                using First_ = std::tuple_element_t<0, Tuple_>;
            public:
                enum {
                    value = std::disjunction_v<
                        std::is_same<First_, T &>,
                        std::is_same<First_, const T &>,
                        std::is_same<First_, T &&>,
                        std::is_same<First_, const T &&>>
                };
            };
        #endif

        public:
            enum {value = result && !is_copy_move_constructor<std::tuple<Args...>>::value};
        };

        template<typename ...Args>
        inline constexpr bool is_default_constructor_accessible_v { is_default_constructor_accessible<Args...>::value };

        template<typename Object>
        struct is_destructor_private {
        private:
            static_assert(std::is_object_v<Object>,"typename Object don't Object type");

            template<typename O>
            static constexpr auto test(int) -> decltype(std::declval<O>().~O(),std::false_type{})
            {return {};}

            template<typename >
            static constexpr auto test(...) -> std::true_type
            {return {};}

        public:
            enum {value = decltype(test<Object>(0))::value };
        };

        template<typename Object>
        inline constexpr bool is_destructor_private_v { is_destructor_private<Object>::value };

    #define STATIC_ASSERT_P \
        static_assert( std::is_object_v< Object >,"typename Object is not an class or struct" ); \
                            \
        static_assert( std::is_final_v< Object > ,"Object must be a final class" ); \
                            \
        static_assert( XPrivate::Has_X_TwoPhaseConstruction_CLASS_Macro_v< Object > \
                ,"No X_HELPER_CLASS in the class!" ); \
                            \
        static_assert( XPrivate::Has_construct_Func_v< Object ,std::decay_t<Args2>... > \
                ,"bool Object::construct_(...) non static member function absent!" ); \
                            \
        static_assert( XPrivate::is_private_mem_func_v< Object ,std::decay_t<Args2>... > \
                ,"bool Object::construct_(...) must be a private non static member function!" ); \
                            \
        static_assert( !XPrivate::is_default_constructor_accessible_v< Object ,std::decay_t< Args1 >... > \
                ,"The Object (...) constructor (non copy and non move) must be a private member function!" );
    } //namespace XPrivate;

    template<typename Tp_, typename = XAllocator< std::decay_t< std::remove_cvref_t<Tp_> > > >
    class XTwoPhaseConstruction;

    template<typename Tp_, typename = XAllocator< std::decay_t< std::remove_cvref_t< Tp_ > > > >
    class XSingleton;

    template<typename ...Args>
    using Parameter = std::tuple<Args...>;

    template<typename Tp_, typename Alloc_>
    class XTwoPhaseConstruction {

        using Object_t = std::decay_t< std::remove_cvref_t<Tp_> >;
        using Allocator = Alloc_;

        static_assert(std::negation_v< std::is_pointer< Object_t > >,"Tp_ Cannot be pointer type");

        inline static Allocator sm_allocator_{};

        template<typename Tuple_>
        static constexpr auto indices(Tuple_ &&) noexcept
            -> std::make_index_sequence< std::tuple_size_v< std::decay_t< Tuple_ > > >
        { return {}; }

    protected:
        template<typename Type> struct Destructor_ {

            using type = Type;
            using value_type = type;
            using allocator_type = Allocator;

            constexpr Destructor_() = default;

            template<typename U> requires std::is_convertible_v<U*,value_type*>
            constexpr Destructor_(Destructor_<U> const &) {}

            static constexpr void cleanup(value_type * const pointer) noexcept {
                static_assert(sizeof(Object_t) > static_cast<std::size_t>(0)
                        ,"Object must be a complete type!");
                if (pointer) {
                    // 先调用析构函数
                    pointer->~value_type();
                    // 然后使用默认分配器释放内存 (静态函数无法访问成员分配器)
                    allocator_type alloc{};
                    std::allocator_traits<allocator_type>::deallocate(alloc, pointer, 1);
                }
            }

            constexpr void operator()(value_type * const pointer) const noexcept
            { cleanup(pointer); }
        };

        using Deleter = Destructor_< Object_t >;

    public:
        using Object = Object_t;
        using ObjectSPtr = std::shared_ptr< Object >;
        using ObjectUPtr = std::unique_ptr< Object , Deleter >;

        static constexpr auto & getAllocator() noexcept
        { return sm_allocator_; }

        // 为裸指针提供专门的删除函数
        static constexpr void Delete(Object * const pointer) noexcept {
            if (pointer) {
                // 先调用析构函数
                pointer->~Object();
                // 使用分配器释放内存
                std::allocator_traits<Allocator>::deallocate(sm_allocator_, pointer, 1);
            }
        }

        constexpr void operator delete(void * const ptr, std::size_t const length ) noexcept {
            if (ptr) {
                std::allocator_traits<Allocator>::deallocate(sm_allocator_,static_cast<Object *>(ptr),length);
            }
        }

        constexpr void operator delete(void * const ptr) noexcept
        { operator delete(ptr,1); }

        template<typename ...Args1,typename ...Args2>
        [[nodiscard]]
        static constexpr auto Create( Parameter< Args1... > && args1 = {},
              Parameter< Args2...> && args2 = {} ) noexcept -> Object *
        {
            static_assert( std::disjunction_v< std::is_base_of< XTwoPhaseConstruction ,Object >
                ,std::is_convertible<Object,XTwoPhaseConstruction >
            > ,"Object must inherit from Class XHelperClass" );

            STATIC_ASSERT_P

            return [&]< std::size_t ...I1 ,std::size_t...I2 >( std::index_sequence< I1... > ,std::index_sequence< I2... > )
                noexcept -> Object *
            {
                auto const raw_ptr { std::allocator_traits<Allocator>::allocate(sm_allocator_, 1) };
                auto const obj_ptr { new (raw_ptr) Object( std::get<I1>( std::forward< decltype( args1 ) >( args1 ) )... ) };
                ObjectUPtr obj { obj_ptr, Deleter {} };
                return obj->construct_( std::get<I2>( std::forward< decltype( args2 ) >( args2 ) )... ) ? obj.release() : nullptr;
            }( indices( args1 ) ,indices( args2 ) );
        }

        template<typename ...Args1,typename ...Args2>
        [[nodiscard]] [[maybe_unused]]
        static constexpr auto CreateSharedPtr ( Parameter< Args1...> && args1 = {}
            ,Parameter< Args2...> && args2 = {} ) noexcept -> ObjectSPtr
        {
            return ObjectSPtr { Create( std::forward< decltype( args1 ) >( args1 )
                ,std::forward< decltype( args2 ) >( args2 ) ) ,Deleter{} , sm_allocator_ };
        }

        template<typename ...Args1,typename ...Args2>
        [[nodiscard]] [[maybe_unused]]
        static constexpr auto CreateUniquePtr ( Parameter< Args1... > && args1 = {}
            ,Parameter< Args2... > && args2 = {} ) noexcept -> ObjectUPtr
        {
            return { Create( std::forward< decltype( args1 ) >( args1 )
                ,std::forward< decltype( args2 ) >( args2 ) ) ,Deleter{} };
        }
    protected:
        constexpr XTwoPhaseConstruction() = default;
        template<typename ,typename > friend class XSingleton;
    };

    template<typename Tp_, typename Alloc_ >
    class XSingleton : protected XTwoPhaseConstruction<Tp_, Alloc_> {
        using Base_ = XTwoPhaseConstruction<Tp_, Alloc_>;
        static_assert(std::is_object_v<typename Base_::Object>,"Tp_ must be a class or struct type!");

    public:
        using Object = Base_::Object;
        using SingletonPtr = Base_::ObjectSPtr;

        template<typename ...Args1,typename ...Args2>
        static constexpr auto UniqueConstruction([[maybe_unused]] Parameter<Args1...> && args1 = {}
            , [[maybe_unused]] Parameter<Args2...> && args2 = {}) noexcept -> SingletonPtr
        {
            static_assert( XPrivate::is_destructor_private_v< Object >
                    , "destructor( ~Object() ) must be private!" );

            static_assert( std::disjunction_v< std::is_base_of< XSingleton ,Object >
                    ,std::is_convertible<Object,XSingleton >
            > ,"Object must inherit from Class XSingleton" );

            STATIC_ASSERT_P

            allocate_([&args1,&args2] {
                return Base_::CreateSharedPtr(std::forward< decltype(args1) >(args1)
                        ,std::forward<decltype(args2) >(args2));
            });

            return data();
        }

        static constexpr auto instance() noexcept -> SingletonPtr
        { return data(); }

        static constexpr bool isConstruct() noexcept
        { return static_cast<bool >(data()); }

    private:
        static constexpr auto data() noexcept -> SingletonPtr &
        { static SingletonPtr d{} ;return d; }

        static constexpr auto initFlag() noexcept -> std::once_flag &
        { static std::once_flag o{} ;return o; }

        template<typename Callable>
        static constexpr void allocate_([[maybe_unused]] Callable && callable) noexcept {
            std::call_once(initFlag(),[&callable]{
                if (auto ptr { std::forward<Callable>(callable)() })
                { data().swap(ptr); }
            });
        }

    protected:
        constexpr XSingleton() = default;
        W_DISABLE_COPY_MOVE(XSingleton)
    };

}

#define X_TWO_PHASE_CONSTRUCTION_CLASS \
private: \
    inline constexpr void checkFriendXTwoPhaseConstruction_() {} \
    template<typename,typename > friend class XTwoPhaseConstruction; \
    template<typename> friend struct XPrivate::Has_X_TwoPhaseConstruction_CLASS_Macro; \
    template<typename ,typename ...> friend struct XPrivate::Has_construct_Func; \
    template<typename,typename > friend class XSingleton;

#undef STATIC_ASSERT_P

#endif
