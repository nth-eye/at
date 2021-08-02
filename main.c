#include <stdio.h>
#include "at.h"

#define SIZE(x) (sizeof(x) / sizeof(x[0]))

void test(AT *at)
{
    printf("<%s>: %s \n", __func__, at->args ? at->args : "NO ARGS");
}

void test_ok(AT *at)
{
    printf("<%s>: OK code is %d \n", __func__, at->code);
}

void test_error(AT *at)
{
    printf("<%s>: ERROR code is %d \n", __func__, at->code);
}

void no_res_cbks(AT *at)
{
    printf("<%s>: %s \n", __func__, at->args ? at->args : "NO ARGS");
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
        .cbks_size = SIZE(callbacks),
        .buf_size = SIZE(buf),
    };

    AT at;

    at_init(&at, &cfg);

    while (1) {
        at_process(&at);
    }
}
