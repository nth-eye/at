#include <stdio.h>
#include "at.h"

#define SIZE(x) (sizeof(x) / sizeof(x[0]))

void test(AT *at)
{
    printf("\n +TEST: %s\n", at->args ? at->args : "NO ARGS");
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
    printf("\n #NORESCBKS: %s\n", at->args ? at->args : "NO ARGS");
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

    AT_Init(&at, &cfg);
    AT_Process(&at);
    // AT_Send("AT+TEST: %d", 777);

    while (1) {
        AT_Process(&at);
    }
}
