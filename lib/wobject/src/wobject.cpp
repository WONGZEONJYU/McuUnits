#include <wobject.hpp>
#include "wobject_p.hpp"
#include "wmetaobject_p.hpp"
#include <wmetaobject.hpp>

//🐞🐞🐞
WObject::WObject()
: WObject(*std::make_unique<WObjectPrivate>().release())
{}

WObject::WObject(WObjectPrivate & dd):d_ptr(&dd)
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
    auto const cd { const_cast<WObjectPrivate::ConnectionData *>(&d->connections) };
    if (!cd->currentSender) {return {};}
    for (auto const * c {cd->senders}; c; c = c->next) {
        if (c->sender == cd->currentSender->sender) {return cd->currentSender->sender;}
    }
    return {};
}

bool WObject::isSignalConnected(void * signal) const {
    W_D(const WObject);
    if (!signal) {return {};}
    return d->isSignalConnected(signal, true);
}

void * WMetaMethod::fromSignalImpl(void ** const signal) noexcept
{ return *signal; }

bool WObject::connectImpl(const WObject * const sender, void ** const signal,
                        const WObject * const receiver, void ** const slotPtr,
                         WPrivate::WSlotObjectBase * const slotObj,
                          WT::ConnectionType const type) {
    if (!signal) {
        if (slotObj) {slotObj->destroyIfLastRef();}
        return {};
    }
    return WObjectPrivate::connectImpl(sender,signal,receiver,slotPtr,slotObj,type);
}

bool WObject::disconnectImpl(const WObject *sender,void ** signal,
                             const WObject *receiver,void **slot)
{
    if (sender == nullptr || (receiver == nullptr && slot != nullptr))
    { return false; }
    return WMetaObjectPrivate::disconnect(sender,signal,receiver,slot);
}

bool WMetaObjectPrivate::disconnect(const WObject * const sender, void ** const signal,
                                    const WObject * const receiver, void ** const slot)
{
    if (!sender){ return false; }
    auto const s { const_cast<WObject *>(sender) };
    auto const scd{ &WObjectPrivate::get(s)->connections };
    if (!scd) {return false;}

    bool success {};
    {
        // remove from all connection lists
        if (auto const connections{scd}
            ; disconnectHelper(connections, signal, receiver, slot))
        { success = true; }
    }

    if (success) { scd->cleanOrphanedConnections(s); }
    return success;
}

bool WMetaObjectPrivate::disconnectHelper(WObjectPrivate::ConnectionData * const connections, void ** const signal,
                                          const WObject * const receiver, void ** const slot)
{
    bool success {};
    auto const & connectionList { connections->signalVector.connectionsForSignal };
    auto c{ connectionList.first };
    while (c){
        if (auto const r { c->receiver };
            r && (receiver == nullptr || ( r == receiver && ( signal || !c->isSlotObject )
                                                                     && ( slot == nullptr || ( c->isSlotObject && c->slotObj->compare(slot) ) ) ) ) ){
            if (c->receiver){ connections->removeConnection(c); }
            success = true;
        }
        c = c->nextConnectionList;
    }

    return success;
}

bool WObjectPrivate::disconnect(Connection * const c)
{
    if (!c) { return false; }
    if (auto const receiver{ c->receiver } ; !receiver)
    { return false; }

    {
        auto const connections { &get(c->sender)->connections };
        W_ASSERT(connections);
        connections->removeConnection(c);
        connections->cleanOrphanedConnections(c->sender);
    }
    return true;
}

