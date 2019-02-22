To do's
=======

[ ] Add support for constructing types from client params; change TFHEServerParams_t to only_TFHEServerParams_t in constructors to avoid insecure usages

[x] Implement Int as a template class, with instances of Int8, Int16, Int32

[x] Drop Varint

[x] Implement Fixed as a template class, with instances of Q4.4, Q8.8, Q16.16, maybe with Q0.x or Q1.x too?

[x] Drop ClientInt, instance Int/Fixed from raw bitspans

[ ] Implement String, instance from raw bitspan

[ ] Add support for arrays

[ ] Add control flow (`if`, `while`, maybe `switch`)

[ ] Fix free_bitspan

[ ] Add support for parallel computing in high-level, expensive operations