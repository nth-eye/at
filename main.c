#include <stdio.h>
#include "at.h"

#define SIZE(x)         (sizeof(x) / sizeof(x[0]))
#define LOG(fmt, ...)   printf("<%s> " fmt "\n", __func__, ##__VA_ARGS__)

void test(AT *at)
{
    LOG(": %s ", at->param);
}

int main(void) 
{
    AT_Cmd callbacks[] = { 
        { "+TEST",  test    },
    };
    char buf[128];

    const AT_Cfg cfg = {
        .cbks       = callbacks,
        .buf        = buf, 
        .cbks_size  = SIZE(callbacks),
        .buf_size   = SIZE(buf),
    };

    AT at;

    at_init(&at, &cfg);

    while (1) {

        char c = getc(stdin);

        at_process(&at, c);

        if (at.rsp == AT_RSP_OK)
            LOG("got OK");
    }
}
