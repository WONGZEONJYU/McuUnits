#ifndef W_NAMESPACE_H
#define W_NAMESPACE_H 1

struct WMetaObject;

namespace WT{
    enum ConnectionType {
        DirectConnection,
        ThreadConnection,
        UniqueConnection = 0x80,
        SingleShotConnection = 0x100,
    };
}

#endif
