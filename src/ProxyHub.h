#include <iostream>
#include <uWS.h>

namespace uWS {

struct ProxyHub : public Hub {
private:
    friend struct ProxyHttpSocket;
public:
    static void onProxyClientConnection(uS::Socket *s, bool error);
    void connect(std::string uri, void *user = nullptr, std::map<std::string, std::string> extraHeaders = {}, int timeoutMs = 5000, Group<CLIENT> *eh = nullptr);
};

}
