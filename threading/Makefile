
all:
	$(CC) -c -g -std=c++11 -fPIC -I/usr/include/qt5 -I../cc-desk/src test_threading.cc threading.cc
	$(CC) test_threading.o threading.o -lboost_thread -lboost_system \
		-lstdc++ -lQt5Widgets -lQt5Core -o app

clean:
	rm *.o app