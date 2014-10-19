#include "forth.h"

#include <assert.h>   // assert
#include <ctype.h>    // isspace
#include <stdio.h>    // EOF, getchar, printf
#include <string.h>   // bzero

static void pop(struct forth* f) { forth_pop(f); }
static void add(struct forth* f) { double b = forth_pop(f), a = forth_pop(f); forth_push(f, a+b); }
static void sub(struct forth* f) { double b = forth_pop(f), a = forth_pop(f); forth_push(f, a-b); }
static void mul(struct forth* f) { double b = forth_pop(f), a = forth_pop(f); forth_push(f, a*b); }
static void div(struct forth* f) { double b = forth_pop(f), a = forth_pop(f); forth_push(f, a/b); }

#define UNUSED __attribute__((unused))

int main(int argc UNUSED, char** argv UNUSED) {
    char buf[128] = {0};
    size_t len = 0;

    struct forth f = {0, 0, {0}};
    forth_add_normal(&f, "pop", pop);
    forth_add_normal(&f, "+", add);
    forth_add_normal(&f, "-", sub);
    forth_add_normal(&f, "*", mul);
    forth_add_normal(&f, "/", div);

    for (int c = getchar(); c != EOF; c = getchar()) {
        if (len > 0 && isspace(c)) {
            forth_eval(&f, buf, len);
            bzero(buf, len);
            len = 0;
            for (size_t i = 0; '\n' == c && i < f.count; i++) {
                printf("%g%c", f.stack[i], i == f.count - 1 ? '\n' : ' ');
            }
            continue;
        }

        assert (len < sizeof(buf));
        buf[len++] = (char)c;
    }
    return 0;
}
