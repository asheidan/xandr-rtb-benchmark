# Xandr RTB Benchmark

This is a minimal setup for benchmarking different listener implementations with respect to how they perform as the number of open sockets (active connections) increases.

All the implementations are in `implementations/`.

## The benchmark

Simple C program that opens the specified number of connections to the listener and then uses random allocation to determine which connection to send each message through.

The result is the number of seconds the test-run took.

### Compiling

As the benchmark is implemented in C there's a Makefile to compile the program. The makefile is also used to manage the test runs.

The following variables might be of interest in the Makefile:

- __IMPLEMENTATIONS__: This should be a list of the implementations that are available and should be run. The text in this list does not directly correspond to the filename of the implementation.
- __DATASET_CONNECTIONS__: This is a list of datapoints that should be used during the test run. The benchmark will be run (at least) once for each of these datapoints.
