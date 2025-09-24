#ifndef WOBJECT_P_H
#define WOBJECT_P_H

#include <wglobal.hpp>
#include <wobject.hpp>

class WObjectPrivate final : public WObjectData {
    W_DECLARE_PUBLIC(WObject)
public:
    struct ConnectionData;
    struct Connection {
        union { Connection * nextInOrphanList , * next{}; };
        Connection ** prev{};
        Connection * nextConnectionList{};
        Connection *prevConnectionList{};
        WObject * sender{} ,* receiver{};
        void * sendPtr{},* slotPtr{};
        WPrivate::WSlotObjectBase * slotObj{};
        //ConnectionData * owr;
        int ref_{};
        uiBase_Type id {};
        uBase_Type connectionType : 2 ;  //0 -- default 1 -- thread_mode
        uBase_Type isSlotObject:1;
        uBase_Type isSingleShot : 1;
        Connection():ref_(2),connectionType{},isSlotObject{},isSingleShot{}{}
        ~Connection();
        void ref() noexcept { ++ref_; }
        void freeSlotObject() noexcept {
            if (isSlotObject) {
                slotObj->destroyIfLastRef();
                isSlotObject = false;
            }
        }

        void deref() noexcept{
            if (!--ref_){
                W_ASSERT(!receiver);
                W_ASSERT(!isSlotObject);
                delete this;
            }
        }
    };

    struct Sender {
        Sender(WObject * const receiver, WObject * const sender):
        receiver(receiver),sender(sender)
        {
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

        void receiverDeleted() noexcept {
            auto s {this};
            while (s) {
                s->receiver = nullptr;
                s = s->previous;
            }
        }
        Sender *previous{};
        WObject *receiver{},*sender{};
    };

    struct ConnectionList {
        Connection * first{};
        Connection * last{};
        ulBase_Type Count{};
    };

    struct SignalVector {
        ConnectionList connectionsForSignal{};
    };

    struct ConnectionData {
        uiBase_Type currentConnectionId{};
        int ref{};
        SignalVector signalVector {};
        Connection *senders {};
        Sender * currentSender {};
        Connection * orphaned{};

        ~ConnectionData() {
            W_ASSERT(static_cast<const int >(ref) == 0);
            auto const c{ orphaned};
            orphaned = nullptr;
            if (c){ deleteOrphaned(c); }
            //clear signalVector
        }

        void removeConnection(Connection * c);

        void cleanOrphanedConnections(WObject *sender) noexcept {
            if (orphaned && 1 == static_cast<const int >(ref)){
                cleanOrphanedConnectionsImpl(sender);
            }
        }
        void cleanOrphanedConnectionsImpl(WObject *sender) noexcept;

        [[nodiscard]] ulBase_Type signalVectorCount() const noexcept
        { return signalVector.connectionsForSignal.Count; }

        static void deleteOrphaned(Connection * o) noexcept;
    }connections{};

    explicit WObjectPrivate();
    ~WObjectPrivate() override;
    void addConnection(Connection * c);
    bool isSignalConnected(void *, bool checkDeclarative = true) const;
    [[nodiscard]] bool maybeSignalConnected(uiBase_Type = 0) const;

    static WObjectPrivate *get(WObject *o) noexcept {return o->d_func();}
    static const WObjectPrivate *get(const WObject *o) noexcept {return o->d_func();}
    static bool connectImpl(const WObject *sender,void ** signal,
                            const WObject *receiver,void ** slot,
                            WPrivate::WSlotObjectBase * slotObj,
                            int type);
    static bool disconnect(Connection *c);

    void ensureConnectionData() noexcept
    { if (!connections.ref){ ++connections.ref; } }
};

#endif
