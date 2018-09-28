#ifndef FHE_TOOLS_DEBUG_H
#define FHE_TOOLS_DEBUG_H

#include "compile_time_settings.h"
#include "tfhe.h"
#if DEBUG

char decrypt(bits_t bit, TFHEServerParams_t p);

#endif
#endif // FHE_TOOLS_DEBUG_H
