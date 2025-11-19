#ifndef W_GLOBAL_HPP
#define W_GLOBAL_HPP 1

/********************************************************信号槽部分，请勿乱用*********************************************/
#define W_DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete;

#define W_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define W_DISABLE_COPY_MOVE(Class)  \
    W_DISABLE_COPY(Class)           \
    W_DISABLE_MOVE(Class)

template <typename T> T * wGetPtrHelper(T * const ptr) noexcept { return ptr; }
template <typename Ptr> auto wGetPtrHelper(Ptr & ptr) noexcept -> decltype(ptr.get())
{ static_assert(noexcept(ptr.get()), "Smart d pointers for X_DECLARE_PRIVATE must have noexcept get()"); return ptr.get(); }
template <typename Ptr> auto wGetPtrHelper(Ptr const & ptr) noexcept -> decltype(ptr.get())
{ static_assert(noexcept(ptr.get()), "Smart d pointers for X_DECLARE_PRIVATE must have noexcept get()"); return ptr.get(); }

#define W_DECLARE_PRIVATE(Class) \
    inline Class##Private* d_func() noexcept \
    { return reinterpret_cast<Class##Private *>(wGetPtrHelper(d_ptr)); } \
    inline const Class##Private* d_func() const noexcept \
    { return reinterpret_cast<const Class##Private *>(wGetPtrHelper(d_ptr)); } \
    friend class Class##Private;

#define W_DECLARE_PRIVATE_D(Dptr, Class) \
    inline Class##Private* d_func() noexcept \
    { return reinterpret_cast<Class##Private *>(wGetPtrHelper(Dptr)); } \
    inline const Class##Private* d_func() const noexcept \
    { return reinterpret_cast<const Class##Private *>(wGetPtrHelper(Dptr)); } \
    friend class Class##Private;

#define W_DECLARE_PUBLIC(Class)                                    \
    inline Class* q_func() noexcept { return static_cast<Class *>(q_ptr); } \
    inline const Class* q_func() const noexcept { return static_cast<const Class *>(q_ptr); } \
    friend class Class;

#define W_D(Class) Class##Private * const d {d_func()}
#define W_Q(Class) Class * const q {q_func()}

/********************************************************信号槽部分，请勿乱用*********************************************/

typedef char Base_Type;
typedef short sBase_Type;
typedef int iBase_Type;
typedef long lBase_Type;
typedef long long llBase_Type;
typedef unsigned char uBase_Type;
typedef unsigned short usBase_Type;
typedef unsigned int uiBase_Type;
typedef unsigned long ulBase_Type;
typedef unsigned long long ullBase_Type;

#define INIT_FUNC           private
#define DE_INIT_FUNC        private

#define EXTERN_C    extern "C"
#define W_BEGIN_EXTERN_C EXTERN_C {
#define W_END_EXTERN_C  }

#define W_UNUSED(x) (void )x

#define w_assert() abort()

#define W_ASSERT(cond) ((cond) ? static_cast<void>(0) : w_assert())

#define _AT_ADDRESS_(name)	__attribute__((section(#name)))
#define __ALIGNED(x)        __attribute__((aligned(x)))

#endif
