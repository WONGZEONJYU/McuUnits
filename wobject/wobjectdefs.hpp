#ifndef WOBJECTDEFS_H
#define WOBJECTDEFS_H

#include "wnamespace.hpp"
#include "wobjectdefs_impl.hpp"
#include "wglobal.hpp"
#include <type_traits>

#define signals public
#define slots
#if 0
//EXAMPLE :: _emit_(nullptr,_TO_ARGS_(pram),Thread::run)
//EXAMPLE :: _emit_(nullptr,_TO_ARGS_(pram) _TO_ARGS_(pram_1),Thread::run)

#define _TO_ARGS_(_args_)  const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_args_)))

/*
#define emit_(_R_,_A_,_SIGNAL_){\
auto f { (&_SIGNAL_)};\
void *_a[] {_R_,_A_};\
WMetaObject::activate(this,reinterpret_cast<void**>(&f),_a);}


//EXAMPLE :: emit(Thread::run)

#define emit(_SIGNAL_){\
auto f = (&_SIGNAL_);\
WMetaObject::activate(this,reinterpret_cast<void**>(&f),nullptr);}
*/
#endif

class wobject;

struct WMetaObject
{
    static void activate(wobject * sender,
                         void**  signalPtr,
                         void ** argv);
};

struct _Sig_
{
    template<typename Sig,typename R,typename ...Args>
    static inline void _emit_(wobject* const _this, Sig&& f, R&& r, Args&&... args)
    {
        static_assert(std::is_member_pointer<Sig>::value);

        if constexpr (std::is_same<R,nullptr_t>::value ) {
          
          static_assert(!(std::is_rvalue_reference<decltype(args)>::value || ...));
            
          void* _a[]{ nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(args)))... };

          WMetaObject::activate(_this,reinterpret_cast<void**>(&f),_a);
        }else {
            static_assert(!(std::is_rvalue_reference<decltype(r)>::value || ... || 
                            std::is_rvalue_reference<decltype(args)>::value));
            
            void* _a[] { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(r))),
                const_cast<void*>(reinterpret_cast<const void*>(std::addressof(args)))... };
            
            WMetaObject::activate(_this,reinterpret_cast<void**>(&f),_a);
        }
    }
};

#endif

