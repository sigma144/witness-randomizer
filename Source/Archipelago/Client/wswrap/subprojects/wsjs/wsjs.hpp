#ifndef _WSJS_H
#define _WSJS_H

#include <string>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#include <memory>
#include <map>
#include <functional>
#include <stdint.h>
#include <mutex>


#ifdef DEBUG_WSJS
#define wsjs_debug log
#else
#define wsjs_debug(...)
#endif


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
    
    unsigned long get_ok_connect_interval() const
    {
        auto& objects = Static::getObjects();
        auto n = objects.size();
        unsigned long time_per_socket = 30000; // TODO: detect browser
        return n>0 ? time_per_socket * n : time_per_socket;
    }

    void send(const std::string& data)
    {
        bool isBinary = data.find('\0') != data.npos; // TODO: detect if data is valid UTF8
        if (isBinary)
            send_binary(data);
        else
            send_text(data);
    }

    void send_text(const std::string& data)
    {
        // utf8 string -> text
        EM_ASM({
            Module.websockets[$0].send(UTF8ToString($1));
        }, _id, data.c_str());
    }

    void send_binary(const std::string& data)
    {
        // binary
        EM_ASM({
            Module.websockets[$0].send(Module.HEAPU8.subarray($1, $1 + $2));
        }, _id, data.c_str(), data.length());
    }

private:
    uint32_t _id;
    onopen_handler _hopen;
    onclose_handler _hclose;
    onmessage_handler _hmessage;
    onerror_handler _herror;

    class Static {
        static Static& instance()
        {
            static Static instance;
            return instance;
        }

        static Static& get()
        {
            static std::once_flag flag;
            std::call_once(flag, [] {
                emscripten::function("wsjs_onopen", &WSJS::onopen);
                emscripten::function("wsjs_onclose", &WSJS::onclose);
                emscripten::function("wsjs_onmessage", &WSJS::onmessage);
                emscripten::function("wsjs_onerror", &WSJS::onerror);
                instance();
            });
            return instance();
        }

        uint32_t _nextId = 0;
        std::map<unsigned, WSJS*> _objects;

        uint32_t nextIdImpl()
        {
            while (_objects.find(_nextId) != _objects.end()) _nextId++; // required if we wrap around
            auto res = _nextId;
            _nextId++; // post-increment to avoid reuse of ID
            return res;
        }

    public:
        static int getNextId()
        {
            return get().nextIdImpl();
        }

        static std::map<unsigned, WSJS*>& getObjects()
        {
            return get()._objects;
        }
    };

    static void create(WSJS* wsjs, const std::string& uri)
    {
        wsjs_debug("create(%u)\n", (unsigned)wsjs->_id);
        auto& objects = Static::getObjects();
        wsjs->_id = Static::getNextId();
        objects[wsjs->_id] = wsjs;
        EM_ASM({
            var uri = UTF8ToString($0);
            console.log('Connecting to ' + uri);
            if (typeof Module.websockets === 'undefined') Module.websockets = {};
            var ws = new WebSocket(uri);
            ws.binaryType = "arraybuffer";
            ws.id = $1;
            Module.websockets[$1] = ws;
            ws.onopen = function() { if (this.id !== null) Module.wsjs_onopen(this.id); };
            ws.onerror = function() { if (this.id !== null) Module.wsjs_onerror(this.id); };
            ws.onclose = function() { if (this.id !== null) Module.wsjs_onclose(this.id); };
            ws.onmessage = function(e) { if (this.id !== null) Module.wsjs_onmessage(this.id, e.data); };
        }, uri.c_str(), wsjs->_id);
    }

    static void destroy(WSJS* wsjs)
    {
        wsjs_debug("destroy(%u)\n", (unsigned)wsjs->_id);
        auto& objects = Static::getObjects();
        objects.erase(wsjs->_id);
        EM_ASM({
            // TODO: check if websockets[$0] exists
            Module.websockets[$0].id = null;
            Module.websockets[$0].close();
            delete Module.websockets[$0];
        }, wsjs->_id);
    }

    static void warn(const char* fmt, ...)
    {
        fprintf(stderr, "WSJS: ");
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
    static void log(const char* fmt, ...)
    {
        fprintf(stdout, "WSJS: ");
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
    }

public: // sadly those need to be public for EM binding
    static void onopen(uint32_t id)
    {
        auto& objects = Static::getObjects();
        auto it = objects.find(id);
        if (it == objects.end()) {
            warn("onopen(%u): no such id\n", (unsigned)id);
            return;
        }
        if (!it->second->_hopen) return;
        wsjs_debug("onopen(%u)\n", id);
        it->second->_hopen();
    }

    static void onerror(uint32_t id)
    {
        auto& objects = Static::getObjects();
        auto it = objects.find(id);
        if (it == objects.end()) {
            warn("onerror(%u): no such id\n", (unsigned)id);
            return;
        }
        if (!it->second->_herror) return;
        wsjs_debug("onerror(%u)\n", id);
        it->second->_herror();
    }

    static void onclose(uint32_t id)
    {
        auto& objects = Static::getObjects();
        auto it = objects.find(id);
        if (it == objects.end()) {
            warn("onclose(%u): no such id\n", (unsigned)id);
            return;
        }
        if (!it->second->_hclose) return;
        wsjs_debug("onclose(%u)\n", id);
        it->second->_hclose();
    }

    static void onmessage(uint32_t id, std::string data)
    {
        auto& objects = Static::getObjects();
        auto it = objects.find(id);
        if (it == objects.end()) {
            warn("onmessage(%u): no such id\n", (unsigned)id);
            return;
        }
        if (!it->second->_hmessage) return;
        std::string s = data;
        wsjs_debug("onmessage(%u)\n", id);
        it->second->_hmessage(s);
    }
};

#endif // _WSJS_H
