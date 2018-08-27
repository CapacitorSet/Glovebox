#ifndef FHE_TOOLS_DEBUG_H
#define FHE_TOOLS_DEBUG_H

#include "tfhe.h"
#include "compile_time_settings.h"
#if DEBUG

char decrypt(bits_t bit, TFHEParams p);

#endif
#endif //FHE_TOOLS_DEBUG_H
