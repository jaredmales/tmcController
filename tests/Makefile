# Hardware-free tests: provide real development headers, but never link libftdi.
CXX ?= c++
FTDI_CFLAGS ?= $(shell pkg-config --cflags libftdi1)
CPPFLAGS += $(FTDI_CFLAGS)
CXXFLAGS ?= -O2 -g
CXXFLAGS += -std=c++17 -Wall -Wextra -Werror

.PHONY: all check clean
all: tmcController_test

tmcController_test: tmcController_test.cpp ftdiHarness.hpp ../tmcController.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(LDFLAGS) -o $@

check: tmcController_test
	./tmcController_test

clean:
	rm -f tmcController_test
