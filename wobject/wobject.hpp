#pragma once
#ifndef _W_OBJECT_H
#define _W_OBJECT_H

#include "wobjectdefs.hpp"
#include "wmetaobject.hpp"
#include "wglobal.hpp"
#include <memory>

struct WMetaObject;
class WObjectPrivate;
class wobject;

class WObjectData
{
    W_DISABLE_COPY(WObjectData)
public:
    WObjectData() = default;
    virtual ~WObjectData() = 0;
    wobject *q_ptr;
    uBase_Type blockSig:1;
};

class wobject
{
    W_DECLARE_PRIVATE(WObject)

public:
    explicit wobject() noexcept;
    virtual ~wobject();

    inline bool signalsBlocked() const noexcept{return d_ptr->blockSig;}
    bool blockSignals(const bool block) noexcept;

    template <typename Func1, typename Func2>
        static inline bool connect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                         const typename WPrivate::FunctionPointer<Func2>::Object *receiver, Func2 slot,
                                         WT::ConnectionType type = WT::DirectConnection)
        {
            typedef WPrivate::FunctionPointer<Func1> SignalType;
            typedef WPrivate::FunctionPointer<Func2> SlotType;

            static_assert(int (SignalType::ArgumentCount) >= int(SlotType::ArgumentCount),
                          "The slot requires more arguments than the signal provides.");

            static_assert((WPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value),
                          "Signal and slot arguments are not compatible.");
            static_assert((WPrivate::AreArgumentsCompatible<typename SlotType::ReturnType, typename SignalType::ReturnType>::value),
                          "Return type of the slot is not compatible with the return type of the signal.");

            return connectImpl(sender, reinterpret_cast<void **>(&signal),
                               receiver, reinterpret_cast<void **>(&slot),
                               new WPrivate::WSlotObject<Func2, typename WPrivate::List_Left<typename SignalType::Arguments, SlotType::ArgumentCount>::Value,
                                               typename SignalType::ReturnType>(slot),
                                               type);
        }

        //connect to a function pointer  (not a member)
        template <typename Func1, typename Func2>
        static inline typename std::enable_if<int(WPrivate::FunctionPointer<Func2>::ArgumentCount) >= 0 &&
                !WPrivate::FunctionPointer<Func2>::IsPointerToMemberFunction,bool>::type
                connect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, Func2 slot,
                        WT::ConnectionType type = WT::DirectConnection)
        {
            return connect(sender, signal, sender, slot,type);
        }

        //connect to a function pointer  (not a member)
        template <typename Func1, typename Func2>
        static inline typename std::enable_if<int(WPrivate::FunctionPointer<Func2>::ArgumentCount) >= 0 &&
                            !WPrivate::FunctionPointer<Func2>::IsPointerToMemberFunction,bool>::type
                connect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, const wobject *context, Func2 slot,
                        WT::ConnectionType type = WT::DirectConnection)
        {
            typedef WPrivate::FunctionPointer<Func1> SignalType;
            typedef WPrivate::FunctionPointer<Func2> SlotType;

            static_assert(int(SignalType::ArgumentCount) >= int(SlotType::ArgumentCount),
                          "The slot requires more arguments than the signal provides.");
            static_assert((WPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value),
                          "Signal and slot arguments are not compatible.");
            static_assert((WPrivate::AreArgumentsCompatible<typename SlotType::ReturnType, typename SignalType::ReturnType>::value),
                          "Return type of the slot is not compatible with the return type of the signal.");

            return connectImpl(sender, reinterpret_cast<void **>(&signal), context, nullptr,
                               new WPrivate::WStaticSlotObject<Func2,
                                                     typename WPrivate::List_Left<typename SignalType::Arguments, SlotType::ArgumentCount>::Value,
                                                     typename SignalType::ReturnType>(slot) ,type);
        }

        //connect to a functor
        template <typename Func1, typename Func2>
        static inline typename std::enable_if<WPrivate::FunctionPointer<Func2>::ArgumentCount == -1,bool>::type
                connect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, Func2 slot,
                        WT::ConnectionType type = WT::DirectConnection)
        {
            return connect(sender, signal, sender, std::move(slot),type);
        }

        //connect to a functor, with a "context" object defining in which event loop is going to be executed
        template <typename Func1, typename Func2>
        static inline typename std::enable_if<WPrivate::FunctionPointer<Func2>::ArgumentCount == -1,bool>::type
                connect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, const wobject *context, Func2 slot,
                        WT::ConnectionType type = WT::DirectConnection)
        {
            typedef WPrivate::FunctionPointer<Func1> SignalType;
            const int FunctorArgumentCount = WPrivate::ComputeFunctorArgumentCount<Func2 , typename SignalType::Arguments>::Value;

            static_assert((FunctorArgumentCount >= 0),
                          "Signal and slot arguments are not compatible.");

            const int SlotArgumentCount = (FunctorArgumentCount >= 0) ? FunctorArgumentCount : 0;
            typedef typename WPrivate::FunctorReturnType<Func2, typename WPrivate::List_Left<typename SignalType::Arguments, SlotArgumentCount>::Value>::Value SlotReturnType;

            static_assert((WPrivate::AreArgumentsCompatible<SlotReturnType, typename SignalType::ReturnType>::value),
                          "Return type of the slot is not compatible with the return type of the signal.");

            return connectImpl(sender, reinterpret_cast<void **>(&signal), context, nullptr,
                               new WPrivate::WFunctorSlotObject<Func2, SlotArgumentCount,
                                    typename WPrivate::List_Left<typename SignalType::Arguments, SlotArgumentCount>::Value,
                                    typename SignalType::ReturnType>(std::move(slot)),type);
        }

/********************************************************disconnect*********************************************************************/

        template <typename Func1, typename Func2>
        static inline bool disconnect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                      const typename WPrivate::FunctionPointer<Func2>::Object *receiver, Func2 slot)
        {
            typedef WPrivate::FunctionPointer<Func1> SignalType;
            typedef WPrivate::FunctionPointer<Func2> SlotType;

            //compilation error if the arguments does not match.
            static_assert((WPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value),
                          "Signal and slot arguments are not compatible.");

            return disconnectImpl(sender, reinterpret_cast<void **>(&signal),
                                  receiver, reinterpret_cast<void **>(&slot));
        }

        template <typename Func1>
        static inline bool disconnect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                      const wobject *receiver, void **zero)
        {
            // This is the overload for when one wish to disconnect a signal from any slot. (slot=nullptr)
            // Since the function template parameter cannot be deduced from '0', we use a
            // dummy void ** parameter that must be equal to 0
            W_ASSERT(!zero);
            typedef WPrivate::FunctionPointer<Func1> SignalType;
            return disconnectImpl(sender, reinterpret_cast<void **>(&signal), receiver, zero);
        }

protected:
    wobject * sender() const;
    bool isSignalConnected(void * signal) const;
protected:
    explicit wobject(WObjectPrivate &dd);

protected:
    std::unique_ptr<WObjectData> d_ptr;
    friend struct WMetaObject;
    friend struct WObjectData;

private:
    W_DISABLE_COPY(wobject)

private:

    static bool connectImpl(const wobject *sender, void ** signal,
                            const wobject *receiver, void ** slot,
                             WPrivate::WSlotObjectBase * slotObj,WT::ConnectionType type);

    static bool disconnectImpl(const wobject *sender,
                               void ** signal,
                               const wobject *receiver,
                               void ** slot);
};

#endif
