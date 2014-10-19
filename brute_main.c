#include "forth.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void eval(struct forth* f, const char* tok, size_t len) {
    char* end;
    double v = strtod(tok, &end);
    if (end == tok + len) {
        forth_push(f, v);
    }

    if (0 == strncmp(tok, "pop", len)) {
        forth_pop(f);
    }
}

#define UNUSED __attribute__((unused))

int main(int argc UNUSED, char** argv UNUSED) {
    char buf[80] = {0};
    size_t len = 0;

    struct forth f = {{0}, 0};

    for (;;) {
        int c = fgetc(stdin);
        if (EOF == c) {
            return 0;
        }

        if (len > 0 && isspace(c)) {
            eval(&f, buf, len);
            bzero(buf, len);
            len = 0;
            for (size_t i = 0; i < f.count; i++) {
                printf("%g%c", f.stack[i], i == f.count - 1 ? '\n' : ' ');
            }
            continue;
        }

        assert (len < sizeof(buf));
        buf[len++] = (char)c;
    }
}
