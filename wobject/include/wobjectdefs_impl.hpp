#pragma once
#ifndef WOBJECTDEFS_H
#error Do not include wobjectdefs_impl.h directly
#include "wnamespace.h"
#endif

#include <wglobal.hpp>
#include <utility>

class WObject;

namespace WPrivate {

    template <typename T> struct RemoveRef { using Type = T; };
    template <typename T> struct RemoveRef<T&> { using Type = T; };
    template <typename T> struct RemoveConstRef { using Type = T; };
    template <typename T> struct RemoveConstRef<const T&> { using Type = T; };

    /*
       The following List classes are used to help to handle the list of arguments.
       It follow the same principles as the lisp lists.
       List_Left<L,N> take a list and a number as a parameter and returns (via the Value typedef,
       the list composed of the first N element of the list
     */
    // With variadic template, lists are represented using a variadic template argument instead of the lisp way
    template <typename...> struct List {};
    template <typename Head, typename... Tail> struct List<Head, Tail...> { using Car = Head ; using Cdr = List<Tail...> ; };
    template <typename, typename> struct List_Append;
    template <typename... L1, typename...L2> struct List_Append<List<L1...>, List<L2...>> { using Value = List<L1..., L2...> ; };
    template <typename L, int N> struct List_Left {
        using Value = typename List_Append<List<typename L::Car>,typename List_Left<typename L::Cdr, N - 1>::Value>::Value ;
    };
    template <typename L> struct List_Left<L, 0> { using Value = List<> ; };
    // List_Select<L,N> returns (via typedef Value) the Nth element of the list L
    template <typename L, int N> struct List_Select { using Value = typename List_Select<typename L::Cdr, N - 1>::Value ; };
    template <typename L> struct List_Select<L,0> { using Value = typename L::Car ; };

    /*
       trick to set the return value of a slot that works even if the signal or the slot returns void
       to be used like     function(), ApplyReturnValue<ReturnType>(&return_value)
       if function() returns a value, the operator,(T, ApplyReturnValue<ReturnType>) is called, but if it
       returns void, the builtin one is used without an error.
    */
    template <typename >
    struct ApplyReturnValue {
        void *data{};
        explicit ApplyReturnValue(void *data_) : data(data_) {}
    };

    template<typename T, typename U>
    void operator,(T &&value, ApplyReturnValue<U> const & container) noexcept
    { if (container.data) { *reinterpret_cast<U *>(container.data) = std::forward<T>(value); } }

    template<typename T>
    void operator,(T, const ApplyReturnValue<void> &) noexcept {}

    /*
      The FunctionPointer<Func> struct is a type trait for function pointer.
        - ArgumentCount  is the number of argument, or -1 if it is unknown
        - the Object typedef is the Object of a pointer to member function
        - the Arguments typedef is the list of argument (in a QtPrivate::List)
        - the Function typedef is an alias to the template parameter Func
        - the call<Args, R>(f,o,args) method is used to call that slot
            Args is the list of argument of the signal
            R is the return type of the signal
            f is the function pointer
            o is the receiver object
            and args is the array of pointer to arguments, as used in qt_metacall

       The Functor<Func,N> struct is the helper to call a functor of N argument.
       its call function is the same as the FunctionPointer::call function.
     */
    template<class T> using InvokeGenSeq = typename T::Type;

    template<int...> struct IndexesList { using Type = IndexesList; };

    template<int N, class S1, class S2> struct ConcatSeqImpl;

    template<int N, int... I1, int... I2>
    struct ConcatSeqImpl<N, IndexesList<I1...>, IndexesList<I2...>>
        : IndexesList<I1..., N + I2...>{};

    template<int N, class S1, class S2>
    using ConcatSeq = InvokeGenSeq<ConcatSeqImpl<N, S1, S2>>;

    template<int N> struct GenSeq;
    template<int N> using makeIndexSequence = InvokeGenSeq<GenSeq<N>>;

    template<int N>
    struct GenSeq : ConcatSeq<N/2, makeIndexSequence<N/2>, makeIndexSequence<N - N/2>>{};

    template<> struct GenSeq<0> : IndexesList<>{};
    template<> struct GenSeq<1> : IndexesList<0>{};

    template<int N>
    struct Indexes { using Value = makeIndexSequence<N>; };

    template<typename > struct FunctionPointer { enum {ArgumentCount = -1, IsPointerToMemberFunction = false}; };

