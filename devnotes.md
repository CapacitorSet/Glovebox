# Developer notes

This is a collection of notes useful for developing for and with this library. It is not yet a complete documentation, but it might become one sometime.

## Parameters

Client params are automatically cast to server params, removing the need for a lot of syntactical overhead. However this can cause API misuse leading to security issues: writing to bits using client parameters encrypts the values, whereas using server parameters will write "unsafe" values.

This makes sense from a theoretical standpoint (the client produces bits that must be kept secret, the server doesn't), but is obviously dangerous in practice. For this reason `only_TFHEServerParams_t` can be used to ensure that the user passes specifically server params, with automatic casts being impossible. You can in turn convert `only_TFHEServerParams_t` to `TFHEServerParams_t` (for functions that only accept the latter) using `unwrap_only`.

(A quick check for the above issue by compiling with PLAINTEXT=0 and running your code. If you have API misuse it will run instantly, otherwise it will take some time ranging from seconds to... ages, sigh.)

The compile flag `STRICT_CLIENT_MODE` was added to automatically protect against API misuse: it will print an error and crash at runtime if `constant()` is used. For this reason, **it is strongly recommended to enable it in client code**: `-DSTRICT_CLIENT_MODE=1`.

## Arrays

All types that can be put into an array must have a `static int wordSize` representing that type's size in bits and expose `data`. Furthermore, if you want to serialize arrays (which is almost always the case) the type must also implement `static char typeID`. It can be any value you want, it's just used as a sanity check when transmitting data.

You can put a value into an array with `void put(T, int)` or `void put(T, Int)` (todo: figure out how to support multiple ints. perhaps with `smallest_Int<N>`?), with the former being definitely recommended for addresses known at compile time.

Accessing an array past its end is undefined behaviour, though it might be interesting to add a debug flag/method/? to assert that this doesn't happen.

## Debugging

Especially in plaintext mode (which you should always use for debugging) ASan and UBSan do a rather good job of detecting leaks and reads from undefined memory. Specifically, when UBSan warns about writing weird numeric values to a bool it means that you're reading uninitialized bitspans.

If you use ASan you must use `ASAN_OPTIONS=alloc_dealloc_mismatch=0`, because the shared pointers used in bitspans call the default destructor rather than `free()`. This is a conscious choice to reduce the memory overhead of bitspans at no cost.

## Parallel computing

Some operations can be computed in parallel using `parallel.h`, however it is an experimental feature inside a library which is itself very experimental. It uses a simple first-come-first-serve work scheduling to distribute tasks over RPC. Note that the RPC layer is rather fragile; refer to the "Typical errors > Runtime" section for what to expect.

To use parallel functions you must define a global `std::vector<parallel_host_t> parallel_hosts`, with `parallel_host_t` being:

```cpp
struct {
	std::string address;
	uint16_t port;
}
```

Compilation will fail with a linker error otherwise.

## Typical errors

### Compile time

**Linker errors about default_server_params** (or more rarely about default_client_params) mean that somewhere you used a function call with an implicit TFHEServerParams_t parameter, and yet you didn't define `default_server_params`.

  * If indeed you did not intend to use `default_server_params`, track down the function call that has an implicit parameter.
  * If you intend to use implicit function parameters, you must define `default_server_params` and assign it a value when you have read the keys.

### Runtime

**terminate() called without an active exception** happens when you try to access a bitspan past its bounds.

**illegal hardware instruction** when in plaintext mode happens when you try to load a non-bool into a bit. This most likely means that you're copying from uninitialized bits (possibly via logic gates); a debugger will be relatively helpful in finding the root cause.

> The "illegal hardware instruction" is intentional: it's a trap placed by [UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).

**hanging at the start** in examples typically means that the server isn't reachable.

**rpc::rpc_error** is thrown when unexpected conditions occur during a RPC call, notably:
  * aren't RPC servers
  * don't have the method you asked for
  * have different arguments
  * have a different return type
  * were compiled with `PLAINTEXT=1` while the client is not (or vice versa)

## Misc

Memory is managed automatically: `make_bitspan` exposes a modified `gsl::span` that uses shared pointers under the hood.