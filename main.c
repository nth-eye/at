#include <stdio.h>
#include "at.h"

void test(AT *at)
{
    printf("\n-- +TEST");
    if (at->args)
        printf(": %s\n", at->args);
    printf(" --\n");
}

void test_ok(AT *at)
{
    printf("\n---- OK code: %d ----\n", at->code);
}

void test_error(AT *at)
{
    printf("\n---- ERROR code: %d ----\n", at->code);
}

void no_res_cbks(AT *at)
{
    printf("\n-- #NORESCBKS");
    if (at->args)
        printf(": %s\n", at->args);
    printf(" --\n");
}

int main(void) 
{
    AT_Callback test_res_cbks[] = { test_ok, NULL, NULL, NULL, test_error, NULL, NULL, NULL, NULL };
    AT_Cmd callbacks[] = { 
        { "+TEST",      test,           test_res_cbks },
        { "#NORESCBKS", no_res_cbks,    NULL },
    };
    char buf[128];

    const AT_Cfg cfg = {
        .cbks = callbacks,
        .buf = buf, 
        .n_cbks = SIZE(callbacks),
        .buf_size = SIZE(buf),
    };

    AT at;

    AT_Init(&at, &cfg);
    AT_Process(&at);

    while (1) {
        AT_Process(&at);
    }
}
