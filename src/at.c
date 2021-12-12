#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "at.h"

#define AT_LOG    printf
#ifndef AT_LOG
#define AT_LOG(...)
#endif

static const char *AT_RESPONSES[AT_RSP_num] = { 
    [AT_RSP_OK]             = "OK", 
    [AT_RSP_CONNECT]        = "CONNECT", 
    [AT_RSP_RING]           = "RING", 
    [AT_RSP_NO_CARRIER]     = "NO CARRIER", 
    [AT_RSP_ERROR]          = "ERROR",
    [AT_RSP_RESERVED]       = "RESERVED",
    [AT_RSP_NO_DIALTONE]    = "NO DIALTONE", 
    [AT_RSP_BUSY]           = "BUSY", 
    [AT_RSP_NO_ANSWER]      = "NO ANSWER",
    [AT_RSP_RDY]            = "RDY",
};

static bool at_save_c(AT *at)
{
    if (at->pos < at->cfg.buf_size) {
        at->cfg.buf[at->pos++] = at->c;
        return true;
    }
    return false;
}

static AT_State at_err(AT *at)
{
    at_clear(at);
    return AT_ST_IDLE;
}

static AT_Event at_next_evt(AT *at, char c)
{
    at->prev_ev = at->ev;
    at->c       = c;

    switch (at->c) {
        case '\r':  return AT_EV_CR;
        case '\n':  return AT_EV_LF;
        case '+':  
        case '#': 
        case '&': 
        case '$':
        case '%':   return AT_EV_MARK;
        case ':':   return AT_EV_SEMI;
        case ' ':   return AT_EV_SPACE;
    }
    return AT_EV_OTHER;
}


static AT_State handle_urc(AT *at)
{
    AT_LOG("[%s: %s]\n", at->cfg.buf, at->param ? at->param : "<NULL>");

    for (size_t i = 0; i < at->cfg.cbks_size; ++i) {
        if (at->cfg.cbks[i].cb && !strcmp(at->cfg.cbks[i].cmd, at->cfg.buf)) {
            at->cfg.cbks[i].cb(at);
            break;
        }
    }
    return AT_ST_IDLE;
}

static AT_State handle_txt(AT *at)
{
    at->cfg.buf[--(at->pos)] = 0;

    AT_LOG("[%s]\n", at->cfg.buf);

    for (int i = 0; i < AT_RSP_num; ++i) {
        if (!strcmp(AT_RESPONSES[i], at->cfg.buf)) {
            at->rsp = i;
            return AT_ST_IDLE;
        }
    }
    return at_err(at);
}


static AT_State on_start(AT *at)
{
    if (at->ev      == AT_EV_LF && 
        at->prev_ev != AT_EV_CR)
        return at_err(at);
    return AT_ST_START;
}

static AT_State on_urc(AT *at)
{
    return at_save_c(at) ? AT_ST_URC : at_err(at);
}

static AT_State on_new_urc(AT *at)
{
    if (at->prev_ev == AT_EV_LF && at_save_c(at))
        return AT_ST_URC;
    return at_err(at);
}

static AT_State on_txt(AT *at)
{
    return at_save_c(at) ? AT_ST_TXT : at_err(at);
}

static AT_State on_new_txt(AT *at)
{
    if (at->prev_ev == AT_EV_LF && at_save_c(at))
        return AT_ST_TXT;
    return at_err(at);
}

static AT_State on_param(AT *at)
{
    return at_save_c(at) ? AT_ST_PARAM : at_err(at);
}

static AT_State on_semi(AT *at)
{
    at->cfg.buf[at->pos++]  = 0;
    at->param               = &at->cfg.buf[at->pos];
    return AT_ST_PARAM_START;
}

static AT_State on_space(AT *at)
{
    return AT_ST_PARAM;
}

static AT_State on_end_cr(AT *at)
{
    if (at->param) {
        if (at->prev_ev == AT_EV_CR)
            return at_err(at);
        at->cfg.buf[at->pos++] = 0;
    } else {
        if (!at_save_c(at))
            return at_err(at);
    }
    return AT_ST_END;
}

static AT_State on_end_lf(AT *at)
{
    return at->param ? handle_urc(at) : handle_txt(at);
}

static const AT_Trans TABLE[AT_ST_num][AT_EV_num] = 
{   // AT_EV_CR,    AT_EV_LF,   AT_EV_MARK, AT_EV_SEMI,     AT_EV_SPACE,    AT_EV_OTHER
    { on_start,     NULL,       NULL,       NULL,           NULL,           NULL        }, // AT_ST_IDLE
    { on_start,     on_start,   on_new_urc, on_new_txt,     on_new_txt,     on_new_txt  }, // AT_ST_START
    { on_end_cr,    NULL,       on_urc,     on_semi,        on_urc,         on_urc      }, // AT_ST_URC
    { NULL,         NULL,       on_param,   on_param,       on_space,       on_param    }, // AT_ST_PARAM_START
    { on_end_cr,    NULL,       on_param,   on_param,       on_param,       on_param    }, // AT_ST_PARAM
    { on_end_cr,    on_txt,     on_txt,     on_txt,         on_txt,         on_txt      }, // AT_ST_TXT
    { on_end_cr,    on_end_lf,  on_txt,     on_txt,         on_txt,         on_txt      }, // AT_ST_END
};

bool at_send(const char *cmd, ...)
{
    char buf[256];

    va_list args;
    va_start(args, cmd);

    int len = vsnprintf(buf, sizeof(buf) - 2, cmd, args);
    if (len < 0 || len > (int) sizeof(buf) - 3)
        return false;

    va_end(args);

    buf[len++] = '\r';
    buf[len++] = '\n';
    buf[len]   = 0;

    for (int i = 0; i < len; ++i) {
        if (putc(buf[i], stdout) == EOF)
            return false;
    }
    return true;
}

void at_init(AT *at, const AT_Cfg *cfg)
{
    at->cfg         = *cfg;
    at->st          = AT_ST_IDLE;
    at->ev          = AT_EV_OTHER;
    at->c           = 0;
    at_clear(at);
}

void at_clear(AT *at)
{
    at->prev_ev     = AT_EV_OTHER;
    at->rsp         = AT_RSP_num;
    at->param       = NULL;
    at->pos         = 0;
}

void at_process(AT *at, char c)
{    
    at->ev = at_next_evt(at, c);

    if (at->st == AT_ST_IDLE)
        at_clear(at);

    AT_Trans handle = TABLE[at->st][at->ev];

    at->st = handle ? handle(at) : at_err(at);
}
