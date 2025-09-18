#ifndef WOBJECT_P_H
#define WOBJECT_P_H

#include "wglobal.hpp"
#include "wobject.hpp"

class WObjectPrivate : public WObjectData
{
    W_DECLARE_PUBLIC(wobject)
public:
    struct ConnectionData;
    struct Connection
    {
        union {
            Connection * nextInOrphanList , * next{};
        };
        Connection ** prev;
        Connection * nextConnectionList{};
        Connection *prevConnectionList;
        wobject *sender ,*receiver;
        void *sendPtr,*slotPtr;
        WPrivate::WSlotObjectBase *slotObj;
        //ConnectionData * owr;
        int ref_;
        uiBase_Type id {};
        uBase_Type connectionType : 2 ;  //0 -- default 1 -- thread_mode
        uBase_Type isSlotObject:1;
        uBase_Type isSingleShot : 1;
        Connection():ref_(2){}
        ~Connection();
        void ref(){ ++ref_; }
        void freeSlotObject()
        {
            if (isSlotObject) {
                slotObj->destroyIfLastRef();
                isSlotObject = false;
            }
        }
        void deref()
        {
            if (!(--ref_)){
                W_ASSERT(!receiver);
                W_ASSERT(!isSlotObject);
                delete this;
            }
        }
    };

    struct Sender
    {
        Sender(wobject *receiver, wobject *sender):
                receiver(receiver),sender(sender){
            if (receiver){
                ConnectionData * cd = &(receiver->d_func()->connections);
                previous = cd->currentSender;
                cd->currentSender = this;
            }
        }

        ~Sender(){
            if (receiver){
                receiver->d_func()->connections.currentSender = previous;
            }
        }

        void receiverDeleted()
        {
            Sender *s = this;
            while (s) {
                s->receiver = nullptr;
                s = s->previous;
            }
        }
        Sender *previous;
        wobject *receiver,*sender;
    };

    struct ConnectionList
    {
        Connection * first{};
        Connection * last{};
        ulBase_Type Count;
    };

    struct SignalVector{
        ConnectionList connectionsForSignal{};
    };

    struct ConnectionData
    {
        uiBase_Type currentConnectionId;
        int ref;
        SignalVector signalVector {};
        Connection *senders {};
        Sender * currentSender {};
        Connection * orphaned{};

        ~ConnectionData()
        {
            W_ASSERT(static_cast<const int >(ref) == 0);
            auto * c = orphaned;
            orphaned = nullptr;
            if (c){
                deleteOrphaned(c);
            }
            //clear signalVector
        }

        void removeConnection(Connection * c);

        void cleanOrphanedConnections(wobject *sender)
        {
            if ((orphaned) && (1 == static_cast<const int >(ref))){
                cleanOrphanedConnectionsImpl(sender);
            }
        }
        void cleanOrphanedConnectionsImpl(wobject *sender);

        ulBase_Type signalVectorCount() const{
            return signalVector.connectionsForSignal.Count;
        }

        static void deleteOrphaned(Connection * o);
    }connections{};

    explicit WObjectPrivate();
    virtual ~WObjectPrivate();
    void addConnection(Connection * c);
    bool isSignalConnected(void *, bool checkDeclarative = true) const;
    bool maybeSignalConnected(uiBase_Type = 0) const;

    static inline WObjectPrivate *get(wobject *o){return o->d_func();}
    static inline const WObjectPrivate *get(const wobject *o){return o->d_func();}
    static bool connectImpl(const wobject *sender,void ** signal,
                            const wobject *receiver,void ** slot,
                            WPrivate::WSlotObjectBase * slotObj,
                            int type);
    static bool disconnect(Connection *c);

    void ensureConnectionData()
    {
        if (!connections.ref){
            ++connections.ref;
        }
    }
};

#endif
