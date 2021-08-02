#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "at.h"

#include <stdarg.h>
#include <stdio.h>

#define AT_PRINT    printf

#ifndef AT_PRINT
#define AT_PRINT(...)
#warning "To enable debug-print please define AT_PRINT as output mechanism";
#endif

// NOTE: Add manufacturer-specific response codes at the end.
const char *AT_RESULTS[AT_CODE_num] = { 
    "OK", "CONNECT", "RING", "NO CARRIER", "ERROR", 
    "_", "NO DIALTONE", "BUSY", "NO ANSWER" 
};

AT_Status at_tx(char c)
{
    return putc(c, stdout) == EOF ? AT_STATUS_FAIL : AT_STATUS_OK;
}

AT_Status at_rx(char *c)
{
    *c = getc(stdin);

    return *c == EOF ? AT_STATUS_FAIL : AT_STATUS_OK;
}

AT_Status at_send(const char *cmd, ...)
{
    char buf[256];

    va_list args;
    va_start(args, cmd);

    int len = vsnprintf(buf, sizeof(buf) - 2, cmd, args);
    if (len < 0 || len > (int) sizeof(buf) - 3)
        return AT_STATUS_FAIL;

    va_end(args);

    buf[len++] = '\r';
    buf[len++] = '\n';
    buf[len]   = 0;

    for (int i = 0; i < len; ++i) {
        switch (at_tx(buf[i])) {
            case AT_STATUS_OK:      break;
            case AT_STATUS_FAIL:    return AT_STATUS_FAIL;
            case AT_STATUS_TIMEOUT: return AT_STATUS_TIMEOUT;
        }
    }
    return AT_STATUS_OK;
}

static bool at_save_c(AT *at)
{
    if (at->pos < at->buf_size) {
        at->buf[at->pos++] = at->c;
        return true;
    }
    return false;
}

