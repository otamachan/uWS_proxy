#include <iostream>
#include <uWS.h>
#include "ProxyHub.h"
#include "ProxyHTTPSocket.h"

namespace uWS {

template <bool isServer>
struct WIN32_EXPORT ProxyGroup : Group<isServer> {
    friend struct ProxyHub;
};

uS::Socket *allocateProxyHttpSocket(uS::Socket *s) {
    return (uS::Socket *) new ProxyHttpSocket(s);
}

uS::Socket *allocateHttpSocket(uS::Socket *s) {
    return (uS::Socket *) new HttpSocket<CLIENT>(s);
}

void ProxyHub::onProxyClientConnection(uS::Socket *s, bool error) {
    ProxyHttpSocket *httpProxySocket = (ProxyHttpSocket *) s;
    if (error) {
        httpProxySocket->onEnd(httpProxySocket);
    } else {
        httpProxySocket->setState<ProxyHttpSocket>();
        httpProxySocket->change(httpProxySocket->nodeData->loop, httpProxySocket, httpProxySocket->setPoll(UV_READABLE));
        httpProxySocket->connect();
    }
}

bool parseURI(std::string &uri, bool &secure, std::string &hostname, int &port, std::string &path) {
    port = 80;
    secure = false;
    size_t offset = 5;
    if (!uri.compare(0, 6, "wss://")) {
        port = 443;
        secure = true;
        offset = 6;
    } else if (!uri.compare(0, 5, "ws://")) {
        offset = 5;
    } else if (!uri.compare(0, 8, "https://")) {
        port = 443;
        secure = true;
        offset = 8;
    } else if (!uri.compare(0, 7, "http://")) {
        offset = 7;
    } else {
        return false;
    }

    if (offset == uri.length()) {
        return false;
    }

    if (uri[offset] == '[') {
        if (++offset == uri.length()) {
            return false;
        }
        size_t endBracket = uri.find(']', offset);
        if (endBracket == std::string::npos) {
            return false;
        }
        hostname = uri.substr(offset, endBracket - offset);
        offset = endBracket + 1;
    } else {
        hostname = uri.substr(offset, uri.find_first_of(":/", offset) - offset);
        offset += hostname.length();
    }

    if (offset == uri.length()) {
        path.clear();
        return true;
    }

    if (uri[offset] == ':') {
        offset++;
        std::string portStr = uri.substr(offset, uri.find('/', offset) - offset);
        if (portStr.length()) {
            try {
                port = stoi(portStr);
            } catch (...) {
                return false;
            }
        } else {
            return false;
        }
        offset += portStr.length();
    }

    if (offset == uri.length()) {
        path.clear();
        return true;
    }

    if (uri[offset] == '/') {
        path = uri.substr(++offset);
    }
    return true;
}

void ProxyHub::connect(std::string uri, void *user, std::map<std::string, std::string> extraHeaders, int timeoutMs, Group<CLIENT> *eh) {
    if (!eh) {
        eh = (Group<CLIENT> *) this;
    }
    int port;
    bool secure;
    std::string hostname, path;
    if (!parseURI(uri, secure, hostname, port, path)) {
        ((ProxyGroup<CLIENT> *)eh)->errorHandler(user);
    } else {
        ProxyHttpSocket *httpSocket = nullptr;
        int proxy_port;
        bool proxy_secure;
        std::string proxy_hostname, proxy_path;
        if (std::getenv("http_proxy")) {
            std::string proxy(std::getenv("http_proxy"));
            if (parseURI(proxy, proxy_secure, proxy_hostname, proxy_port, proxy_path)) {
                 httpSocket = (ProxyHttpSocket *) uS::Node::connect<allocateProxyHttpSocket, onProxyClientConnection>(proxy_hostname.c_str(), proxy_port, proxy_secure, eh);
                 if (httpSocket) {
                     httpSocket->startTimeout<ProxyHttpSocket::onEnd>(timeoutMs);
                     httpSocket->hostname = hostname;
                     httpSocket->port = port;
                     httpSocket->secure = secure;
                 }
            }
        } else {
            httpSocket = (ProxyHttpSocket *) uS::Node::connect<allocateProxyHttpSocket, onClientConnection>(hostname.c_str(), port, secure, eh);
            if (httpSocket) {
                httpSocket->startTimeout<ProxyHttpSocket::onEnd>(timeoutMs);
            }
        }
        if (httpSocket) {
            // startTimeout occupies the user
            httpSocket->httpUser = user;

            std::string randomKey = "x3JJHMbDL1EzLkh9GBhXDw==";
//            for (int i = 0; i < 22; i++) {
//                randomKey[i] = rand() %
//            }

            httpSocket->httpBuffer = "GET /" + path + " HTTP/1.1\r\n"
                                     "Upgrade: websocket\r\n"
                                     "Connection: Upgrade\r\n"
                                     "Sec-WebSocket-Key: " + randomKey + "\r\n"
                                     "Host: " + hostname + "\r\n" +
                                     "Sec-WebSocket-Version: 13\r\n";

            for (std::pair<std::string, std::string> header : extraHeaders) {
                httpSocket->httpBuffer += header.first + ": " + header.second + "\r\n";
            }

            httpSocket->httpBuffer += "\r\n";
        } else {
             ((ProxyGroup<CLIENT> *)eh)->errorHandler(user);
        }
    }
}

}
