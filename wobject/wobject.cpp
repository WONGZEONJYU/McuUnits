#include <wobject.hpp>
#include <wobject_p.hpp>
#include <wmetaobject_p.hpp>
#include <wmetaobject.hpp>

//🐞🐞🐞

WObject::WObject() noexcept :WObject(*new WObjectPrivate{})
{}

WObject::WObject(WObjectPrivate& dd):d_ptr(&dd)
{ d_ptr->q_ptr = this; }

WObjectPrivate::WObjectPrivate()
{ blockSig = false;q_ptr = nullptr; }

bool WObject::blockSignals(const bool block) noexcept {
    W_D(WObject);
    auto const previous {d->blockSig};
    d->blockSig = block;
    return previous;
}

WObject * WObject::sender() const {
    W_D(const WObject);

    WObjectPrivate::ConnectionData * cd {
            const_cast<WObjectPrivate::ConnectionData *>(&(d->connections))
    };

    if (!cd || !cd->currentSender)
        return nullptr;

    for (WObjectPrivate::Connection *c = cd->senders; c; c = c->next) {
        if (c->sender == cd->currentSender->sender)
            return cd->currentSender->sender;
    }

    return nullptr;
}

bool WObject::isSignalConnected(void * signal) const
{
    W_D(const WObject);
    if (!signal)
        return false;
    return d->isSignalConnected(signal, true);
}

void * WMetaMethod::fromSignalImpl(void ** signal)
{
    return *signal;
}

bool WObject::connectImpl(const WObject *sender, void ** signal,
                        const WObject *receiver, void ** slotPtr,
                         WPrivate::WSlotObjectBase * slotObj,
                          WT::ConnectionType type)
{
    if (!signal) {
        if (slotObj)
            slotObj->destroyIfLastRef();
        return false;
    }

    return WObjectPrivate::connectImpl(sender,signal,receiver,slotPtr,slotObj,type);
}

bool WObject::disconnectImpl(const WObject *sender,void ** signal,
                             const WObject *receiver,void **slot)
{

    if (sender == nullptr || (receiver == nullptr && slot != nullptr)) {
        return false;
    }
    return WMetaObjectPrivate::disconnect(sender,signal,receiver,slot);
}

bool WMetaObjectPrivate::disconnect(const WObject *sender, void **signal,
                                    const WObject *receiver, void **slot)
{
    if (!sender){
        return false;
    }

    WObject *s = const_cast<WObject *>(sender);

    WObjectPrivate::ConnectionData *scd = &(WObjectPrivate::get(s)->connections);
    if (!scd)
        return false;

    bool success = false;
    {
        WObjectPrivate::ConnectionData *connections(scd);

        // remove from all connection lists
        if (disconnectHelper(connections, signal, receiver, slot)){
            success = true;
        }
    }

    if (success) {
        scd->cleanOrphanedConnections(s);
    }

    return success;

}

bool WMetaObjectPrivate::disconnectHelper(WObjectPrivate::ConnectionData *connections, void **signal,
                                          const WObject *receiver, void **slot)
{
    bool success = false;
    auto &connectionList = connections->signalVector.connectionsForSignal;
    auto *c = connectionList.first;

    while (c){
        WObject *r = c->receiver;

        if (r && (receiver == nullptr || ( r == receiver && ( signal || ( !c->isSlotObject) )
                 && ( slot == nullptr || ( c->isSlotObject && c->slotObj->compare(slot) ) ) ) ) ){
            if (c->receiver){
                connections->removeConnection(c);
            }
            success = true;
        }
        c = c->nextConnectionList;
    }

    return success;
}

bool WObjectPrivate::disconnect(WObjectPrivate::Connection *c)
{
    if (!c){
        return false;
    }
    WObject *receiver = c->receiver;
    if (!receiver){
        return false;
    }
    WObjectPrivate::ConnectionData *connections;
    {
        receiver = c->receiver;
        if (!receiver){
            return false;
        }
        connections = &(WObjectPrivate::get(c->sender)->connections);
        W_ASSERT(connections);
        connections->removeConnection(c);
        connections->cleanOrphanedConnections(c->sender);
    }
    return true;
}