    template <typename, typename, typename, typename> struct FunctorCall;

    template <int... II, typename... SignalArgs, typename R, typename Function>
    struct FunctorCall<IndexesList<II...>, List<SignalArgs...>, R, Function> {
        static void call(Function &f, void ** const arg)
        { f((*reinterpret_cast<typename RemoveRef<SignalArgs>::Type *>(arg[II+1]))...), ApplyReturnValue<R>(arg[0]); }
    };

    template <int... II, typename... SignalArgs, typename R, typename... SlotArgs, typename SlotRet, class Obj>
    struct FunctorCall<IndexesList<II...>, List<SignalArgs...>, R, SlotRet (Obj::*)(SlotArgs...)> {
        using Function = SlotRet (Obj::*)(SlotArgs...);
        static void call(Function const f, Obj * const o, void ** const arg)
        { (o->*f)((*reinterpret_cast<typename RemoveRef<SignalArgs>::Type *>(arg[II+1]))...), ApplyReturnValue<R>(arg[0]); }
    };

    template <int... II, typename... SignalArgs, typename R, typename... SlotArgs, typename SlotRet, class Obj>
    struct FunctorCall<IndexesList<II...>, List<SignalArgs...>, R, SlotRet (Obj::*)(SlotArgs...) const> {
        using Function = SlotRet (Obj::*)(SlotArgs...) const;
        static void call(Function const f, Obj * const o, void ** const arg)
        { (o->*f)((*reinterpret_cast<typename RemoveRef<SignalArgs>::Type *>(arg[II+1]))...), ApplyReturnValue<R>(arg[0]); }
    };

    template <int... II, typename... SignalArgs, typename R, typename... SlotArgs, typename SlotRet, class Obj>
    struct FunctorCall<IndexesList<II...>, List<SignalArgs...>, R, SlotRet (Obj::*)(SlotArgs...) noexcept> {
        using Function = SlotRet (Obj::*)(SlotArgs...) noexcept;
        static void call(Function const f, Obj * const o, void ** const arg)
        { (o->*f)((*reinterpret_cast<typename RemoveRef<SignalArgs>::Type *>(arg[II+1]))...), ApplyReturnValue<R>(arg[0]); }
    };

    template <int... II, typename... SignalArgs, typename R, typename... SlotArgs, typename SlotRet, class Obj>
    struct FunctorCall<IndexesList<II...>, List<SignalArgs...>, R, SlotRet (Obj::*)(SlotArgs...) const noexcept> {
        using Function = SlotRet (Obj::*)(SlotArgs...) const noexcept;
        static void call( Function const f, Obj * const o, void ** const arg)
        { (o->*f)((*reinterpret_cast<typename RemoveRef<SignalArgs>::Type *>(arg[II+1]))...), ApplyReturnValue<R>(arg[0]); }
    };

    template<class Obj, typename Ret, typename... Args> struct FunctionPointer<Ret (Obj::*) (Args...)> {
        using Object = Obj;
        using Arguments = List<Args...>;
        using ReturnType = Ret;
        using Function = Ret (Obj::*) (Args...);
        enum {ArgumentCount = sizeof...(Args), IsPointerToMemberFunction = true};
        template <typename SignalArgs, typename R>
        static void call(Function const f, Obj * const o, void ** const arg)
        { FunctorCall<typename Indexes<ArgumentCount>::Value, SignalArgs, R, Function>::call(f, o, arg); }
    };

    template<class Obj, typename Ret, typename... Args> struct FunctionPointer<Ret (Obj::*) (Args...) const> {
        using Object = Obj ;
        using Arguments = List<Args...>  ;
        using ReturnType =  Ret ;
        using Function = Ret (Obj::*) (Args...) const;
        enum {ArgumentCount = sizeof...(Args), IsPointerToMemberFunction = true};
        template <typename SignalArgs, typename R>
        static void call(Function const f, Obj * const o, void ** const arg)
        { FunctorCall<typename Indexes<ArgumentCount>::Value, SignalArgs, R, Function>::call(f, o, arg); }
    };

    template<typename Ret, typename... Args> struct FunctionPointer<Ret (*) (Args...)> {
        using Arguments = List<Args...> ;
        using ReturnType = Ret ;
        using Function = Ret (*) (Args...);
        enum {ArgumentCount = sizeof...(Args), IsPointerToMemberFunction = false};
        template <typename SignalArgs, typename R>
        static void call(Function const f, void *, void ** const arg)
        { FunctorCall<typename Indexes<ArgumentCount>::Value, SignalArgs, R, Function>::call(f, arg); }
    };

