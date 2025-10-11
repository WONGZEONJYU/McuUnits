#include "buttoncore_p.hpp"

void ButtonCorePrivate::addButton(XSharedPtr<Button> const & b) noexcept
{ CHECK_EMPTY(b,return);m_buttons[b.get()] = b; }

void ButtonCorePrivate::removeButton(void * const d) noexcept
{ CHECK_EMPTY(d,return); m_buttons.erase(d); }

void ButtonCorePrivate::exec() const noexcept
{ for (const auto & v : m_buttons | std::views::values) { v->exec_(); } }

ButtonCore * ButtonCore::handle() noexcept
{ return UniqueConstruction().get(); }

void ButtonCore::exec() noexcept
{ d_func()->exec(); }

void ButtonCore::push(XSharedPtr<Button> const & b) noexcept
{ d_func()->addButton(b); }

void ButtonCore::remove(void * const d) noexcept
{ d_func()->removeButton(d); }

bool ButtonCore::construct_() noexcept {
    auto dd{ makeUnique<ButtonCorePrivate>(this) };
    CHECK_EMPTY(dd,return {});
    m_d_ptr_ = std::move(dd);
    return true;
}
