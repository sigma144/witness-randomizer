#ifndef _WSWRAP_H
#define _WSWRAP_H

#include <string>
#include <functional>

#ifndef __EMSCRIPTEN__
#include <asio.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#else
#include "../subprojects/wsjs/wsjs.hpp"
#endif

namespace wswrap {

class WS final {
private:
#ifdef __EMSCRIPTEN__
    typedef WSJS IMPL;
#else
    typedef asio::io_service SERVICE;
    typedef websocketpp::client<websocketpp::config::asio_client> WSClient;
    typedef struct {
        WSClient first;
        WSClient::connection_ptr second;
    } IMPL;
#endif

public:
    typedef std::function<void(void)> onopen_handler;
    typedef std::function<void(void)> onclose_handler;
    typedef std::function<void(void)> onerror_handler;
    typedef std::function<void(const std::string&)> onmessage_handler;

    WS(const std::string& uri, onopen_handler hopen, onclose_handler hclose, onmessage_handler hmessage, onerror_handler herror=nullptr);
    virtual ~WS();

    unsigned long get_ok_connect_interval() const;

    void send(const std::string& data);
    void send_text(const std::string& data);
    void send_binary(const std::string& data);
    bool poll();
    size_t run();

private:
    IMPL *_impl;
#ifndef __EMSCRIPTEN__
    SERVICE *_service;
#endif
};

}; // namespace wsrap

#endif // _WSWRAP_H
