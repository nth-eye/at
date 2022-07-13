#ifndef AT_H
#define AT_H

#include <string_view>

namespace at {

using String = std::string_view;

enum Rsp {
    RSP_OK,
    RSP_CONNECT,
    RSP_RING,
    RSP_NO_CARRIER,
    RSP_ERROR,
    RSP_NO_DIALTONE,
    RSP_BUSY,
    RSP_NO_ANSWER,
    RSP_RDY,
    RSP_num,
};

enum State {
    ST_IDLE,
    ST_NEW_CR,
    ST_NEW_LF,
    ST_ARG_START,
    ST_ARG,
    ST_URC,
    ST_TXT,
    ST_END,
    ST_num,
};

enum Event {
    EV_CR,
    EV_LF,
    EV_MARK,
    EV_SEMI,
    EV_SPACE,
    EV_OTHER,
    EV_num,
};

struct Data {
    String raw;
    String txt;
    String arg;
    Rsp rsp;
    bool found;
};

struct At {

    At() = delete;
    At(char *buf, size_t len) : buf{buf}, max{len} {}

    auto data() const
    {
        return Data{String{buf, pos}, txt, arg, rsp, found};
    }

    auto raw() const
    {
        return String{buf, pos};
    }

    auto line() const
    {
        return txt;
    }

    auto args() const
    {
        return arg;
    }

    auto response() const
    {
        return rsp;
    }

    bool acquired() const
    {
        return found;
    }

    bool full() const
    {
        return pos >= max;
    }

    void setup(String target)
    {
        trg = target;
    }

    void clear()
    {
        pos = 0;
        reset();
    }

    void reset()
    {
        st      = ST_IDLE;
        rsp     = RSP_num;
        arg     = {};
        txt     = {};
        found   = false;
    }

    void process(char c)
    {
        if (full())
            return;
        constexpr State (At::*table[ST_num][EV_num])() = 
        {   // EV_CR,       EV_LF,      EV_MARK,    EV_SEMI,    EV_SPACE,   EV_OTHER
            { on_new_cr,    nullptr,    nullptr,    nullptr,    nullptr,    nullptr     }, // ST_IDLE
            { nullptr,      on_new_lf,  nullptr,    nullptr,    nullptr,    nullptr     }, // ST_NEW_CR
            { nullptr,      nullptr,    on_new_urc, on_new_txt, on_new_txt, on_new_txt  }, // ST_NEW_LF
            { nullptr,      nullptr,    on_arg,     on_arg,     on_space,   on_arg      }, // ST_ARG_START
            { on_end_cr,    nullptr,    on_arg,     on_arg,     on_arg,     on_arg      }, // ST_ARG
            { on_end_cr,    nullptr,    on_urc,     on_new_arg, on_urc,     on_urc      }, // ST_URC
            { on_end_cr,    on_txt,     on_txt,     on_txt,     on_txt,     on_txt      }, // ST_TXT
            { on_end_cr,    on_end_lf,  on_txt,     on_txt,     on_txt,     on_txt      }, // ST_END
        };
        auto cb = table[st][next_ev(c)];
        if (!cb)
            reset();
        else
            st = (this->*cb)();
        buf[pos++] = c;
    }
private:
    static constexpr Event next_ev(char c)
    {
        switch (c) {
            case '\r':  return EV_CR;
            case '\n':  return EV_LF;
            case '+':  
            case '#': 
            case '&': 
            case '$':
            case '%':   return EV_MARK;
            case ':':   return EV_SEMI;
            case ' ':   return EV_SPACE;
        }
        return EV_OTHER;
    }

    auto ptr() const 
    { 
        return buf + pos; 
    }

    size_t diff(const char *p) const
    {
        return ptr() - p;
    }

    State handle_arg()
    {
        found = trg == txt;
        return ST_IDLE;
    }

    State handle_txt()
    {
        constexpr String responses[RSP_num] = { 
            "OK", "CONNECT", "RING", "NO CARRIER", "ERROR", 
            "NO DIALTONE", "BUSY", "NO ANSWER", "RDY",
        };
        for (int i = 0; i < RSP_num; ++i) {
            if (txt == responses[i]) {
                rsp = Rsp(i);
                break;
            }
        }
        return handle_arg();
    }

    State on_arg()
    {
        return ST_ARG;
    }

    State on_urc()
    {
        return ST_URC;
    }

    State on_txt()
    {
        return ST_TXT;
    }

    State on_space()
    {
        arg = {ptr() + 1, 0};
        return ST_ARG;
    }

    State on_new_arg()
    {
        txt = {txt.begin(), diff(txt.begin())};
        return ST_ARG_START;
    }

    State on_new_urc()
    {
        txt = {ptr(), 0};
        return ST_URC;
    }

    State on_new_txt()
    {
        txt = {ptr(), 0};
        return ST_TXT;
    }

    State on_new_cr()
    {
        reset();
        return ST_NEW_CR;
    }

    State on_new_lf()
    {
        return ST_NEW_LF;
    }

    State on_end_cr()
    {
        if (arg.begin())
            arg = {arg.begin(), diff(arg.begin())};
        else
            txt = {txt.begin(), diff(txt.begin())};
        return ST_END;
    }

    State on_end_lf()
    {
        return arg.empty() ? handle_txt() : handle_arg();
    }
private:
    String  trg;    // Target string to match
    String  arg;    // Parsed string with arguments (parameters)
    String  txt;    // Parsed string with last text line
    char    *buf;
    size_t  max;
    size_t  pos     = 0;
    State   st      = ST_IDLE;
    Rsp     rsp     = RSP_num;
    bool    found   = false;
};

template<size_t N>
struct Parser : At {
    Parser() : At{buf, N} {}
private:
    char buf[N];
};

}

#endif