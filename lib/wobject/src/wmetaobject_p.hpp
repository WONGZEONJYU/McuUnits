#ifndef W_METAOBJECT_P_HPP
#define W_METAOBJECT_P_HPP 1

#include <wobjectdefs.hpp>

struct WMetaObjectPrivate {
    static bool disconnect(const WObject * sender ,void ** signal,
                           const WObject *receiver,void **slot);

    static inline bool disconnectHelper(WObjectPrivate::ConnectionData * connections,void ** signal,
                                        const WObject * receiver,void **slot);
};

#endif
