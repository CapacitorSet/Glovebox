#ifndef FLOW_CONTROL_H
#define FLOW_CONTROL_H
#include <functional>
#include <tfhe.h>

using maskable_function_t = std::function<void(bit_t)>;

void _if(bit_t cond, maskable_function_t iftrue);
void _if_else(bit_t cond, maskable_function_t iftrue,
              maskable_function_t iffalse,
              TFHEServerParams_t = default_server_params);
maskable_function_t _m_if(bit_t cond, maskable_function_t iftrue,
                          TFHEServerParams_t = default_server_params);
maskable_function_t _m_if_else(bit_t cond, maskable_function_t iftrue,
                               maskable_function_t iffalse,
                               TFHEServerParams_t = default_server_params);
#endif