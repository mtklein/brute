#include "forth.h"

#include <assert.h>

void forth_push(struct forth* f, double v) {
    assert (f->count < sizeof(f->stack) / sizeof(f->stack[0]));
    f->stack[f->count++] = v;
}

double forth_pop(struct forth* f) {
    return f->count == 0 ? 0 : f->stack[--f->count];
}
