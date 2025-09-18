#ifndef WMETAOBJECT_P_H
#define WMETAOBJECT_P_H

#include "wobjectdefs.hpp"
#include "wobject_p.hpp"

struct WMetaObjectPrivate
{
    static bool disconnect(const wobject * sender ,
                           void ** signal,
                           const wobject *receiver,
                           void **slot);

    static inline bool disconnectHelper(WObjectPrivate::ConnectionData * connections,
                                        void ** signal,
                                        const wobject * receiver,
                                        void **slot);
};

//static bool disconnect(const QObject *sender, int signal_index,
//                       const QMetaObject *smeta,
//                       const QObject *receiver, int method_index, void **slot,
//                       DisconnectType = DisconnectAll);
//static inline bool disconnectHelper(QObjectPrivate::ConnectionData *connections, int signalIndex,
//                                    const QObject *receiver, int method_index, void **slot,
//                                    QBasicMutex *senderMutex, DisconnectType = DisconnectAll);

#endif
