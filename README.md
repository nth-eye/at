# at

The library provides a minimalistic and efficient solution for parsing and handling AT commands, commonly used in communication with modems and other network interfaces. The core is a finite state machine (FSM) that interprets the responses from AT command interfaces.

To use the library, simply instantiate a parser with the desired buffer size, feed input characters into the FSM via the `process` method, and retrieve parsed results through the `get_result` method. The FSM handles the complexities of AT command syntax, providing a straightforward interface for command response handling.

## Example

```cpp
at::parser<64> parser;

parser.setup("+TEST");
parser.clear();

while (1) {

    parser.process(getc()); // from UART for example

    if (parser.full()       ||
        parser.acquired()   ||
        parser.response()   != at::invalid) 
    {
        puts("+-------RAW_-------+");
        log_hex(parser.get_raw());
        puts("+-------LINE-------+");
        log_hex(parser.get_line());
        puts("+-------ARGS-------+");
        log_hex(parser.get_args());
        puts("+------------------+");
        parser.clear();
    }
}
```

## TODO

- [x] source
- [ ] tests
- [x] readme
