#ifndef BUTTON_CORE_P_HPP
#define BUTTON_CORE_P_HPP 1

#include <buttoncore.hpp>
#include <xcontainer.hpp>
#include <button.hpp>
#include <ranges>
#include <xhelper.hpp>

class ButtonCorePrivate final: public ButtonCoreData {
    W_DECLARE_PUBLIC(ButtonCore)
    XUnordered_map<void *,XSharedPtr<Button>> m_buttons{};

public:
    explicit ButtonCorePrivate(ButtonCore * const o)
    { q_ptr = o; }
    ~ButtonCorePrivate() override = default;
    void addButton(XSharedPtr<Button> const &) noexcept;
    void removeButton(void *) noexcept;
    void exec() const noexcept;
};

inline void ButtonCorePrivate::addButton(XSharedPtr<Button> const & b) noexcept
{ CHECK_EMPTY(b,return);m_buttons[b.get()] = b; }

inline void ButtonCorePrivate::removeButton(void * const d) noexcept
{ CHECK_EMPTY(d,return); m_buttons.erase(d); }

inline void ButtonCorePrivate::exec() const noexcept {
    for (const auto & v : m_buttons | std::views::values) {
        v->exec_();
    }
}

#endif
