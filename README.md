# uWS_proxy

Using [uWebSockets](https://github.com/uNetworking/uWebSockets) as a websocket clinet from behind a proxy.

# Test(using tinyproxy)

## Docker

```bash
git clone ...
cd uws_proxy
docker build -t uws_proxy .
docker run --rm -it uws_proxy
```

## Ubuntu 16.04

```bash
sudo apt-get install tinyproxy
```

```bash
git clone ...
cd uws_proxy
git submodule init
git submodule update
sudo cp tinyproxy.conf /etc/tinyproxy.conf
sudo systemctl restart tinyproxy
cd src
make
export http_proxy=http://127.0.0.1:8888
./testBin
```
