#ifndef QMETAOBJECT_H
#define QMETAOBJECT_H

#include "wobjectdefs.hpp"

struct WMetaMethod
{
    template <typename PointerToMemberFunction>
    static inline void *fromSignal(PointerToMemberFunction signal)
    {
        typedef WPrivate::FunctionPointer<PointerToMemberFunction> SignalType;
        return fromSignalImpl(reinterpret_cast<void **>(&signal));
    }

private:
    static void *fromSignalImpl(void **);

    friend class wobject;
    friend struct WMetaObject;
};

#endif

