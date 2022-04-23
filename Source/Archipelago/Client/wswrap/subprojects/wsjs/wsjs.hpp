#ifndef _WSJS_H
#define _WSJS_H

#include <string>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <memory>
#include <map>
#include <functional>
#include <stdint.h>

class WSJS;

class WSJS {
public:
    typedef std::function<void(void)> onopen_handler;
    typedef std::function<void(void)> onclose_handler;
    typedef std::function<void(void)> onerror_handler;
    typedef std::function<void(const std::string&)> onmessage_handler;

    WSJS(const std::string& uri, onopen_handler hopen, onclose_handler hclose, onmessage_handler hmessage, onerror_handler herror=nullptr)
        : _id(-1), _hopen(hopen), _hclose(hclose), _hmessage(hmessage), _herror(herror)
    {
        create(this, uri);
    }

    virtual ~WSJS()
    {
        destroy(this);
    }
    
    unsigned long get_ok_connect_interval() const;

    void send(const std::string& data);
    void send_text(const std::string& data);
    void send_binary(const std::string& data);

protected:
    uint32_t _id;
    onopen_handler _hopen;
    onclose_handler _hclose;
    onmessage_handler _hmessage;
    onerror_handler _herror;

    static uint32_t nextid(); 
    static void create(WSJS* wsjs, const std::string& uri);
    static void destroy(WSJS* wsjs);

public: // sadly those need to be public for EM binding
    static void onopen(uint32_t id);
    static void onerror(uint32_t id);
    static void onclose(uint32_t id);
    static void onmessage(uint32_t id, std::string message);
};

#endif // _WSJS_H