    template<class Obj, typename Ret, typename... Args> struct FunctionPointer<Ret (Obj::*) (Args...) noexcept> {
        using Object = Obj ;
        using Arguments = List<Args...> ;
        using ReturnType = Ret ;
        using Function = Ret (Obj::*) (Args...) noexcept;
        enum {ArgumentCount = sizeof...(Args), IsPointerToMemberFunction = true};
        template <typename SignalArgs, typename R>
        static void call(Function const f, Obj * const o, void ** const arg)
        { FunctorCall<typename Indexes<ArgumentCount>::Value, SignalArgs, R, Function>::call(f, o, arg); }
    };

    template<class Obj, typename Ret, typename... Args> struct FunctionPointer<Ret (Obj::*) (Args...) const noexcept> {
        using Object = Obj ;
        using Arguments = List<Args...>  ;
        using ReturnType = Ret ;
        using Function = Ret (Obj::*) (Args...) const noexcept;
        enum {ArgumentCount = sizeof...(Args), IsPointerToMemberFunction = true};
        template <typename SignalArgs, typename R>
        static void call(Function const f, Obj * const o, void ** const arg)
        { FunctorCall<typename Indexes<ArgumentCount>::Value, SignalArgs, R, Function>::call(f, o, arg); }
    };

    template<typename Ret, typename... Args> struct FunctionPointer<Ret (*) (Args...) noexcept> {
        using Arguments = List<Args...> ;
        using ReturnType = Ret ;
        using Function = Ret (*) (Args...) noexcept;
        enum {ArgumentCount = sizeof...(Args), IsPointerToMemberFunction = false};
        template <typename SignalArgs, typename R>
        static void call(Function const f, void *, void ** const arg)
        { FunctorCall<typename Indexes<ArgumentCount>::Value, SignalArgs, R, Function>::call(f, arg); }
    };

    template<typename Function, int N> struct Functor {
        template <typename SignalArgs, typename R>
        static void call(Function & f, void *, void ** const arg)
        { FunctorCall<typename Indexes<N>::Value, SignalArgs, R, Function>::call(f, arg); }
    };

    /*
        Logic that checks if the underlying type of an enum is signed or not.
        Needs an external, explicit check that E is indeed an enum. Works
        around the fact that it's undefined behavior to instantiate
        std::underlying_type on non-enums (cf. §20.13.7.6 [meta.trans.other]).
    */
    template<typename , typename = void>
    struct IsEnumUnderlyingTypeSigned : std::false_type {};

    template<typename E>
    struct IsEnumUnderlyingTypeSigned<E, std::enable_if_t<std::is_enum_v<E>>>
            : std::integral_constant<bool, std::is_signed_v<std::underlying_type_t<E>>> {};

    /*
       Logic that checks if the argument of the slot does not narrow the
       argument of the signal when used in list initialization. Cf. §8.5.4.7
       [dcl.init.list] for the definition of narrowing.
       For incomplete From/To types, there's no narrowing.
    */
    template<typename , typename , typename = void>
    struct AreArgumentsNarrowedBase : std::false_type {};

    template <typename T>
    using is_bool = std::is_same<bool, std::decay_t<T>>;

    template <typename T>
    inline constexpr auto is_bool_v { is_bool<T>::value };

    template<typename From, typename To>
    struct AreArgumentsNarrowedBase<From, To, std::enable_if_t<sizeof(From) && sizeof(To)>>
        : std::integral_constant<bool,
              (std::is_floating_point_v<From> && std::is_integral_v<To>) ||
              (std::is_floating_point_v<From> && std::is_floating_point_v<To> && sizeof(From) > sizeof(To)) ||
              ((std::is_pointer_v<From> || std::is_member_pointer_v<From>) && is_bool_v<To>) ||
              ((std::is_integral_v<From> || std::is_enum_v<From>) && std::is_floating_point_v<To>) ||
              (std::is_integral_v<From> && std::is_integral_v<To>
               && (sizeof(From) > sizeof(To)
                   || (std::is_signed_v<From> ? !std::is_signed_v<To>
                       : std::is_signed_v<To> && sizeof(From) == sizeof(To)))) ||
              (std::is_enum_v<From> && std::is_integral_v<To>
               && (sizeof(From) > sizeof(To)
                   || (IsEnumUnderlyingTypeSigned<From>::value ? !std::is_signed_v<To>
                       : std::is_signed_v<To> && sizeof(From) == sizeof(To))))
              >
    {};

