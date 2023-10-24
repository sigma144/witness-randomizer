// wsjs implementation of wswrap
#ifndef _WSWRAP_WSJS_HPP
#define _WSWRAP_WSJS_HPP

#ifndef _WSWRAP_HPP
#error "Don't  include wswrap_wsjs.hpp directly, include wsrap.hpp instead"
#endif


#include <string>
#include <functional>
#include <stdarg.h>
#include "../subprojects/wsjs/wsjs.hpp"


namespace wswrap {

    class WS final {
    private:
        typedef WSJS IMPL;
        typedef void SERVICE;

    public:
        typedef std::function<void(void)> onopen_handler;
        typedef std::function<void(void)> onclose_handler;
        typedef std::function<void(void)> onerror_handler;
        typedef std::function<void(const std::string&)> onerror_ex_handler;
        typedef std::function<void(const std::string&)> onmessage_handler;

        WS(const std::string& uri_string, onopen_handler hopen, onclose_handler hclose, onmessage_handler hmessage,
           onerror_ex_handler herror=nullptr, const std::string& = "")
                : WS(uri_string, hopen, hclose, hmessage, [herror](){herror("Unknown");})
        {
        }

        WS(const std::string& uri, onopen_handler hopen, onclose_handler hclose, onmessage_handler hmessage, onerror_handler herror=nullptr, const std::string& = "")
        {
            _impl = new IMPL(uri, hopen, hclose, hmessage, herror);
        }

        virtual ~WS()
        {
            delete _impl;
            _impl = nullptr;
        }

        unsigned long get_ok_connect_interval() const
        {
            return _impl->get_ok_connect_interval();
        }

#ifdef WSWRAP_SEND_EXCEPTIONS
        void send(const std::string& data)
        {
            _impl->send(data);
        }

        void send_text(const std::string& data)
        {
            _impl->send_text(data);
        }

        void send_binary(const std::string& data)
        {
            _impl->send_binary(data);
        }
#else
        bool send(const std::string& data)
        {
            try {
                _impl->send(data);
                return true;
            } catch (...) {
                return false;
            }
        }

        bool send_text(const std::string& data)
        {
            try {
                _impl->send_text(data);
                return true;
            } catch (...) {
                return false;
            }
        }

        bool send_binary(const std::string& data)
        {
            try {
                _impl->send_binary(data);
                return true;
            } catch (...) {
                return false;
            }
        }
#endif

        bool poll()
        {
            // poll is not required for wsjs
            return false;
        }

        size_t run()
        {
            // run does not work for wsjs
            return 0;
        }

        void stop()
        {
        }

    private:
        void warn(const char* fmt, ...)
        {
            va_list args;
            va_start (args, fmt);
            vfprintf (stderr, fmt, args);
            va_end (args);
        }

        IMPL *_impl;
        SERVICE *_service;
    };

}; // namespace wsrap

#endif //_WSWRAP_WSJS_HPP
