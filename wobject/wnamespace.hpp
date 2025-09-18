#ifndef WNAMESPACE_H
#define WNAMESPACE_H

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
