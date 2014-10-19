#pragma once

#include <stddef.h>

struct forth {
    double stack[1024];
    size_t count;
};

void forth_push(struct forth*, double);
double forth_pop(struct forth*);
