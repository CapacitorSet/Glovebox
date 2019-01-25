FHE-tools
=========

A high-level wrapper around [libtfhe](https://github.com/tfhe/tfhe/).

Check out the examples folder to see it in action.

## Usage

 * Install TFHE v1 (https://github.com/tfhe/tfhe/tree/v1.0.1)
 * Install the GSL
 * `cmake . -DPLAINTEXT=0` (PLAINTEXT=1 uses a mock implementation and enables debug flags)
 * `make`

## Tests

 * In addition to TFHEv1 and the GSL, clone [Rapidcheck](https://github.com/emil-e/rapidcheck/)
 * `cmake . -DPLAINTEXT=0 -DTESTS_ENABLED=1`
 * `make fhetools_test`
 * `bin/fhetools_test`

## Misc notes

Uses a custom version of gsl::span for syntactic reasons. If it ever needs to be updated, apply the patch in gsl_span.diff to gsl/span from the Microsoft GSL.