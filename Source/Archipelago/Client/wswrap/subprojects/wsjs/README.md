# WSJS

C++ websocket client library for emscripten webbrowser contexts.

## How to use

* Submodule or copy into project
* `#include <wsjs.hpp>` - it's header-only
* Constructor and destructor (de)registers WS with JS
* Callbacks will fire from JS event loop
* Use `::send` (defaults to text), `::send_text` or `::send_binary` to write to the socket

## Example

```c++
#include <iostream>
#include <wsjs.hpp>

using namespace std;

WSJS *ws = nullptr;

void onopen()
{
    ws->send_text("hello world!\n");
}

void onclose()
{
    delete ws;
    ws = nullptr;
}

void onmessage(const string& msg)
{
    cout << msg << endl;
}

void onerror()
{
    cerr << "Error received!" << endl;
}

int main(int argc, char **argv)
{
    ws = new WSJS(onopen, onclose, onmessage, onerror);
    // emscripten animation frame loop goes here
    return 0;
}
```
