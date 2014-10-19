#include "forth.h"

#include <assert.h>   // assert
#include <ctype.h>    // isspace
#include <stdio.h>    // EOF, getchar, printf
#include <string.h>   // bzero

#define UNUSED __attribute__((unused))

static void push(struct forth* f, double v) { forth_push(f, v); }
static double pop(struct forth* f) { return forth_pop(f); }

static void add(struct forth* f, void* arg UNUSED) { double b = pop(f), a = pop(f); push(f, a+b); }
static void sub(struct forth* f, void* arg UNUSED) { double b = pop(f), a = pop(f); push(f, a-b); }
static void mul(struct forth* f, void* arg UNUSED) { double b = pop(f), a = pop(f); push(f, a*b); }
static void div(struct forth* f, void* arg UNUSED) { double b = pop(f), a = pop(f); push(f, a/b); }

static void drop(struct forth* f, void* arg UNUSED) { pop(f); }

int main(int argc UNUSED, char** argv UNUSED) {
    char buf[128] = {0};
    size_t len = 0;

    struct forth f = {0, 0, {0}};
    forth_add_normal(&f, "+", add, NULL);
    forth_add_normal(&f, "-", sub, NULL);
    forth_add_normal(&f, "*", mul, NULL);
    forth_add_normal(&f, "/", div, NULL);
    forth_add_normal(&f, "drop", drop, NULL);

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
