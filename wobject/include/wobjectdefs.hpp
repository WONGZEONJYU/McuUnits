#ifndef WOBJECTDEFS_H
#define WOBJECTDEFS_H

#include <type_traits>
#include <wobjectdefs_impl.hpp>

#define signals public
#define slots

class WObject;

struct WMetaObject {
    static void activate(WObject * sender,void**  signalPtr,void ** argv) noexcept;
};

struct Emit {
    template<typename Sig,typename R,typename ...Args>
    static void emit_(WObject* const _this, Sig&& f, R&& r, Args&&... args) {
        static_assert(std::is_member_pointer_v<Sig>);
        if constexpr (std::is_same_v<R,std::nullptr_t> ) {
          static_assert(!(std::is_rvalue_reference_v<decltype(args)> || ...));
          void* _a[]{ nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(args)))... };
          WMetaObject::activate(_this,reinterpret_cast<void**>(&f),_a);
        }else {
            static_assert(!(std::is_rvalue_reference_v<decltype(r)> || ... ||
                            std::is_rvalue_reference_v<decltype(args)>));
            void* _a[] { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(r))),
                const_cast<void*>(reinterpret_cast<const void*>(std::addressof(args)))... };
            WMetaObject::activate(_this,reinterpret_cast<void**>(&f),_a);
        }
    }
};

#endif
