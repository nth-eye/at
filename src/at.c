#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "at.h"

extern const char *RESULTS[AT_CODE_size];

static bool AT_SaveChar(AT *at)
{
    if (at->pos < at->buf_size) {
        at->buf[at->pos++] = at->c;
        return true;
    }
    return false;
}

static void AT_Clear(AT *at)
{
    at->parse_st    = AT_ST_IDLE;
    at->prev_ev     = AT_EV_OTHER;
    at->args        = NULL;
    at->pos         = 0;
}

static AT_State ON_Err(AT *at)
{
    at->cb_idx = at->n_cbks;
    AT_Clear(at);
    return AT_ST_IDLE;
}

static AT_State ON_Start(AT*);
static AT_State ON_Rsp(AT*);
static AT_State ON_Code(AT*);
static AT_State ON_NewRsp(AT*);
static AT_State ON_Semi(AT*);
static AT_State ON_Space(AT*);
static AT_State ON_Args(AT*);
static AT_State ON_NewText(AT*);
static AT_State ON_Text(AT*);
static AT_State ON_EndCR(AT*);
static AT_State ON_EndLF(AT*);

static AT_State HANDLE_Rsp(AT*);
static AT_State HANDLE_Code(AT*);
static AT_State HANDLE_Text(AT*);
static AT_State HANDLE_Result(AT *at, int res);

static const AT_Trans TABLE[AT_ST_size][AT_EV_size] = 
{   // AT_EV_CR,    AT_EV_LF,   AT_EV_MARK, AT_EV_SEMI, AT_EV_SPACE,    AT_EV_DIGIT,    AT_EV_LETTER,   AT_EV_OTHER
    { ON_Start,     NULL,       ON_NewRsp,  NULL,       NULL,           ON_Code,        NULL,           NULL        }, // AT_ST_IDLE
    { ON_Start,     ON_Start,   ON_NewRsp,  ON_NewText, ON_NewText,     ON_NewText,     ON_NewText,     ON_NewText  }, // AT_ST_START
    { ON_EndCR,     NULL,       ON_Rsp,     ON_Semi,    NULL,           ON_Rsp,         ON_Rsp,         NULL        }, // AT_ST_RSP
    { NULL,         NULL,       NULL,       NULL,       ON_Space,       NULL,           NULL,           NULL        }, // AT_ST_ARGS_START
    { ON_EndCR,     NULL,       ON_Args,    ON_Args,    ON_Args,        ON_Args,        ON_Args,        ON_Args     }, // AT_ST_ARGS
    { HANDLE_Code,  NULL,       NULL,       NULL,       NULL,           ON_Code,        NULL,           NULL        }, // AT_ST_CODE
    { ON_EndCR,     ON_Text,    ON_Text,    ON_Text,    ON_Text,        ON_Text,        ON_Text,        ON_Text     }, // AT_ST_TEXT
    { ON_EndCR,     ON_EndLF,   ON_Text,    ON_Text,    ON_Text,        ON_Text,        ON_Text,        ON_Text     }, // AT_ST_END
};

static AT_State ON_Start(AT *at)
{
    if (at->ev == AT_EV_LF && at->prev_ev != AT_EV_CR)
        return ON_Err(at);
    return AT_ST_START;
}

static AT_State ON_Rsp(AT *at)
{
    if (AT_SaveChar(at))
        return AT_ST_RSP;
    return ON_Err(at);
}

static AT_State ON_Code(AT *at)
{
    if (AT_SaveChar(at))
        return AT_ST_CODE;
    return ON_Err(at);
}

static AT_State ON_NewRsp(AT *at)
{
    if (at->prev_ev != AT_EV_CR && AT_SaveChar(at)) {
        at->parse_st = AT_ST_RSP;
        return AT_ST_RSP;
    }
    return ON_Err(at);
}

static AT_State ON_EndCR(AT *at)
{
    if (at->parse_st == AT_ST_RSP) {
        if (at->prev_ev == AT_EV_CR)
            return ON_Err(at);
        at->buf[at->pos++] = 0;
    } else {
        if (!AT_SaveChar(at))
            return ON_Err(at);
    }
    return AT_ST_END;
}

static AT_State ON_EndLF(AT *at)
{
    if (at->parse_st == AT_ST_RSP) {
        return HANDLE_Rsp(at);
    } else {
        return HANDLE_Text(at);
    }
}

static AT_State ON_Semi(AT *at)
{
    return AT_ST_ARGS_START;
}

