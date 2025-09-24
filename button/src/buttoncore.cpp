#include "buttoncore_p.hpp"

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
