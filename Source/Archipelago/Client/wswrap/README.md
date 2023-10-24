# wswrap

Wrapper around [zaphoyd/websocketpp](https://github.com/zaphoyd/websocketpp)
and [black-sliver/wsjs](https://github.com/black-sliver/wsjs) that allows easy
websocket client use for c++ in desktop and browser (emscripten) context.

## How to use

* Recursive add submodule to your project or dump contents of the "full source"
    * get the source with `git clone --recurse-submodules https://github.com/black-sliver/wswrap.git`
    * or add it as submodule with `git submodule add https://github.com/black-sliver/wswrap.git; git submodule update --init --recursive`
* For desktop add [asio](https://github.com/chriskohlhoff/asio) or boost::asio to include paths
* For desktop define ASIO_STANDALONE when not using boost
* For desktop add [websocketpp](https://github.com/zaphoyd/websocketpp) to include paths
* Not all websocketpp versions are compatible to all asio versions.
    * [Try those](https://github.com/black-sliver/ap-soeclient/tree/master/subprojects) (download repo as zip and extract)
* For desktop with SSL support, link against OpenSSL (libssl and libcrypto), or `#define WSWRAP_NO_SSL` to disable SSL support
* Include wswrap.hpp - it's header-only
* For webbrowser
    * events will fire from js event loop
* For desktop
    * use `::run` or `::poll` on the socket for communication to occur/events to fire
* On windows
    * you may have to `#define ASIO_NO_WIN32_LEAN_AND_MEAN` (standalone) or `BOOST_ASIO_NO_WIN32_LEAN_AND_MEAN` (boost)
      before including wswrap.hpp, move wswrap include before windows.h and define `_WIN32_WINNT` globally.
    * link against ws2_32
    * when using SSL, also link against crypt32

## API

`WS::WS(const std::string& uri, onopen_handler hopen, onclose_handler hclose, onmessage_handler hmessage, onerror_handler herror=nullptr);`

Constructor will start connecting and at some point the object fires
* `hopen()` once the connection is established
* `hclose()` once the connection is closed or failed
* `hmessage(const std::string& msg)` when a message was received
* `herror()` when an error happened

`~WS()`

Destructor will close the socket.

By default, this can't be run from a callback (onerror, onclose, onmessage), but you can `#define WSWRAP_ASYNC_CLEANUP`
to have it create a detached thread for the cleanup.

`bool/void send(const std::string& data);`

Send data on the websocket. Defaults to send_text.

`bool/void send_text(const std::string& data);`

Send data as text frame on the websocket.

`bool/void send_binary(const std::string& data);`

Send data as binary frame on the websocket.

`bool poll();`

* Desktop: handle queued incoming and outgoing data, callbacks are fired from here.
* Webbrowser: no-op

`size_t run();`

* Desktop: poll() until the socket is closed, you can call stop() to close it.
* Webbrowser: no-op; use `#ifdef __EMSCRIPTEN__` to decide if ::run() is usable.

### Changes

#### v1.01

* `send`/`send_*` will now return bool. If `WSWRAP_SEND_EXCEPTIONS` is defined, they will throw exceptions as before.

#### v1.02

* Automatically send pings on desktop (not JS) if the server doesn't.
* error handler callback now optionally has a `const std::string&` argument for a readable error message