bool WObjectPrivate::connectImpl(const WObject * const sender,void ** const signal,
                                 const WObject * const receiver, void ** const slot,
                                WPrivate::WSlotObjectBase * const slotObj,int type)
{
    if( !sender || !signal || !receiver || !slotObj) {
        if(slotObj) { slotObj->destroyIfLastRef(); }
        return false;
    }

    auto const s {const_cast<WObject*>(sender)};
    auto const r {const_cast<WObject*>(receiver)};

    if (type & WT::UniqueConnection && slot && &get(s)->connections) {

        auto const connections{ &get(s)->connections };
            auto const * c2{ connections->signalVector.connectionsForSignal.first};
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

    std::unique_ptr<Connection> c { new Connection };
    c->sender = s;
    c->receiver = r;
    c->sendPtr = *signal;
    c->slotObj = slotObj;
    c->slotPtr = slot ? (*slot) : (slot);
    c->isSlotObject = true;
    c->connectionType = type;
    c->isSingleShot = isSingleShot;
    get(s)->addConnection(c.get());
    return c.release();
}

void WObjectPrivate::addConnection(Connection * const c)
{
    W_ASSERT(c->sender == q_ptr);
    ensureConnectionData();
    auto const cd { &connections };
    auto & connectionList { cd->signalVector.connectionsForSignal };
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

    auto const rd { get(c->receiver) };
    rd->ensureConnectionData();
    c->prev = &rd->connections.senders;
    c->next = *c->prev;
    *c->prev = c;
    if (c->next){ c->next->prev = &c->next; }
}

void WObjectPrivate::ConnectionData::removeConnection(Connection * const c) {
    W_ASSERT(c->receiver);
    auto const cd = this;
//    ConnectionData * cd = c->owr;
    //if (!cd) return;
    auto & connections = cd->signalVector.connectionsForSignal;
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
    if (n) {n->prevConnectionList = c->prevConnectionList;}
    if (c->prevConnectionList) {c->prevConnectionList->nextConnectionList = n;}
    c->prevConnectionList = {};

    W_ASSERT(c != orphaned);
    c->nextInOrphanList = orphaned;
    orphaned = c;
}

void WObjectPrivate::ConnectionData::cleanOrphanedConnectionsImpl(WObject *) noexcept {
    if (static_cast<const int >(ref) > 1){
        return;
    }

    if (auto const c{ std::exchange(orphaned, nullptr) }){
        deleteOrphaned(c);
    }
}

void WObjectPrivate::ConnectionData::deleteOrphaned(Connection * o) noexcept {
    Connection * next {};
    while(o){
        auto const c {o};
        next = c->nextInOrphanList;
        W_ASSERT(!c->receiver);
        W_ASSERT(!c->prev);
        c->freeSlotObject();
        o = next;
    }
}

bool WObjectPrivate::maybeSignalConnected(uiBase_Type) const {
    auto const cd{ &connections };
    if (!cd){ return false; }
    auto const signalVector { &cd->signalVector };
    if (signalVector->connectionsForSignal.first){ return true; }
    auto const c{ signalVector->connectionsForSignal.first};
    return nullptr != c;
}

bool WObjectPrivate::isSignalConnected(void *signal, bool) const {
    const ConnectionData *cd = &connections;
    if (!cd){
        return false;
    }

    auto const signalVector { &cd->signalVector };

    auto c{ signalVector->connectionsForSignal.first };
    auto const _signal{signal};
    while (c) {
        if (c->receiver && _signal == c->sendPtr){
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
    { if (m_slotObject) { m_slotObject->ref(); } }
    WPrivate::WSlotObjectBase const *operator->() const
    { return m_slotObject; }
    WPrivate::WSlotObjectBase *operator->()
    { return m_slotObject; }
    ~SlotObjectGuard()
    { if (m_slotObject) {m_slotObject->destroyIfLastRef();} }
private:
    WPrivate::WSlotObjectBase *m_slotObject = nullptr;
};

inline void doActivate(WObject * const sender, void ** const signalPtr, void ** const argv) noexcept
{
    WObjectPrivate * sp = WObjectPrivate::get(sender);

    if(sp->blockSig || !sp->maybeSignalConnected() ) { return; }

    auto senderDeleted {false};
    {
        W_ASSERT(&sp->connections);
        auto const connections{ &sp->connections };
        auto const signalVector{ &connections->signalVector };
        auto const list { &signalVector->connectionsForSignal };

        auto const highestConnectionId{ connections->currentConnectionId};
        do {
            auto c { list->first};

            if (!c){
                continue;
            }

            do {
                auto const receiver{ c->receiver };
                if (!receiver) {continue;}

                if (auto const signal{ *signalPtr }
                    ; signal == c->sendPtr )
                {
                    WObjectPrivate::Sender senderData(WT::ThreadConnection != c->connectionType ?  receiver : nullptr, sender);
                    if (c->isSlotObject) {
                        SlotObjectGuard obj{c->slotObj};
                        obj->call(receiver,argv);
                    }
                }
            } while ( nullptr != (c = c->nextConnectionList)  && c->id <= highestConnectionId);

        } while (false);

        if (!connections->currentConnectionId) { senderDeleted = true; }
    }
    if (!senderDeleted){ sp->connections.cleanOrphanedConnections(sender); }
}

void WMetaObject::activate(WObject * const sender,void ** const signalPtr,void ** const argv) noexcept
{ doActivate(sender,signalPtr,argv); }

WObject::~WObject() {
    W_D(WObject);
    auto const cd{&d->connections};
    W_ASSERT(cd);

    if (cd->currentSender){
        cd->currentSender->receiverDeleted();
        cd->currentSender = nullptr;
    }

    auto const & connectionList { cd->signalVector.connectionsForSignal};

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

WObjectData::~WObjectData() = default;

WObjectPrivate:: ~WObjectPrivate() = default;

WObjectPrivate::Connection::~Connection()
{ if (isSlotObject){ slotObj->destroyIfLastRef(); } }
