#ifndef FHE_TOOLS_COMPILE_TIME_SETTINGS_H
#define FHE_TOOLS_COMPILE_TIME_SETTINGS_H

#define IS_PLAINTEXT (0)
#define DEBUG (1)

#ifndef IS_PLAINTEXT
#error IS_PLAINTEXT must be defined.
#endif
#ifndef DEBUG
#error DEBUG must be defined.
#endif
#endif //FHE_TOOLS_COMPILE_TIME_SETTINGS_H