static void at_next_evt(AT *at)
{
    at->prev_ev = at->ev;
    at->status = at_rx(&(at->c));

    if (at->status != AT_STATUS_OK)
        return;

    if (at->c >= '0' && at->c <= '9') {
        at->ev = AT_EV_DIGIT;
        return;
    }
    switch (at->c) {
        case '\r':  at->ev = AT_EV_CR; return;
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

static AT_State at_err(AT *at)
{
    at->cb_idx = at->cbks_size;
    at_clear(at);
    return AT_ST_IDLE;
}


static AT_State handle_res(AT *at, int res)
{
    at->code = res;
    if (at->cb_idx < at->cbks_size) {
        if (at->cbks[at->cb_idx].res_cbks && 
            at->cbks[at->cb_idx].res_cbks[res])
        {
            at->cbks[at->cb_idx].res_cbks[res](at);
        }
        at->cb_idx = at->cbks_size;
    }
    return AT_ST_IDLE;
}

static AT_State handle_rsp(AT *at)
{
    AT_PRINT("[%s]: %s\n", at->buf, at->args ? at->args : "NO ARGS");

    for (size_t i = 0; i < at->cbks_size; ++i) {
        if (at->cbks[i].cb && !strcmp(at->cbks[i].cmd, at->buf)) {
            at->cbks[i].cb(at);
            at->cb_idx = i;
            break;
        }
    }
    return AT_ST_IDLE;
}

static AT_State handle_code(AT *at)
{
    at->buf[at->pos] = 0;

    long res = strtol(at->buf, NULL, 10);

    if (res == AT_CODE_RESERVED)
        return at_err(at);

    if (res >= AT_CODE_OK && 
        res <  AT_CODE_num)
    {
        return handle_res(at, res);
    }
    return at_err(at);
}

static AT_State handle_text(AT *at)
{
    at->buf[--(at->pos)] = 0;

    AT_PRINT("[%s]\n", at->buf);

    for (int i = 0; i < AT_CODE_num; ++i) {
        if (!strcmp(AT_RESULTS[i], at->buf) && i != AT_CODE_RESERVED)
            return handle_res(at, i);
    }
    return at_err(at);
}


static AT_State on_start(AT *at)
{
    if (at->ev == AT_EV_LF && at->prev_ev != AT_EV_CR)
        return at_err(at);
    return AT_ST_START;
}

static AT_State on_rsp(AT *at)
{
    return at_save_c(at) ? AT_ST_RSP : at_err(at);
}

static AT_State on_code(AT *at)
{
    return at_save_c(at) ? AT_ST_CODE : at_err(at);
}

static AT_State on_new_rsp(AT *at)
{
    if (at->prev_ev != AT_EV_CR && at_save_c(at)) {
        at->parse_st = AT_ST_RSP;
        return AT_ST_RSP;
    }
    return at_err(at);
}

static AT_State on_end_cr(AT *at)
{
    if (at->parse_st == AT_ST_RSP) {
        if (at->prev_ev == AT_EV_CR)
            return at_err(at);
        at->buf[at->pos++] = 0;
    } else {
        if (!at_save_c(at))
            return at_err(at);
    }
    return AT_ST_END;
}

static AT_State on_end_lf(AT *at)
{
    if (at->parse_st == AT_ST_RSP)
        return handle_rsp(at);
    else
        return handle_text(at);
}

static AT_State on_semi(AT *at)
{
    return AT_ST_ARGS_START;
}

static AT_State on_space(AT *at)
{
    at->buf[at->pos++] = 0;
    at->args = &at->buf[at->pos];
    return AT_ST_ARGS;
}

static AT_State on_args(AT *at)
{
    return at_save_c(at) ? AT_ST_ARGS : at_err(at);
}

static AT_State on_new_text(AT *at)
{
    if (at->prev_ev == AT_EV_LF && at_save_c(at)) {
        at->parse_st = AT_ST_TEXT;
        return AT_ST_TEXT;
    }
    return at_err(at);
}

static AT_State on_text(AT *at)
{
    if (at->parse_st == AT_ST_TEXT && at_save_c(at))
        return AT_ST_TEXT;
    return at_err(at);
}


static const AT_Trans TABLE[AT_ST_num][AT_EV_num] = 
{   // AT_EV_CR,    AT_EV_LF,   AT_EV_MARK, AT_EV_SEMI,     AT_EV_SPACE,    AT_EV_DIGIT,    AT_EV_LETTER,   AT_EV_OTHER
    { on_start,     NULL,       on_new_rsp, NULL,           NULL,           on_code,        NULL,           NULL        }, // AT_ST_IDLE
    { on_start,     on_start,   on_new_rsp, on_new_text,    on_new_text,    on_new_text,    on_new_text,    on_new_text }, // AT_ST_START
    { on_end_cr,    NULL,       on_rsp,     on_semi,        on_rsp,         on_rsp,         on_rsp,         NULL        }, // AT_ST_RSP
    { NULL,         NULL,       NULL,       NULL,           on_space,       NULL,           NULL,           NULL        }, // AT_ST_ARGS_START
    { on_end_cr,    NULL,       on_args,    on_args,        on_args,        on_args,        on_args,        on_args     }, // AT_ST_ARGS
    { handle_code,  NULL,       NULL,       NULL,           NULL,           on_code,        NULL,           NULL        }, // AT_ST_CODE
    { on_end_cr,    on_text,    on_text,    on_text,        on_text,        on_text,        on_text,        on_text     }, // AT_ST_TEXT
    { on_end_cr,    on_end_lf,  on_text,    on_text,        on_text,        on_text,        on_text,        on_text     }, // AT_ST_END
};


void at_init(AT *at, const AT_Cfg *cfg)
{
    at->status      = AT_STATUS_OK;
    at->st          = AT_ST_IDLE;
    at->ev          = AT_EV_OTHER;
    at->code        = AT_CODE_OK;
    at->cbks        = cfg->cbks;
    at->buf         = cfg->buf;
    at->cbks_size   = cfg->cbks_size;
    at->buf_size    = cfg->buf_size;
    at->cb_idx      = cfg->cbks_size;
    at->c           = 0;
    at_clear(at);
}

void at_process(AT *at)
{    
    at_next_evt(at);

    if (at->st == AT_ST_IDLE)
        at_clear(at);

    if (at->status != AT_STATUS_OK)
        return;

    AT_Trans handle = TABLE[at->st][at->ev];

    at->st = handle ? handle(at) : at_err(at);
}

void at_clear(AT *at)
{
    at->parse_st    = AT_ST_IDLE;
    at->prev_ev     = AT_EV_OTHER;
    at->args        = NULL;
    at->pos         = 0;
}