static AT_State ON_Space(AT *at)
{
    at->buf[at->pos++] = 0;
    at->args = &at->buf[at->pos];
    return AT_ST_ARGS;
}

static AT_State ON_Args(AT *at)
{
    if (AT_SaveChar(at))
        return AT_ST_ARGS;
    return ON_Err(at);
}

static AT_State ON_NewText(AT *at)
{
    if (at->prev_ev == AT_EV_LF && AT_SaveChar(at)) {
        at->parse_st = AT_ST_TEXT;
        return AT_ST_TEXT;
    }
    return ON_Err(at);
}

static AT_State ON_Text(AT *at)
{
    if (at->parse_st == AT_ST_TEXT && AT_SaveChar(at))
        return AT_ST_TEXT;
    return ON_Err(at);
}


static AT_State HANDLE_Rsp(AT *at)
{
    for (size_t i = 0; i < at->n_cbks; ++i) {
        if (at->cbks[i].cb && !strcmp(at->cbks[i].cmd, at->buf)) {
            at->cbks[i].cb(at);
            at->cb_idx = i;
            break;
        }
    }
    return AT_ST_IDLE;
}

static AT_State HANDLE_Code(AT *at)
{
    at->buf[at->pos] = 0;

    long res = strtol(at->buf, NULL, 10);

    if (res == AT_CODE_RESERVED)
        return ON_Err(at);

    if (res >= AT_CODE_OK && 
        res <  AT_CODE_size)
    {
        return HANDLE_Result(at, res);
    }
    return ON_Err(at);
}

static AT_State HANDLE_Text(AT *at)
{
    at->buf[--(at->pos)] = 0;

    for (int i = 0; i < AT_CODE_size; ++i) {
        if (!strcmp(RESULTS[i], at->buf) && i != AT_CODE_RESERVED)
            return HANDLE_Result(at, i);
    }
    return ON_Err(at);
}

static AT_State HANDLE_Result(AT *at, int res)
{
    at->code = res;
    if (at->cb_idx < at->n_cbks) {
        if (at->cbks[at->cb_idx].res_cbks && 
            at->cbks[at->cb_idx].res_cbks[res])
        {
            at->cbks[at->cb_idx].res_cbks[res](at);
        }
        at->cb_idx = at->n_cbks;
    }
    return AT_ST_IDLE;
}


void AT_Init(AT *at, const AT_Cfg *cfg)
{
    at->status      = AT_STATUS_OK;
    at->st          = AT_ST_IDLE;
    at->ev          = AT_EV_OTHER;
    at->code        = AT_CODE_OK;
    at->cbks        = cfg->cbks;
    at->buf         = cfg->buf;
    at->n_cbks      = cfg->n_cbks;
    at->buf_size    = cfg->buf_size;
    AT_Clear(at);
}

void AT_Process(AT *at)
{    
    AT_NextEvent(at);

    if (at->st == AT_ST_IDLE)
        AT_Clear(at);

    AT_PRINT("\nAT_Process: [AT_State: 0x%02x] + ", at->st);
    if (at->status != AT_STATUS_OK) {
        AT_PRINT("[AT_Status: 0x%02x] -> failed\n", at->status);
        return;
    }
    AT_PRINT("[AT_Event: 0x%02x] -> ", at->ev);

    AT_Trans handle = TABLE[at->st][at->ev];

    if (handle) {
        at->st = handle(at);
    } else {
        at->st = ON_Err(at);
        AT_PRINT("[AT_Trans: NULL]\n");
    }
}

void AT_NextEvent(AT *at)
{
    at->prev_ev = at->ev;
    at->status = AT_Rx(&(at->c));

    if (at->status != AT_STATUS_OK)
        return;

    if (at->c >= '0' && at->c <= '9') {
        at->ev = AT_EV_DIGIT;
        return;
    }
    switch (at->c) {
        case '\v':  at->ev = AT_EV_CR; return;
        case '\n':  at->ev = AT_EV_LF; return;
        case '+':  
        case '#': 
        case '&': 
        case '$':
        case '%':   at->ev = AT_EV_MARK; return;
        case ':':   at->ev = AT_EV_SEMI; return;
        case ' ':   at->ev = AT_EV_SPACE; return;
    }
    if ((at->c >= 'A' && at->c <= 'Z') ||
        (at->c >= 'a' && at->c <= 'z')) 
    {
        at->ev = AT_EV_LETTER;
        return;
    }
    at->ev = AT_EV_OTHER;
}
