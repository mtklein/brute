#include "forth.h"

#include <assert.h>   // assert
#include <stdlib.h>   // malloc, strtod
#include <string.h>   // strdup, strncmp

void forth_push(struct forth* f, double v) {
    assert (f->count < sizeof(f->stack) / sizeof(f->stack[0]));
    f->stack[f->count++] = v;
}

double forth_pop(struct forth* f) {
    return f->count == 0 ? 0 : f->stack[--f->count];
}

void forth_add_normal(struct forth* f, const char* identifier, word_fn word) {
    struct dictionary* d = malloc(sizeof(*d));
    d->next       = f->normal;
    d->identifier = strdup(identifier);
    d->word       = word;

    f->normal = d;
}

static word_fn lookup(const struct forth* f, const char* tok, size_t len) {
    for (const struct dictionary* d = f->normal; d != NULL; d = d->next) {
        if (0 == strncmp(d->identifier, tok, len)) {
            return d->word;
        }
    }
    return NULL;
}

void forth_eval(struct forth* f, const char* tok, size_t len) {
    char* end;
    double v = strtod(tok, &end);
    if (end == tok + len) {
        forth_push(f, v);
    }

    word_fn word = lookup(f, tok, len);
    if (word != NULL) {
        word(f);
    }
}

