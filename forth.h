#pragma once

#include <stddef.h>

struct forth;

typedef void (*word_fn)(struct forth*);

struct dictionary {
    struct dictionary* next;
    const char* identifier;
    word_fn word;
};

struct forth {
    size_t count;
    struct dictionary* normal;
    double stack[1024];
};

void forth_eval(struct forth*, const char*, size_t);

void forth_push(struct forth*, double);
double forth_pop(struct forth*);

void forth_add_normal(struct forth*, const char*, void(*)(struct forth*));
