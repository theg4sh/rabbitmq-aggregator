.DEFAULT_GOAL := all
LIBRABBITMQ_C_PATH=./thirdparty/rabbitmq-c

LIBRABBITMQ_INCLUDE_PATH=$(LIBRABBITMQ_C_PATH)/librabbitmq
LIBRABBITMQ_LIBRARY_PATH=$(LIBRABBITMQ_C_PATH)/build/librabbitmq

CXX_INCLUDE_PATH=-I./include -I$(LIBRABBITMQ_INCLUDE_PATH)
CXX_LIBRARIES=-L$(LIBRABBITMQ_LIBRARY_PATH) -lrabbitmq -lpthread

CXX_FLAGS=-std=c++14 -Wall -g $(CXX_INCLUDE_PATH)
CXX=g++ $(CXX_FLAGS)

SOURCES := \
	src/utils.o \
	src/error.o \
	src/resolver.o \
	src/channel.o \
	src/connection.o \
	src/consumer.o \
	src/publisher.o \
	src/operation.o \
	src/main.o

.PHONY+=tests all

include test/Makefile.in

all: $(LIBRABBITMQ_LIBRARY_PATH)/librabbitmq.so rmq-aggregator

rmq-aggregator: $(SOURCES)
	$(CXX) -o $@ $^ $(CXX_LIBRARIES)

$(LIBRABBITMQ_INCLUDE_PATH):
	git submodule update --init --recursive

$(LIBRABBITMQ_LIBRARY_PATH)/librabbitmq.so: $(LIBRABBITMQ_INCLUDE_PATH)
	mkdir -p $(LIBRABBITMQ_C_PATH)/build
	cd $(LIBRABBITMQ_C_PATH)/build && cmake .. && make

clean: clean-tests
	rm -f $(SOURCES) ./rmq-aggregator

clean-all: clean
	rm -rf $(LIBRABBITMQ_C_PATH)/build

tests: $(LIBRABBITMQ_LIBRARY_PATH)/librabbitmq.so $(TEST_TARGETS)

%.o: %.cpp
	$(CXX) -c -o $@ $^ $(CXX_LIBRARIES)
