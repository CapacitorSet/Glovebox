#include "debug.h"
#include "compile_time_settings.h"

#if DEBUG

#if IS_PLAINTEXT
char decrypt(bits_t bit, TFHEParams p) { return *bit; }
#else
#include "tfhe.h"

char decrypt(bits_t bit, TFHEServerParams_t p) {
  return bootsSymDecrypt(bit, p.secret_key);
}
#endif

#endif