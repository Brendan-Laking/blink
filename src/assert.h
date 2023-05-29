#ifndef ASSERT_H_
#define ASSERT_H_

#define ASSERT(_condition)                      \
    if (!(_condition)) {                        \
        fprintf(stderr, "Assertion failed.\n"); \
        exit(1);                                \
    }
    
#endif