#include <uWS.h>

namespace uWS {

struct ProxyHttpSocket : HttpSocket<CLIENT> {
public:
    ProxyHttpSocket(Socket *socket) : HttpSocket<CLIENT>(socket) {}
    void connect() {
        std::string buf = "CONNECT " + hostname + ":" + std::to_string(port) + " HTTP/1.1\r\n\r\n";
        Queue::Message *messagePtr = allocMessage(buf.length(), buf.data());
        bool wasTransferred;
        if (write(messagePtr, wasTransferred)) {
            if (!wasTransferred) {
                freeMessage(messagePtr);
            } else {
                messagePtr->callback = nullptr;
            }
        } else {
            freeMessage(messagePtr);
        }
    }
    static void onEnd(uS::Socket *s) {
        HttpSocket<CLIENT>::onEnd(s);
    }
    static uS::Socket *onData(uS::Socket *s, char *data, size_t length) {
        std::string ss(data, length);
        ProxyHttpSocket *httpProxySocket = (ProxyHttpSocket *) s;
        httpProxySocket->cork(true);
        SSL *ssl = nullptr;
        uS::NodeData *nodeData = httpProxySocket->getNodeData();
        if (httpProxySocket->secure) {
            ssl = SSL_new(nodeData->clientContext);
            SSL_set_connect_state(ssl);
            SSL_set_tlsext_host_name(ssl, httpProxySocket->hostname.c_str());
        }
        Socket initialSocket(nodeData, httpProxySocket->nodeData->loop, httpProxySocket->getFd(), ssl);
        HttpSocket<CLIENT> *httpSocket = new HttpSocket<CLIENT>(&initialSocket);
        httpProxySocket->cancelTimeout();
        httpProxySocket->close(nodeData->loop, [](Poll *p) {
            delete (ProxyHttpSocket *) p;
        });
        httpSocket->httpUser = httpProxySocket->httpUser;
        httpSocket->httpBuffer = httpProxySocket->httpBuffer;
        httpSocket->cork(true);
        ProxyHub::onClientConnection(httpSocket, false);
        httpSocket->cork(false);
        return httpSocket;
    }
    std::string hostname;
    int port;
    bool secure;
private:
    friend struct ProxyHub;
};

}
