#ifndef W_METAOBJECT_H
#define W_METAOBJECT_H 1

struct WMetaMethod {
    template <typename PointerToMemberFunction>
    static void *fromSignal(PointerToMemberFunction signal)
    { return fromSignalImpl(reinterpret_cast<void **>(&signal)); }

private:
    static void *fromSignalImpl(void **);
    friend class WObject;
    friend struct WMetaObject;
};

#endif