    /*
       Logic that check if the arguments of the slot matches the argument of the signal.
       To be used like this:
       Q_STATIC_ASSERT(CheckCompatibleArguments<FunctionPointer<Signal>::Arguments, FunctionPointer<Slot>::Arguments>::value)
    */
    template<typename A1, typename A2> struct AreArgumentsCompatible {
        static int test(const typename RemoveRef<A2>::Type&){return {};}
        static char test(...){return {};}
        static const typename RemoveRef<A1>::Type & dummy() {return {};}
        enum { value = sizeof(test(dummy())) == sizeof(int) };
#ifdef W_NO_NARROWING_CONVERSIONS_IN_CONNECT
        using AreArgumentsNarrowed = AreArgumentsNarrowedBase<typename RemoveRef<A1>::Type, typename RemoveRef<A2>::Type>;
        Q_STATIC_ASSERT_X(!AreArgumentsNarrowed::value, "Signal and slot arguments are not compatible (narrowing)");
#endif
    };
    template<typename A1, typename A2> struct AreArgumentsCompatible<A1, A2&> { enum { value = false }; };
    template<typename A> struct AreArgumentsCompatible<A&, A&> { enum { value = true }; };
    // void as a return value
    template<typename A> struct AreArgumentsCompatible<void, A> { enum { value = true }; };
    template<typename A> struct AreArgumentsCompatible<A, void> { enum { value = true }; };
    template<> struct AreArgumentsCompatible<void, void> { enum { value = true }; };

    template <typename , typename > struct CheckCompatibleArguments { enum { value = false }; };
    template <> struct CheckCompatibleArguments<List<>, List<>> { enum { value = true }; };
    template <typename List1> struct CheckCompatibleArguments<List1, List<>> { enum { value = true }; };
    template <typename Arg1, typename Arg2, typename... Tail1, typename... Tail2>
    struct CheckCompatibleArguments<List<Arg1, Tail1...>, List<Arg2, Tail2...>> {
        enum { value = AreArgumentsCompatible<typename RemoveConstRef<Arg1>::Type, typename RemoveConstRef<Arg2>::Type>::value
                    && CheckCompatibleArguments<List<Tail1...>, List<Tail2...>>::value };
    };

    /*
       Find the maximum number of arguments a functor object can take and be still compatible with
       the arguments from the signal.
       Value is the number of arguments, or -1 if nothing matches.
     */
    template <typename , typename > struct ComputeFunctorArgumentCount;

    template <typename , typename , bool > struct ComputeFunctorArgumentCountHelper
    { enum { Value = -1 }; };

    template <typename Functor, typename First, typename... ArgList>
    struct ComputeFunctorArgumentCountHelper<Functor, List<First, ArgList...>, false>
        : ComputeFunctorArgumentCount<Functor,
            typename List_Left<List<First, ArgList...>, sizeof...(ArgList)>::Value> {};

    template <typename Functor, typename... ArgList> struct ComputeFunctorArgumentCount<Functor, List<ArgList...>>
    {
        template <typename D> static D dummy()
        {return {};}
        template <typename F> static auto test([[maybe_unused]] F f) -> decltype(f.operator()((dummy<ArgList>())...), int())
        {return {};}
        static char test(...)
        { return {};}
        enum {
            Ok = sizeof(test(dummy<Functor>())) == sizeof(int),
            Value = Ok ? static_cast<int>(sizeof...(ArgList)) : static_cast<int>(ComputeFunctorArgumentCountHelper<Functor, List<ArgList...>, Ok>::Value)
        };
    };

    /* get the return type of a functor, given the signal argument list  */
    template <typename Functor, typename ArgList> struct FunctorReturnType;
    template <typename Functor, typename ... ArgList> struct FunctorReturnType<Functor, List<ArgList...>> {
        template <typename D> static D dummy() {return {};}
        using Value = decltype(dummy<Functor>().operator()((dummy<ArgList>())...)) ;
    };

