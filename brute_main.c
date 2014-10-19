#include "forth.h"

#include <assert.h>   // assert
#include <ctype.h>    // isspace
#include <stdio.h>    // EOF, getchar, printf
#include <string.h>   // bzero

#define UNUSED __attribute__((unused))

static void push(struct forth* f, double v) { forth_push(f, v); }
static double pop(struct forth* f) { return forth_pop(f); }

#define WORD(word) static void word(struct forth* f, void* arg UNUSED)

WORD(add) { double b=pop(f), a=pop(f); push(f, a+b); }
WORD(sub) { double b=pop(f), a=pop(f); push(f, a-b); }
WORD(mul) { double b=pop(f), a=pop(f); push(f, a*b); }
WORD(div) { double b=pop(f), a=pop(f); push(f, a/b); }
WORD(drop) { pop(f); }
WORD(dup)  { double v=pop(f); push(f,v); push(f,v); }
WORD(rot)  { double a=pop(f), b=pop(f), c=pop(f); push(f,b); push(f,a); push(f,c); }
WORD(swap) { double a=pop(f), b=pop(f); push(f,a); push(f,b); }

int main(int argc UNUSED, char** argv UNUSED) {
    char buf[128] = {0};
    size_t len = 0;

    struct forth f = {0, 0, {0}};
    forth_add_normal(&f, "+", add, NULL);
    forth_add_normal(&f, "-", sub, NULL);
    forth_add_normal(&f, "*", mul, NULL);
    forth_add_normal(&f, "/", div, NULL);
    forth_add_normal(&f, "drop", drop, NULL);
    forth_add_normal(&f,  "dup",  dup, NULL);
    forth_add_normal(&f,  "rot",  rot, NULL);
    forth_add_normal(&f, "swap", swap, NULL);

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