bool WObjectPrivate::connectImpl(const WObject *sender,void ** signal,
                                 const WObject *receiver, void ** slot,
                                WPrivate::WSlotObjectBase * slotObj,
                                 int type)
{
    if( (!sender) || (!signal) || (!receiver) || (!slotObj)) {
        if(slotObj) slotObj->destroyIfLastRef();
        return false;
    }

    auto * s = const_cast<WObject*>(sender);
    auto * r = const_cast<WObject*>(receiver);

    if (type & WT::UniqueConnection && slot && (&(WObjectPrivate::get(s)->connections))) {

        WObjectPrivate::ConnectionData *connections = &(WObjectPrivate::get(s)->connections);

            const WObjectPrivate::Connection *c2 = connections->signalVector.connectionsForSignal.first;
            while (c2) {
                if (c2->receiver == receiver && c2->isSlotObject && c2->slotObj->compare(slot)) {
                    slotObj->destroyIfLastRef();
                    return false;
                }
                c2 = c2->nextConnectionList;
            }
    }

    type &= ~WT::UniqueConnection;
    const bool isSingleShot = type & WT::SingleShotConnection;
    type &= ~WT::SingleShotConnection;

    std::unique_ptr<WObjectPrivate::Connection> c {new WObjectPrivate::Connection};
    c->sender = s;
    c->receiver = r;
    c->sendPtr = *signal;
    c->slotObj = slotObj;
    c->slotPtr = slot ? (*slot) : (slot);
    c->isSlotObject = true;
    c->connectionType = type;
    c->isSingleShot = isSingleShot;
    WObjectPrivate::get(s)->addConnection(c.get());
    return c.release();
}

void WObjectPrivate::addConnection(Connection * c)
{
    W_ASSERT(c->sender == q_ptr);
    ensureConnectionData();
    ConnectionData * cd = &connections;
    ConnectionList &connectionList = cd->signalVector.connectionsForSignal;
    if(connectionList.last){
        W_ASSERT(connectionList.last->receiver);
        connectionList.last->nextConnectionList = c;
    } else {
      connectionList.first = c;
    }
//    c->owr = cd;
    c->id = ++cd->currentConnectionId;
    c->prevConnectionList = connectionList.last;
    connectionList.last = c;

    WObjectPrivate *rd = WObjectPrivate::get(c->receiver);
    rd->ensureConnectionData();
    c->prev = &(rd->connections.senders);
    c->next = *c->prev;
    *c->prev = c;
    if (c->next){
        c->next->prev = &c->next;
    }
}

void WObjectPrivate::ConnectionData::removeConnection(Connection *c)
{
    W_ASSERT(c->receiver);
   ConnectionData * cd = this;
//    ConnectionData * cd = c->owr;
    //if (!cd) return;
    ConnectionList &connections = cd->signalVector.connectionsForSignal;
    c->receiver = nullptr;

    // remove from the senders linked list
    *c->prev = c->next;
    if (c->next){
        c->next->prev = c->prev;
    }
    c->prev = nullptr;

    if ( connections.first == c )
        connections.first = c->nextConnectionList;
    if (connections.last == c )
        connections.last = c->prevConnectionList;
    W_ASSERT(connections.first != c);
    W_ASSERT(connections.last != c);

    Connection *n = c->nextConnectionList;
    if (n)
        n->prevConnectionList = c->prevConnectionList;
    if (c->prevConnectionList)
        c->prevConnectionList->nextConnectionList = n;
    c->prevConnectionList = nullptr;

    W_ASSERT(c != orphaned);
    Connection * o = orphaned;
    c->nextInOrphanList = o;
    orphaned = c;
}

void WObjectPrivate::ConnectionData::cleanOrphanedConnectionsImpl(WObject *sender)
{
    (void) sender;
    if (static_cast<const int >(ref) > 1){
        return;
    }
    Connection * c = orphaned;
    orphaned = nullptr;
    if (c){
        deleteOrphaned(c);
    }
}

void WObjectPrivate::ConnectionData::deleteOrphaned(Connection * o)
{
    Connection * next = nullptr;
    while(o){
        Connection * c = o;
        next = c->nextInOrphanList;
        W_ASSERT(!c->receiver);
        W_ASSERT(!c->prev);
        c->freeSlotObject();
        o = next;
    }
}

bool WObjectPrivate::maybeSignalConnected(uiBase_Type) const
{
    const ConnectionData * cd = &connections;
    if (!cd){
        return false;
    }
    const SignalVector *signalVector = &cd->signalVector;
    if (!signalVector){
        return false;
    }
    if (signalVector->connectionsForSignal.first){
        return true;
    }
    const WObjectPrivate::Connection *c = signalVector->connectionsForSignal.first;
    return (c != nullptr);
    //return false;
}

