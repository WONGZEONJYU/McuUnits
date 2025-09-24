#pragma once
#ifndef W_OBJECT_HPP
#define W_OBJECT_HPP 1

#include <wobjectdefs.hpp>
#include <wglobal.hpp>
#include <wnamespace.hpp>
#include <memory>

struct WMetaObject;
class WObjectPrivate;
class WObject;

class WObjectData {
    W_DISABLE_COPY(WObjectData)
public:
    WObjectData() = default;
    virtual ~WObjectData() = 0;
    WObject *q_ptr{};
    uBase_Type blockSig:1{};
};

class WObject {
    W_DECLARE_PRIVATE(WObject)
public:
    explicit WObject() noexcept;
    virtual ~WObject();

    [[nodiscard]] bool signalsBlocked() const noexcept{return d_ptr->blockSig;}
    bool blockSignals(bool block) noexcept;

    template <typename Func1, typename Func2>
        static bool connect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                         const typename WPrivate::FunctionPointer<Func2>::Object *receiver, Func2 slot,
                                         WT::ConnectionType type = WT::DirectConnection)
        {
            typedef WPrivate::FunctionPointer<Func1> SignalType;
            typedef WPrivate::FunctionPointer<Func2> SlotType;

            static_assert(static_cast<int>(SignalType::ArgumentCount) >= static_cast<int>(SlotType::ArgumentCount),
                          "The slot requires more arguments than the signal provides.");

            static_assert(WPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value,
                          "Signal and slot arguments are not compatible.");
            static_assert(WPrivate::AreArgumentsCompatible<typename SlotType::ReturnType, typename SignalType::ReturnType>::value,
                          "Return type of the slot is not compatible with the return type of the signal.");

            return connectImpl(sender, reinterpret_cast<void **>(&signal),
                               receiver, reinterpret_cast<void **>(&slot),
                               new WPrivate::WSlotObject<Func2, typename WPrivate::List_Left<typename SignalType::Arguments, SlotType::ArgumentCount>::Value,
                                               typename SignalType::ReturnType>(slot),
                                               type);
        }

        //connect to a function pointer  (not a member)
        template <typename Func1, typename Func2>
        static std::enable_if_t<static_cast<int>(WPrivate::FunctionPointer<Func2>::ArgumentCount) >= 0 &&
                !WPrivate::FunctionPointer<Func2>::IsPointerToMemberFunction,bool>
                connect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, Func2 slot,
                        WT::ConnectionType type = WT::DirectConnection)
        { return connect(sender, signal, sender, slot,type); }

        //connect to a function pointer  (not a member)
        template <typename Func1, typename Func2>
        static std::enable_if_t<static_cast<int>(WPrivate::FunctionPointer<Func2>::ArgumentCount) >= 0 &&
                            !WPrivate::FunctionPointer<Func2>::IsPointerToMemberFunction,bool>
                connect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, const WObject *context, Func2 slot,
                        WT::ConnectionType type = WT::DirectConnection)
        {
            using SignalType = WPrivate::FunctionPointer<Func1> ;
            using SlotType = WPrivate::FunctionPointer<Func2> ;

            static_assert(static_cast<int>(SignalType::ArgumentCount) >= static_cast<int>(SlotType::ArgumentCount),
                          "The slot requires more arguments than the signal provides.");
            static_assert(WPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value,
                          "Signal and slot arguments are not compatible.");
            static_assert(WPrivate::AreArgumentsCompatible<typename SlotType::ReturnType, typename SignalType::ReturnType>::value,
                          "Return type of the slot is not compatible with the return type of the signal.");

            return connectImpl(sender, reinterpret_cast<void **>(&signal), context, nullptr,
                               new WPrivate::WStaticSlotObject<Func2,
                                                     typename WPrivate::List_Left<typename SignalType::Arguments, SlotType::ArgumentCount>::Value,
                                                     typename SignalType::ReturnType>(slot) ,type);
        }

        //connect to a functor
        template <typename Func1, typename Func2>
        static std::enable_if_t<WPrivate::FunctionPointer<Func2>::ArgumentCount == -1,bool>
                connect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, Func2 slot,
                        WT::ConnectionType type = WT::DirectConnection)
        {
            return connect(sender, signal, sender, std::move(slot),type);
        }

        //connect to a functor, with a "context" object defining in which event loop is going to be executed
        template <typename Func1, typename Func2>
        static std::enable_if_t<WPrivate::FunctionPointer<Func2>::ArgumentCount == -1,bool>
                connect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, const WObject *context, Func2 slot,
                        WT::ConnectionType type = WT::DirectConnection)
        {
            using SignalType = WPrivate::FunctionPointer<Func1> ;
            constexpr int FunctorArgumentCount { WPrivate::ComputeFunctorArgumentCount<Func2 , typename SignalType::Arguments>::Value};

            static_assert(FunctorArgumentCount >= 0,
                          "Signal and slot arguments are not compatible.");

            constexpr int SlotArgumentCount { FunctorArgumentCount >= 0 ? FunctorArgumentCount : 0 };
            using SlotReturnType = typename WPrivate::FunctorReturnType<Func2, typename WPrivate::List_Left<typename SignalType::Arguments, SlotArgumentCount>::Value>::Value;

            static_assert(WPrivate::AreArgumentsCompatible<SlotReturnType, typename SignalType::ReturnType>::value,
                          "Return type of the slot is not compatible with the return type of the signal.");

            return connectImpl(sender, reinterpret_cast<void **>(&signal), context, nullptr,
                               new WPrivate::WFunctorSlotObject<Func2, SlotArgumentCount,
                                    typename WPrivate::List_Left<typename SignalType::Arguments, SlotArgumentCount>::Value,
                                    typename SignalType::ReturnType>(std::move(slot)),type);
        }

/********************************************************disconnect*********************************************************************/

        template <typename Func1, typename Func2>
        static bool disconnect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                      const typename WPrivate::FunctionPointer<Func2>::Object *receiver, Func2 slot) noexcept
        {
            using SignalType = WPrivate::FunctionPointer<Func1>;
            using SlotType = WPrivate::FunctionPointer<Func2>;

            //compilation error if the arguments does not match.
            static_assert(WPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value,
                          "Signal and slot arguments are not compatible.");

            return disconnectImpl(sender, reinterpret_cast<void **>(&signal),
                                  receiver, reinterpret_cast<void **>(&slot));
        }

        template <typename Func1>
        static bool disconnect(const typename WPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                      const WObject *receiver, void **zero)
        {
            // This is the overload for when one wish to disconnect a signal from any slot. (slot=nullptr)
            // Since the function template parameter cannot be deduced from '0', we use a
            // dummy void ** parameter that must be equal to 0
            W_ASSERT(!zero);
            //using SignalType = WPrivate::FunctionPointer<Func1> ;
            return disconnectImpl(sender, reinterpret_cast<void **>(&signal), receiver, zero);
        }

protected:
    [[nodiscard]] WObject * sender() const;
    bool isSignalConnected(void * signal) const;
    explicit WObject(WObjectPrivate &dd);
    std::unique_ptr<WObjectData> d_ptr{};
    friend struct WMetaObject;
    friend class WObjectData;

private:
    W_DISABLE_COPY(WObject)

    static bool connectImpl(const WObject *sender, void ** signal,
                            const WObject *receiver, void ** slot,
                             WPrivate::WSlotObjectBase * slotObj,WT::ConnectionType type);

    static bool disconnectImpl(const WObject *sender,
                               void ** signal,
                               const WObject *receiver,
                               void ** slot);
};

#endif
