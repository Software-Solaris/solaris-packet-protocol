#!/bin/bash

# Build script for Solaris Packet Protocol (SPP)
# Usage: ./build.sh [options]

set -e  # Exit on error

# Default configuration
BUILD_TYPE="Release"
INSTALL_PREFIX="/usr/local"
BUILD_DIR="build"
CLEAN_BUILD=false
CROSS_COMPILE=""
TOOLCHAIN_FILE=""
VERBOSE=false
INIT_SUBMODULES=false

# Function to list available toolchains
list_available_toolchains() {
    echo "Available toolchains:"
    if [ -d "cmake/toolchains" ]; then
        for toolchain in cmake/toolchains/*.cmake; do
            if [ -f "$toolchain" ]; then
                basename "$toolchain" .cmake | sed 's/^/  - /'
            fi
        done
    else
        echo "  No toolchains directory found"
    fi
}

# Help function
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "OPTIONS:"
    echo "    -h, --help              Show this help message"
    echo "    -t, --type TYPE         Build type (Debug|Release) [default: Release]"
    echo "    -p, --prefix PREFIX     Installation prefix [default: /usr/local]"
    echo "    -b, --build-dir DIR     Build directory [default: build]"
    echo "    -c, --clean             Clean previous build"
    echo "    -x, --cross ARCH        Cross-compilation for architecture"
    echo "    -T, --toolchain FILE    Custom toolchain file"
    echo "    -v, --verbose           Verbose output"
    echo "    -j, --jobs N            Number of parallel jobs [default: number of CPUs]"
    echo "    -s, --submodules        Initialize and update git submodules"
    echo "    -l, --list-toolchains   List available toolchains"
    echo ""
    echo "EXAMPLES:"
    echo "    $0                                          # Basic build"
    echo "    $0 -t Debug -v                            # Debug build with verbose output"
    echo "    $0 -c -t Release                          # Clean and build release"
    echo "    $0 -x arm-linux-gnueabihf                 # Cross-compilation for ARM"
    echo "    $0 -x riscv64-linux-gnu                   # Cross-compilation for RISC-V"
    echo "    $0 -x esp32s3                             # Cross-compilation for ESP32-S3"
    echo "    $0 -T custom-toolchain.cmake             # Use custom toolchain"
    echo "    $0 -s                                     # Initialize submodules and build"
    echo "    $0 -l                                     # List available toolchains"
    echo ""
    list_available_toolchains
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -x|--cross)
            CROSS_COMPILE="$2"
            shift 2
            ;;
        -T|--toolchain)
            TOOLCHAIN_FILE="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -s|--submodules)
            INIT_SUBMODULES=true
            shift
            ;;
        -l|--list-toolchains)
            list_available_toolchains
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Configure number of jobs
if [[ -z "$JOBS" ]]; then
    JOBS=$(nproc 2>/dev/null || echo 4)
fi

# Configure verbose output
if [[ "$VERBOSE" == true ]]; then
    CMAKE_VERBOSE=""
    MAKE_VERBOSE="VERBOSE=1"
else
    CMAKE_VERBOSE=""
    MAKE_VERBOSE=""
fi

# Initialize submodules if requested
if [[ "$INIT_SUBMODULES" == true ]]; then
    echo "Initializing and updating submodules..."
    git submodule update --init --recursive
fi

# Configure cross-compilation
if [[ -n "$CROSS_COMPILE" ]]; then
    # Check if toolchain file exists
    POTENTIAL_TOOLCHAIN="cmake/toolchains/${CROSS_COMPILE}.cmake"
    
    if [[ -f "$POTENTIAL_TOOLCHAIN" ]]; then
        TOOLCHAIN_FILE="$POTENTIAL_TOOLCHAIN"
        BUILD_DIR="${BUILD_DIR}-${CROSS_COMPILE}"
        echo "Found toolchain: $TOOLCHAIN_FILE"
        
        # Special handling for ESP32-S3
        if [[ "$CROSS_COMPILE" == "esp32s3" ]]; then
            echo "Verifying ESP-IDF environment..."
            if [[ -z "$IDF_PATH" ]]; then
                echo "Error: IDF_PATH is not set. Please run:"
                echo "  source \$IDF_PATH/export.sh"
                exit 1
            fi
            echo "ESP-IDF found at: $IDF_PATH"
        fi
    else
        echo "Error: Toolchain file not found: $POTENTIAL_TOOLCHAIN"
        echo ""
        list_available_toolchains
        exit 1
    fi
fi

# Configure CMake arguments
CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX"
)

if [[ -n "$TOOLCHAIN_FILE" ]]; then
    if [[ ! -f "$TOOLCHAIN_FILE" ]]; then
        echo "Error: Toolchain file not found: $TOOLCHAIN_FILE"
        exit 1
    fi
    CMAKE_ARGS+=("-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE")
fi

echo "=== Build Configuration ==="
echo "Build type: $BUILD_TYPE"
echo "Build directory: $BUILD_DIR"
echo "Installation prefix: $INSTALL_PREFIX"
echo "Parallel jobs: $JOBS"
if [[ -n "$CROSS_COMPILE" ]]; then
    echo "Cross-compilation: $CROSS_COMPILE"
    echo "Toolchain: $TOOLCHAIN_FILE"
fi
if [[ "$INIT_SUBMODULES" == true ]]; then
    echo "Submodules: Initialized"
fi
echo "======================================"

# Clean if requested
if [[ "$CLEAN_BUILD" == true ]]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake..."
if [[ "$VERBOSE" == true ]]; then
    cmake "${CMAKE_ARGS[@]}" ..
else
    cmake "${CMAKE_ARGS[@]}" .. > /dev/null
fi

# Build
echo "Building library..."
if [[ "$VERBOSE" == true ]]; then
    cmake --build . --config "$BUILD_TYPE" -j "$JOBS" -- $MAKE_VERBOSE
else
    cmake --build . --config "$BUILD_TYPE" -j "$JOBS"
fi

# Show library information
echo ""
echo "=== Build Completed ==="
echo "Generated library files:"
find . -name "*.a" -type f | head -10

echo ""
echo "To install the library, run:"
echo "  cd $BUILD_DIR && sudo cmake --install ."

echo ""
echo "To use the library in another project:"
echo "  find_package(spp REQUIRED)"
echo "  target_link_libraries(your_target spp::spp)"

# ESP32-S3 specific information
if [[ "$CROSS_COMPILE" == "esp32s3" ]]; then
    echo ""
    echo "=== ESP32-S3 Specific Information ==="
    echo "To use in an ESP-IDF project:"
    echo "  1. Copy the libraries to your ESP-IDF project"
    echo "  2. Add SPP includes to your CMakeLists.txt"
    echo "  3. Link with SPP libraries in your component"
fi
