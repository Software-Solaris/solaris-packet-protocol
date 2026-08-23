# Runs the Cgreen unit tests under tests/.
# Dependencies: see tests/README.md.
#
# Usage:
#   make        -> configure, build and run all tests
#   make test   -> same as `make`
#   make build  -> configure and build, without running tests
#   make clean  -> remove the test build directory

BUILD_DIR := build-tests

.PHONY: all test build configure clean

all: test

configure:
	cmake -S . -B $(BUILD_DIR) -DSPP_BUILD_TESTS=ON -DSPP_PORT=posix

build: configure
	cmake --build $(BUILD_DIR)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)
