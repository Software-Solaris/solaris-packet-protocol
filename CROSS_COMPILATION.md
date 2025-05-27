# SPP Cross-Compilation Guide

This guide details how to compile the Solaris Packet Protocol (SPP) library for different architectures using cross-compilation.

## Supported Architectures

### ARM (32-bit)
- **Toolchain**: `arm-linux-gnueabihf-gcc`
- **Architecture**: ARMv7-A with NEON
- **ABI**: Hard float

### RISC-V (64-bit)
- **Toolchain**: `riscv64-linux-gnu-gcc`
- **Architecture**: RV64GC
- **ABI**: LP64D

## Toolchain Installation

### Ubuntu/Debian

```bash
# For ARM
sudo apt-get update
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# For RISC-V
sudo apt-get install gcc-riscv64-linux-gnu g++-riscv64-linux-gnu

# Verify installation
arm-linux-gnueabihf-gcc --version
riscv64-linux-gnu-gcc --version
```

### Fedora/CentOS/RHEL

```bash
# For ARM
sudo dnf install gcc-arm-linux-gnu g++-arm-linux-gnu

# For RISC-V
sudo dnf install gcc-riscv64-linux-gnu g++-riscv64-linux-gnu
```

## Cross-Compilation

### Method 1: Automated Script (Recommended)

```bash
# ARM
./build.sh -x arm -t Release

# RISC-V
./build.sh -x riscv64 -t Release

# With additional options
./build.sh -x arm -t Debug -v -p /opt/spp-arm
```

### Method 2: Manual CMake

```bash
# ARM
mkdir build-arm
cd build-arm
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/arm-linux-gnueabihf.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/spp-arm \
      ..
make -j$(nproc)

# RISC-V
mkdir build-riscv64
cd build-riscv64
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/riscv64-linux-gnu.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/spp-riscv \
      ..
make -j$(nproc)
```

## Creating Custom Toolchain

### Example for a new architecture

```cmake
# cmake/toolchains/my-architecture.cmake

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR my_arch)

# Specify compilers
set(CMAKE_C_COMPILER my-arch-gcc)
set(CMAKE_CXX_COMPILER my-arch-g++)

# Configure sysroot if needed
set(CMAKE_SYSROOT /path/to/sysroot)

# Configure search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Architecture-specific flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=my-arch-flags")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=my-arch-flags")
```

### Using custom toolchain

```bash
./build.sh -T cmake/toolchains/my-architecture.cmake
```

## Cross-Compilation Verification

### Verify library architecture

```bash
# For ARM
file build-arm/core/libspp_core.a
file build-arm/services/databank/libspp_databank.a

# For RISC-V
file build-riscv64/core/libspp_core.a
file build-riscv64/services/databank/libspp_databank.a
```

### Verify symbols

```bash
# List symbols in library
nm build-arm/core/libspp_core.a
objdump -t build-arm/core/libspp_core.a
```

## Cross-Compiled Installation

### Local installation

```bash
# After building
cd build-arm
sudo make install

# Or with CMake
sudo cmake --install . --prefix /opt/spp-arm
```

### Packaging for distribution

```bash
# Create tar package
cd build-arm
make install DESTDIR=/tmp/spp-arm-package
cd /tmp/spp-arm-package
tar -czf spp-arm-1.0.0.tar.gz *

# Create deb package (if you have checkinstall)
cd build-arm
sudo checkinstall --pkgname=spp-arm --pkgversion=1.0.0
```

## Usage in Target Project

### CMakeLists.txt for cross-compiled project

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyARMProject LANGUAGES C)

# Use the same toolchain
set(CMAKE_TOOLCHAIN_FILE "path/to/arm-linux-gnueabihf.cmake")

# Configure SPP path
set(CMAKE_PREFIX_PATH "/opt/spp-arm")

find_package(spp REQUIRED)

add_executable(my_app main.c)
target_link_libraries(my_app spp::spp)
```

### Project compilation

```bash
mkdir build-target
cd build-target
cmake -DCMAKE_TOOLCHAIN_FILE=../arm-linux-gnueabihf.cmake ..
make
```

## Troubleshooting

### Error: "Compiler not found"

```bash
# Verify toolchain is in PATH
which arm-linux-gnueabihf-gcc
export PATH=$PATH:/path/to/toolchain/bin
```

### Error: "Cannot find libraries"

```bash
# Configure PKG_CONFIG_PATH
export PKG_CONFIG_PATH=/opt/spp-arm/lib/pkgconfig:$PKG_CONFIG_PATH

# Or use CMAKE_PREFIX_PATH
cmake -DCMAKE_PREFIX_PATH=/opt/spp-arm ..
```

### Error: "Wrong architecture"

```bash
# Clean previous build
rm -rf build-*
./build.sh -c -x arm
```

## Architecture-Specific Optimizations

### ARM Cortex-A Series

```cmake
# In the toolchain file
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-a53 -mfpu=neon-fp-armv8")
```

### RISC-V with specific extensions

```cmake
# In the toolchain file
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=rv64imafd -mabi=lp64d")
```

## Testing Cross-Compiled Libraries

### Using QEMU

```bash
# Install QEMU
sudo apt-get install qemu-user-static

# Run ARM binary on x86_64
qemu-arm-static -L /usr/arm-linux-gnueabihf/ ./my_app_arm
```

### Automated testing

```bash
# Run tests using QEMU
ctest -VV

# Or with custom test runner
./run_tests.sh arm
``` 