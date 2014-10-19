#pragma once

#include <stddef.h>

struct dictionary;
struct forth {
    size_t count;
    struct dictionary* normal;
    double stack[1024];
};
typedef void (*word_fn)(struct forth*, void*);

void forth_eval(struct forth*, const char*, size_t);

void forth_push(struct forth*, double);
double forth_pop(struct forth*);

void forth_add_normal(struct forth*, const char*, word_fn, void*);
