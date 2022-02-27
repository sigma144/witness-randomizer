# wswrap

Wrapper around [https://github.com/zaphoyd/websocketpp](zaphoyd/websocketpp)
and [https://github.com/black-sliver/wsjs](black-sliver/wsjs) that allows easy
websocket client use for c++ in desktop and browser (emscripten) context.

## How to use

* Recusive add submodule to your project or dump contents of the "full source"
    * get the source with `git clone --recurse-submodules https://github.com/black-sliver/wswrap.git`
    * or add it as submodule with `git submodule add https://github.com/black-sliver/wswrap.git; git submodule update --init --recursive`
* For desktop add [https://github.com/chriskohlhoff/asio](asio) or boost::asio to include paths
* For desktop define ASIO_STANDALONE when not using boost
* For desktop add [https://github.com/zaphoyd/websocketpp](websocketpp) to include paths
* Add wswrap.cpp to source files, include wswrap.hpp
* For webbrowser
    * events will fire from js event loop
* For desktop
    * use `::run` or `::poll` on the socket for communication to occur/events to fire

## API

`WS::WS(const std::string& uri, onopen_handler hopen, onclose_handler hclose, onmessage_handler hmessage, onerror_handler herror=nullptr);`

Constructor will start connecting and at some point the object fires
* `hopen()` once the connection is established
* `hclose()` once the connection is closed or failed
* `hmessage(const std::string& msg)` when a message was received
* `herror()` when an error happened

`~WS()`

Destructor will close the socket.

`void send(const std::string& data);`

Send data on the websocket. Defaults to send_text.

`void send_text(const std::string& data);`

Send data as text frame on the websocket.

`void send_binary(const std::string& data);`

Send data as binary frame on the websocket.

`bool poll();`

* Desktop: handle queued incoming and outgoing data, callbacks are fired from here
* Webbrowser: no-op

`size_t run();`

* Desktop: poll() until the socket is closed
* Webbrowser: no-op; use `#ifdef __EMSCRIPTEN__` to decide if ::run() is usable.
