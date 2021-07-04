## Usage
--
```shell
$ mg_jsonrpc ws://localhost:8000
```
```shell
$ websocat --text jsonrpc:- ws://localhost:8000/ws/jsonrpc
```
use websocat to debug, use input method and param, websocat
is not converient to edit, improve it

## mjson
mjsonrpc didn't include jsonrpc 2.0 version member, maybe use
mjson_merge to fixup

## Reference
1. https://www.jsonrpc.org/specification
2. https://github.com/cesanta/mjson
3. https://github.com/cesanta/mongoose
4. https://github.com/vi/websocat
5. http://catb.org/~esr/microjson/microjson.html
