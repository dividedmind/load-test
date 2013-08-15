#ifndef LOAD_H
#define LOAD_H

// global init, return nonzero on error
int load_test_init(int argc, char **argv);

// per-thread init, return nonzero on error
// remember to declare private pragmas where appropriate
int load_test_prepare();

// run a single case
// return nonzero to abort the test
int load_test_run();

#endif
