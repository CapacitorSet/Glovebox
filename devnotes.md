# Developer notes

This is a collection of notes useful for developing for and with this library. It is not yet a complete documentation, but it might become one sometime.

## Parameters

Client params are automatically cast to server params, removing the need for a lot of syntactical overhead. However this can cause API misuse leading to security issues: writing to bits using client parameters encrypts the values, whereas using server parameters will write "unsafe" values.

This makes sense from a theoretical standpoint (the client produces bits that must be kept secret, the server doesn't), but is obviously dangerous in practice. For this reason `only_TFHEServerParams_t` can be used to ensure that the user passes specifically server params, with automatic casts being impossible. You can in turn convert `only_TFHEServerParams_t` to `TFHEServerParams_t` (for functions that only accept the latter) using `unwrap_only`.

You can check for the above issue by compiling with PLAINTEXT=0 and running your code. If you have API misuse it will run instantly, otherwise it will take some time (ranging from seconds to... ages, sigh).

Todo: add a compile flag like SERVER_USAGE and CLIENT_USAGE, and add the respective warnings in encrypt() and constant()?

## Arrays

All types that can be put into an array must have a `static int wordSize` representing that type's size in bits and expose `data`.

You can put a value into an array with `void put(T, int)` or `void put(T, Int)` (todo: figure out how to support multiple ints. perhaps with `smallest_Int<N>`?), with the former being definitely recommended for addresses known at compile time.

Accessing an array past its end is undefined behaviour, though it might be interesting to add a debug flag/method/? to assert that this doesn't happen.