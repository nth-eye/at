#include <stdio.h>
#include "at_platform.h"

// NOTE: Add manufacturer-specific response codes at the end.
const char *RESULTS[AT_CODE_size] = { 
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

void AT_SendCmd(const char *cmd, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        switch (AT_Tx(cmd[i])) {
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