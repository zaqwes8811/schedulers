

all:
	g++ -c -I../../extern/gmock-1.6.0/fused-src \
		test_asio_and_threading.cc

	#g++ -c \
	#	-I../../extern/gmock-1.6.0/fused-src \
	#	../../extern/gmock-1.6.0/fused-src/gmock-gtest-all.cc

	g++ \
		gmock-gtest-all.o test_asio_and_threading.o  \
		-lboost_system -lboost_thread -lpthread -lrt