bool WObjectPrivate::isSignalConnected(void *signal, bool checkDeclarative) const
{
    (void) checkDeclarative;
    const ConnectionData *cd = &connections;
    if (!cd){
        return false;
    }

    const SignalVector *signalVector = &(cd->signalVector);
    if (!signalVector){
        return false;
    }

    const WObjectPrivate::Connection *c = signalVector->connectionsForSignal.first;
    const void * const _signal {signal};
    while (c) {
        if (c->receiver && (c->sendPtr == _signal)){
            return true;
        }
        c = c->nextConnectionList;
    }

    return false;
}

struct SlotObjectGuard {
    SlotObjectGuard() = default;
    // move would be fine, but we do not need it currently
    W_DISABLE_COPY_MOVE(SlotObjectGuard)
    explicit SlotObjectGuard(WPrivate::WSlotObjectBase *slotObject)
            : m_slotObject(slotObject)
    {
        if (m_slotObject)
            m_slotObject->ref();
    }
    WPrivate::WSlotObjectBase const *operator->() const
    { return m_slotObject; }
    WPrivate::WSlotObjectBase *operator->()
    { return m_slotObject; }
    ~SlotObjectGuard() {
        if (m_slotObject)
            m_slotObject->destroyIfLastRef();
    }
private:
    WPrivate::WSlotObjectBase *m_slotObject = nullptr;
};

inline void doActivate(WObject *sender, void ** signalPtr, void **argv)
{
    WObjectPrivate * sp = WObjectPrivate::get(sender);

    if(sp->blockSig) {
        return;
    }

    if (!sp->maybeSignalConnected()){
        return;
    }

    bool senderDeleted = false;
    {
        W_ASSERT(&(sp->connections));
        WObjectPrivate::ConnectionData * connections (&(sp->connections));
        WObjectPrivate::SignalVector *signalVector = &(connections->signalVector);
        const WObjectPrivate::ConnectionList *list = &(signalVector->connectionsForSignal);

        uiBase_Type highestConnectionId = connections->currentConnectionId;
        do {
            WObjectPrivate::Connection *c = list->first;

            if (!c){
                continue;
            }

            do {
                WObject * const receiver = c->receiver;
                if (!receiver)
                    continue;

                const void * const signal { *signalPtr };
                if ( signal == c->sendPtr ){
                    WObjectPrivate::Sender senderData(((WT::ThreadConnection != c->connectionType) ?  receiver : nullptr), sender);
                    if (c->isSlotObject){
                        SlotObjectGuard obj{c->slotObj};
                        {
                            obj->call(receiver,argv);
                        }
                    }
                }
            } while ( ((c = c->nextConnectionList) != nullptr) && (c->id <= highestConnectionId));

        } while (false);

        if (0 == connections->currentConnectionId){
            senderDeleted = true;
        }
    }
    if (!senderDeleted){
        sp->connections.cleanOrphanedConnections(sender);
    }
}

void WMetaObject::activate(WObject * sender,
                           void ** signalPtr,
                           void ** argv)
{
    doActivate(sender,signalPtr,argv);
}

WObject::~WObject()
{
    W_D(WObject);

    WObjectPrivate::ConnectionData * cd = &d->connections;
    if (cd){
        if (cd->currentSender){
            cd->currentSender->receiverDeleted();
            cd->currentSender = nullptr;
        }
    }

    WObjectPrivate::ConnectionList &connectionList = cd->signalVector.connectionsForSignal;

    while (WObjectPrivate::Connection *c = connectionList.first) {
        W_ASSERT(c->receiver);
        if (c == connectionList.first) {
            cd->removeConnection(c);
            W_ASSERT(connectionList.first != c);
        }
    }

    while (WObjectPrivate::Connection *node = cd->senders) {
        W_ASSERT(node->receiver);
        WObject *sender = node->sender;
        WObjectPrivate::ConnectionData *senderData = &(sender->d_func()->connections);
        W_ASSERT(senderData);
        WPrivate::WSlotObjectBase *slotObj = nullptr;
        if (node->isSlotObject) {
            slotObj = node->slotObj;
            node->isSlotObject = false;
        }
        senderData->removeConnection(node);
        senderData->cleanOrphanedConnections(sender);

        if (slotObj){
            slotObj->destroyIfLastRef();
        }
    }
    cd->currentConnectionId = 0;
    --cd->ref;
}

WObjectData::~WObjectData()
{
}

WObjectPrivate:: ~WObjectPrivate()
{
}

WObjectPrivate::Connection::~Connection()
{
    if (isSlotObject){
        slotObj->destroyIfLastRef();
    }
}