    // internal base class (interface) containing functions required to call a slot managed by a pointer to function.
    class WSlotObjectBase {
        int m_ref{};
        // don't use virtual functions here; we don't want the
        // compiler to create tons of per-polymorphic-class stuff that
        // we'll never need. We just use one function pointer.
        using ImplFn = void (*)(int which, WSlotObjectBase* this_, WObject *receiver, void **args, bool *ret);
        ImplFn const m_impl{};
    protected:
        enum Operation {
            Destroy,
            Call,
            Compare,

            NumOperations
        };
    public:
        explicit WSlotObjectBase(ImplFn const fn) :m_ref(1), m_impl(fn) {}
        int ref() noexcept { return ++m_ref; }
        void destroyIfLastRef() noexcept
        { if (!--m_ref) { m_impl(Destroy, this, nullptr, nullptr, nullptr); } }
        bool compare(void ** const a) noexcept
        { bool ret {}; m_impl(Compare, this, nullptr, a, &ret); return ret; }
        void call(WObject * const r, void ** const a)
        { m_impl(Call,this, r, a, nullptr); }
    protected:
        ~WSlotObjectBase() = default;
    private:
        W_DISABLE_COPY_MOVE(WSlotObjectBase)
    };

    // implementation of QSlotObjectBase for which the slot is a pointer to member function of a QObject
    // Args and R are the List of arguments and the return type of the signal to which the slot is connected.
    template<typename Func, typename Args, typename R> class WSlotObject : public WSlotObjectBase {
        using FuncType = FunctionPointer<Func> ;
        Func function{};
        static void impl(int const which, WSlotObjectBase * const this_, WObject * const r, void ** const a, bool * const ret) {
            switch (which) {
            case Destroy:
                delete static_cast<WSlotObject*>(this_);
                break;
            case Call:
                FuncType::template call<Args, R>(static_cast<WSlotObject*>(this_)->function, static_cast<typename FuncType::Object *>(r), a);
                break;
            case Compare:
                *ret = *reinterpret_cast<Func *>(a) == static_cast<WSlotObject*>(this_)->function;
                break;
            case NumOperations: ;
                    default: break;
            }
        }
    public:
        explicit WSlotObject(Func f) : WSlotObjectBase(&impl), function(f) {}
    };
    // implementation of QSlotObjectBase for which the slot is a functor (or lambda)
    // N is the number of arguments
    // Args and R are the List of arguments and the return type of the signal to which the slot is connected.
    template<typename Func, int N, typename Args, typename R> class WFunctorSlotObject : public WSlotObjectBase
    {
        using FuncType = Functor<Func, N> ;
        Func function{};
        static void impl(int const which, WSlotObjectBase * const this_, WObject * const r, void ** const a, bool *) {
            switch (which) {
            case Destroy:
                delete static_cast<WFunctorSlotObject*>(this_);
                break;
            case Call:
                FuncType::template call<Args, R>(static_cast<WFunctorSlotObject*>(this_)->function, r, a);
                break;
            case Compare: // not implemented
            case NumOperations:
            default:break;
            }
        }
    public:
        explicit WFunctorSlotObject(Func f) : WSlotObjectBase(&impl), function(std::move(f)) {}
    };

    template<typename Func, typename Args, typename R> class WStaticSlotObject : public WSlotObjectBase {
        using FuncType = FunctionPointer<Func> ;
        Func function{};
        static void impl(int const which, WSlotObjectBase *this_, WObject *r, void **a, bool *) {
            switch (which) {
            case Destroy:
                delete static_cast<WStaticSlotObject*>(this_);
                break;
            case Call:
                FuncType::template call<Args, R>(static_cast<WStaticSlotObject*>(this_)->function, r, a);
                break;
            case Compare:   // not implemented
            case NumOperations:
            default:break;
            }
        }
    public:
        explicit WStaticSlotObject(Func f) : WSlotObjectBase(&impl), function(std::move(f)) {}
    };

    // typedefs for readability for when there are no parameters
    template <typename Func>
    using WSlotObjectWithNoArgs = WSlotObject<Func,List<>,typename FunctionPointer<Func>::ReturnType>;

    template <typename Func, typename R>
    using WFunctorSlotObjectWithNoArgs = WFunctorSlotObject<Func, 0, List<>, R>;

    template <typename Func>
    using WFunctorSlotObjectWithNoArgsImplicitReturn = WFunctorSlotObjectWithNoArgs<Func, typename FunctionPointer<Func>::ReturnType>;
}
