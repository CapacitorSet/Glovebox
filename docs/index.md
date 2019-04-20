# FHE-Tools

FHE-Tools is a C++ library for developing applications with privacy-by-design. It implements [fully homomorphic encryption](https://en.wikipedia.org/wiki/Homomorphic_encryption) by wrapping [libtfhe v1.0](https://github.com/tfhe/tfhe/).

## Learning by example

It features several [examples](https://github.com/CapacitorSet/FHE-tools/tree/master/examples/) that you can check out to get a feel of how to develop applications with privacy-by-design using FHE-Tools. In order of difficulty they are roughly:

  1. Fibonacci, where the server computes the n-th Fibonacci number from a user-provided seed
  2. Linreg, where the server computes a polynomial over user-provided points
  3. Contacts, where the server tells the client which of its contacts it already knows
  4. Health, where the server computes statistics and queries over user-provided health data

## Documentation

### General principles

FHE-Tools tries to replicate the functions and feel of a standard programming language. There are however a few systematic differences due to the impossibility to access user data: in general you can't simply branch based on secret (user-provided) data. Indeed, if you think about it, being able to branch based on secret data would allow for timing attacks that reveal the content of that secret data. The way TFHE solves this problem is by using *masks*: branches are always executed, and they are passed a bit that controls whether they are effective or not (eg. whether they will write to memory or not).

From the point of view of the user, this reflects in branch-heavy code being very slow (notably array access with a secret index), which the programmer must minimize; from the point of view of the programmer, this reflects in control flow being somewhat clunky, requiring to pass masks around.

An important consequence is that loops must have a known upper bound on the number of times they are executed (as doing otherwise would be branching on secret data).

From the point of view of syntax, FHE-Tools tries to be as lightweight as possible, especially by making use of default parameters and conversions. The principles in what to make default are that when possible parameters should be omitted, and that for functions that encode values (and thus could take server parameters to do trivial encryption or client parameters to do actual encryptions) the omitted parameter should be the client one (so that client code, which typically makes the heaviest use of initialization, can be syntactically lighter).

Because FHE is slow, you can test it on plaintext data by compiling with `DEBUG=1`; it is guaranteed to behave the same (and tested to do so). It exposes the exact same interface.

### Basics

Use `examples/keygen` to generate a public/private keypair. To read the keys as a client, use `TFHEClientParams_t makeTFHEClientParams(FILE*)`; as a server, `TFHEServerParams_t makeTFHEServerParams(FILE*)`.

You'll usually need to pass the keys around to functions; however, for simplicity they can often be omitted on the condition that you call them `default_client_params` and `default_server_params` respectively if you're a client or a server.

>For instance, if your server params were called `p` you'd write `Int8 a(123, p)` to create a new `Int8`; if you called the params `default_server_params` you could simply write `Int8 a(123)`.

>"Why can't the params always be omitted?" Because it can sometimes be ambiguous whether the default parameter would be `default_server_params` or `default_client_params`.

Note that all functions that accept `TFHEServerParams_t` in the prototype can also accept `TFHEClientParams_t` with an implicit conversion. On the other hand, you can *only* pass `TFHEServerParams_t` to functions that accept `only_TFHEServerParams_t` in the prototype. This constraint is introduced in functions that encrypt data trivially and for this reason they must not be called from client code.

### Bits

The fundamental data type is `bit_t`, which represents a secret bit. You can allocate one with `bit_t make_bit(TFHEServerParams_t)` or `bit_t make_bit(TFHEClientParams_t)`; note that are exactly equivalent. They are internally implemented as smart pointers, so they are deallocated automatically.

```c++
FILE *cloud_key = fopen("cloud.key", "rb");
p = makeTFHEClientParams(cloud_key);
bit_t tmp = make_bit(p);
```

It also has a default argument of `default_server_params`:

```c++
FILE *cloud_key = fopen("cloud.key", "rb");
default_server_params = makeTFHEClientParams(cloud_key);
bit_t tmp = make_bit();
```

> From now on, a default argument of `default_server_params` will be represented as eg. `make_bit([TFHEServerParams_t])`; a default argument of `default_client_params` will be represented as eg. `make_bit([TFHEClientParams_t])`.

When created, a bit is uninitialized. As a client you can write a value into it with `void encrypt(bit_t dst, bool value, [TFHEClientParams_t])`, which internally will encrypt the bit; as a server you can write a value into it with `void constant(bit_t dst, bool value, [only_TFHEServerParams_t])`, which internally will use a **trivial** encryption. Take care not to accidentally call `constant` with `TFHEClientParams_t` via implicit conversions! Client applications are **strongly suggested** to defend from this by compiling with `-DSTRICT_CLIENT_MODE=1`, which will cause calls to `constant()` to crash at runtime.

If you must use `constant()` and you know that your usage is safe (eg. in `include/parallel.h` which implements `any_of` which clients might want to use), use `constant<true>()` which will override `STRICT_CLIENT_MODE`.

Clients can decrypt bits with `bool decrypt(bit_t, [TFHEClientParams_t])`:

```c++
bit_t tmp = make_bit();
encrypt(tmp, false);
std::cout << decrypt(tmp) << std::endl; // Prints "0"
```

The TFHE primitives are exposed directly as `_and(bit_t dst, bit_t a, bit_t b, [TFHEServerParams_t])`, `_andyn`, `_mux` and so on.

### Bitspans

A contiguous array of bits is a `bitspan_t`. Allocate bitspans with `bitspan_t make_bitspan(int size, [TFHEServerParams_t])` or `bitspan_t make_bitspan(int size, TFHEClientParams_t)`; they are deallocated automatically.

You can access individual bits in a bitspan the standard way, with the [] operator:

```c++
bitspan_t coinFlips = make_bitspan(2, default_client_params);
encrypt(coinFlips[0], true);
encrypt(coinFlips[1], false);
```

You can also iterate over bitspans:

```c++
puts("Coin flips:");
for (const auto &bit : coinFlips) {
	puts(decrypt(bit) ? "Heads" : "Tails");
}
```

### Serialization

To transmit bits you can use `std::string exportToString(bitspan_t src, TFHEServerParams_t)` to serialize a bit/bitspan as a string; `void serialize(std::ostream &output, bitspan_t src, TFHEServerParams_t)` to serialize it to a stream (likewise with `TFHEClientParams_t`).

Deserialize bits from strings with `bit_t make_bit(const std::string &string, [TFHEServerParams_t])` and bitspans from strings `bitspan_t make_bitspan(int N, const std::string &string, [TFHEServerParams_t])`; deserialize bits/bitspans from streams with `void deserialize(std::istream &input, bitspan_t src, TFHEClientParams_t p)`.

### Ints

Todo

### Fixed-point numbers

Todo

### Strings

Todo

### Polynomials

Todo

### Arrays

Todo

### Structs

Todo

### Control flow

Todo

### Parallelization

Todo
