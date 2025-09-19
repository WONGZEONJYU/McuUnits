#ifndef W_METAOBJECT_P_HPP
#define W_METAOBJECT_P_HPP 1

#include <wobjectdefs.hpp>
#include <wobject_p.hpp>

struct WMetaObjectPrivate {
    static bool disconnect(const WObject * sender ,void ** signal,
                           const WObject *receiver,void **slot);

    static inline bool disconnectHelper(WObjectPrivate::ConnectionData * connections,void ** signal,
                                        const WObject * receiver,void **slot);
};

#if 0
static bool disconnect(const QObject *sender, int signal_index,
                       const QMetaObject *smeta,
                       const QObject *receiver, int method_index, void **slot,
                       DisconnectType = DisconnectAll);
static inline bool disconnectHelper(QObjectPrivate::ConnectionData *connections, int signalIndex,
                                    const QObject *receiver, int method_index, void **slot,
                                    QBasicMutex *senderMutex, DisconnectType = DisconnectAll);
#endif

#endif
