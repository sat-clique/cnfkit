#include <cnfkit/Literal.h>

#include <type_traits>

namespace cnfkit {

// Check whether lit and var are regular types (+ noexcept):
static_assert(std::is_nothrow_default_constructible_v<var>);
static_assert(std::is_nothrow_copy_assignable_v<var>);
static_assert(std::is_nothrow_copy_constructible_v<var>);
static_assert(std::is_nothrow_destructible_v<var>);
static_assert(std::is_nothrow_move_assignable_v<var>);
static_assert(std::is_nothrow_move_constructible_v<var>);
static_assert(std::is_nothrow_swappable_v<var>);

static_assert(std::is_nothrow_default_constructible_v<lit>);
static_assert(std::is_nothrow_copy_assignable_v<lit>);
static_assert(std::is_nothrow_copy_constructible_v<lit>);
static_assert(std::is_nothrow_destructible_v<lit>);
static_assert(std::is_nothrow_move_assignable_v<lit>);
static_assert(std::is_nothrow_move_constructible_v<lit>);
static_assert(std::is_nothrow_swappable_v<lit>);

// TODO: check for comparability

}
