#include "../include/wswrap.hpp"
#include <chrono>

#ifdef __EMSCRIPTEN__
#include "../subprojects/wsjs/wsjs.hpp"
#include "../subprojects/wsjs/wsjs.cpp"
#else
// websocketpp is header-only
#endif

namespace wswrap {
WS::WS(const std::string& uri, onopen_handler hopen, onclose_handler hclose, onmessage_handler hmessage, onerror_handler herror)
{
#ifdef __EMSCRIPTEN__
    _impl = new IMPL(uri, hopen, hclose, hmessage, herror);
#else
    _service = new SERVICE();
    _impl = new IMPL();
    auto& client = _impl->first;
    auto& conn = _impl->second;
    client.clear_access_channels(websocketpp::log::alevel::all);
    client.set_access_channels(websocketpp::log::alevel::none | websocketpp::log::alevel::app);
    client.clear_error_channels(websocketpp::log::elevel::all);
    client.set_error_channels(websocketpp::log::elevel::warn|websocketpp::log::elevel::rerror|websocketpp::log::elevel::fatal);
    client.init_asio(_service);

    client.set_message_handler([this,hmessage] (websocketpp::connection_hdl hdl, WSClient::message_ptr msg) {
        if (_impl->second && hmessage) hmessage(msg->get_payload());
    });
    client.set_open_handler([this,hopen] (websocketpp::connection_hdl hdl) {
        if (_impl->second && hopen) hopen();
    });
    client.set_close_handler([this,hclose] (websocketpp::connection_hdl hdl) {
        if (_impl->second) {
            _impl->second = nullptr;
            if (hclose) hclose();
        }
    });
    client.set_fail_handler([this,herror,hclose] (websocketpp::connection_hdl hdl) {
        if (_impl->second) {
            _impl->second = nullptr;
            if (herror) herror();
            if (hclose) hclose();
        }
    });

    websocketpp::lib::error_code ec;
    conn = client.get_connection(uri, ec);
    if (ec) {
        // TODO: run close and error handler? or throw exception?
    }
    if (!client.connect(conn)) {
        // TODO: run close and error handler? or throw exception?
    }
#endif
}

WS::~WS()
{
#ifdef __EMSCRIPTEN__
    delete _impl;
    _impl = nullptr;
#else
    auto& client = _impl->first;
    auto& conn = _impl->second;
    client.set_message_handler(nullptr);
    client.set_open_handler(nullptr);
    client.set_close_handler(nullptr);
    client.set_fail_handler([this](...){ _impl->second = nullptr; });
    try {
        if (conn) {
            conn->close(websocketpp::close::status::normal, "");
            conn = nullptr;
            // wait for connection to close -- client.run() will hang if
            // if the destructor is called from a message callback, so we poll
            // with timeout instead, possibly leaking the underlying socket
            auto t = std::chrono::steady_clock::now();
            while (!client.stopped()) {
                client.poll();
                auto td = (std::chrono::steady_clock::now()-t);
                if (td > std::chrono::milliseconds(500)) break; // timeout
            }
            if (!client.stopped()) {
                printf("wswrap: disconnect timed out. "
                        "Possibly disconnecting while handling an event.\n");
                client.stop();
            }
        }
    } catch (const std::exception& ex) {
        printf("wswrap: exception during close: %s\n", ex.what());
        conn = nullptr;
        client.stop();
    }
    // NOTE: the destructor can not be called from a ws callback in some
    //       circumstances, otherwise it will hang here. TODO: Document this.
    delete _impl;
    _impl = nullptr;
    delete _service;
    _service = nullptr;
#endif
}

unsigned long WS::get_ok_connect_interval() const
{
#ifdef __EMSCRIPTEN__
    return _impl->get_ok_connect_interval();
#else
    return 1000;
#endif
}

void WS::send(const std::string& data)
{
#ifdef __EMSCRIPTEN__
    _impl->send(data);
#else
    // TODO: auto-detect
    _impl->first.send(_impl->second,data,websocketpp::frame::opcode::text);
#endif
}

void WS::send_text(const std::string& data)
{
#ifdef __EMSCRIPTEN__
    _impl->send_text(data);
#else
    _impl->first.send(_impl->second,data,websocketpp::frame::opcode::text);
#endif
}

void WS::send_binary(const std::string& data)
{
#ifdef __EMSCRIPTEN__
    _impl->send_binary(data);
#else
    _impl->first.send(_impl->second,data,websocketpp::frame::opcode::binary);
#endif
}

bool WS::poll()
{
#ifdef __EMSCRIPTEN__
    return false;
#else
    return _service->poll();
#endif
}

size_t WS::run()
{
#ifdef __EMSCRIPTEN__
    return 0;
#else
    return _service->run();
#endif
}

} // namespace wsrap
