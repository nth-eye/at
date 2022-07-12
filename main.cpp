#include <cstdio>
#include <cctype>
#include "at.h"

constexpr char bin_to_char(uint8_t bin)
{
    return "0123456789abcdef"[bin & 0xf];
}

inline void log_hex(const void *dat, size_t len)
{
    if (!dat || !len)
        return;

    auto p = static_cast<const uint8_t*>(dat);

    for (size_t i = 0; i < len; ++i) {

        if (!(i & 15)) {
            putchar('|');
            putchar(' ');
        }
        putchar(bin_to_char(p[i] >> 4));
        putchar(bin_to_char(p[i] & 0xF));
        putchar(' ');
        
        if ((i & 7) == 7)
            putchar(' ');

        if ((i & 15) == 15) {
            putchar('|');
            for (int j = 15; j >= 0; --j) {
                char c = p[i - j];
                putchar(isprint(c) ? c : '.');
            }
            putchar('|');
            putchar('\n');
        }
    }
    int rem = len - ((len >> 4) << 4);
    if (rem) {
        for (int j = (16 - rem) * 3 + ((~rem & 8) >> 3); j >= 0; --j)
            putchar(' ');
        putchar('|');
        for (int j = rem; j; --j) {
            char c = p[len - j];
            putchar(isprint(c) ? c : '.');
        }
        for (int j = 0; j < 16 - rem; ++j)
            putchar('.');
        putchar('|');
        putchar('\n');
    }
}

int main(void) 
{
    at::Parser<128> parser;
    
    parser.setup("+TEST");
    parser.clear();

    while (1) {

        char c = getc(stdin);

        parser.process(c);

        if (parser.full()) {
            printf("+-------FULL-------+\n");
            log_hex(parser.raw().data(), parser.raw().size());
            printf("+------------------+\n");
            parser.clear();
        }

        if (parser.response() != at::RSP_num) {
            printf("+--------RAW-------+\n");
            log_hex(parser.raw().data(), parser.raw().size());
            printf("+--------RSP-------+\n");
            log_hex(parser.line().data(), parser.line().size());
            printf("+------------------+\n");
            parser.clear();
        }

        if (parser.acquired()) {
            printf("+--------RAW-------+\n");
            log_hex(parser.raw().data(), parser.raw().size());
            printf("+--------TXT-------+\n");
            log_hex(parser.line().data(), parser.line().size());
            printf("+--------ARG-------+\n");
            log_hex(parser.args().data(), parser.args().size());
            printf("+------------------+\n");
            parser.clear();
        }
    }
}
