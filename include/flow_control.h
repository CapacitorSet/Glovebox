#ifndef FLOW_CONTROL_H
#define FLOW_CONTROL_H
#include <functional>
#include <tfhe.h>
#include <types/int.h>

using maskable_function_t = std::function<void(bit_t)>;

void _if(bit_t cond, maskable_function_t iftrue);
void _if_else(bit_t cond, maskable_function_t iftrue, maskable_function_t iffalse);
maskable_function_t _m_if(bit_t cond, maskable_function_t iftrue);
maskable_function_t _m_if_else(bit_t cond, maskable_function_t iftrue, maskable_function_t iffalse);

/* Calls `body` for `max` times. As long as `condition` returns true, it will
 * call `body` with a high mask; when `condition` returns false, that call to
 * `body` and subsequent ones will have a low mask.
 * Returns an overflow bit, which is true if `condition` is true after `max`
 * iterations.
 */
bit_t _while(std::function<bit_t(void)> condition, uint64_t max, maskable_function_t body);

/* Calls `body` for `max` times, of which `src` with a high mask and the
 * rest with a low one. Returns an overflow bit, which is true if src > max.
 * `src` is not modified. No iterations are done if `src` < 0.
 */
[[nodiscard]] bit_t times(Int8 src, uint8_t max, maskable_function_t body);

#endif