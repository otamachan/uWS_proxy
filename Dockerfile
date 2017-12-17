FROM ubuntu:16.04
RUN apt-get update \
    && apt-get install -y build-essential libssl-dev zlib1g-dev netcat-openbsd tinyproxy \
    && rm -rf /var/lib/apt/lists/*
ADD src /uws_proxy
ADD tinyproxy.conf /etc/tinyproxy.conf
WORKDIR /uws_proxy
RUN patch -d uWebSockets -p1 < uWS.patch \
    && make -C uWebSockets default install
RUN make
RUN adduser test
RUN mkdir -p /var/run/tinyproxy
ENV http_proxy=http://127.0.0.1:8888
CMD /usr/sbin/tinyproxy -d -c /etc/tinyproxy.conf & su test -c /uws_proxy/testBin