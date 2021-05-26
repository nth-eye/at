#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "at_platform.h"

// NOTE: Add manufacturer-specific response codes at the end.
const char *AT_RESULTS[AT_CODE_size] = { 
    "OK", "CONNECT", "RING", "NO CARRIER", "ERROR", 
    "_", "NO DIALTONE", "BUSY", "NO ANSWER" 
};

AT_Status AT_Tx(char c)
{
    if (putchar(c) == EOF)
        return AT_STATUS_FAIL;
    return AT_STATUS_OK;
}

AT_Status AT_Rx(char *c)
{
    *c = getchar();

    if (*c == EOF)
        return AT_STATUS_FAIL;
    return AT_STATUS_OK;
}

void AT_Send(const char *cmd, ...)
{
    char buf[256];

    va_list args;
    va_start(args, cmd);
    vsnprintf(buf, 256, cmd, args);
    va_end(args);

    size_t len = strlen(buf);

    for (size_t i = 0; i < len; ++i) {
        switch (AT_Tx(buf[i])) {
            case AT_STATUS_OK:
                break;
            case AT_STATUS_FAIL:
                break;
            case AT_STATUS_TIMEOUT:
                break;
        }
    }
    AT_Tx('\r');
    AT_Tx('\n');
}