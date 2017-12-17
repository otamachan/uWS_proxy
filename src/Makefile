all: testBin


uWebSockets/libuWS.so:
	git -C uWebSockets reset --hard
	patch -d uWebSockets -p1 < uWS.patch
	make -C uWebSockets

main.cpp:
	cp uWebSockets/tests/main.cpp .
	sed -i -e 's/::Hub/::ProxyHub/g' main.cpp
	sed -i -e 's/uWS\.h/ProxyHub\.h/g' main.cpp
	sed -i -e 's|misc|uWebSockets/misc|g' main.cpp

testBin: uWebSockets/libuWS.so main.cpp
	$(CXX) -std=c++11 -O3 ProxyHub.cpp main.cpp -o testBin -I. -IuWebSockets/src  -LuWebSockets -Wl,-rpath=uWebSockets -lpthread -luWS -lssl -lcrypto -lz

clean:
	rm -f uWebSockets/libuWS.so main.cpp testBin
