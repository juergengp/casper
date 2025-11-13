# Building ollamaCode C++ Version

## Prerequisites

### Fedora/RHEL/CentOS
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    make \
    libcurl-devel \
    sqlite-devel \
    readline-devel \
    git
```

### Ubuntu/Debian
```bash
sudo apt-get install -y \
    g++ \
    cmake \
    make \
    libcurl4-openssl-dev \
    libsqlite3-dev \
    libreadline-dev \
    git
```

### macOS
```bash
brew install cmake curl sqlite readline
```

## Build Steps

### 1. Clone/Navigate to Project
```bash
cd /root/ollamaCode/cpp
```

### 2. Create Build Directory
```bash
mkdir build
cd build
```

### 3. Configure with CMake
```bash
cmake ..
```

Or with custom options:
```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DBUILD_TESTS=OFF
```

### 4. Build
```bash
make -j$(nproc)
```

### 5. Install
```bash
sudo make install
```

This will install:
- Binary: `/usr/local/bin/ollamacode`
- Config dir: `~/.config/ollamacode/`

## Build Options

- `CMAKE_BUILD_TYPE` - Debug, Release, RelWithDebInfo (default: Release)
- `BUILD_TESTS` - Build test suite (default: OFF)
- `CMAKE_INSTALL_PREFIX` - Installation prefix (default: /usr/local)

## Quick Build Script

```bash
#!/bin/bash
cd "$(dirname "$0")"
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
echo "Build complete! Binary at: $(pwd)/ollamacode"
echo "To install: sudo make install"
```

## Development Build

For development with debug symbols:

```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

## Troubleshooting

### Missing curl
```bash
# Fedora
sudo dnf install libcurl-devel

# Ubuntu
sudo apt-get install libcurl4-openssl-dev
```

### Missing SQLite
```bash
# Fedora
sudo dnf install sqlite-devel

# Ubuntu
sudo apt-get install libsqlite3-dev
```

### Missing readline
```bash
# Fedora
sudo dnf install readline-devel

# Ubuntu
sudo apt-get install libreadline-dev
```

### CMake version too old
```bash
# Install newer CMake from official website
wget https://github.com/Kitware/CMake/releases/download/v3.27.0/cmake-3.27.0-linux-x86_64.sh
sudo sh cmake-3.27.0-linux-x86_64.sh --prefix=/usr/local --skip-license
```

## Testing

After building:

```bash
# Test if it runs
./ollamacode --version

# Test configuration
./ollamacode --help

# Test connection to Ollama (must be running)
./ollamacode "hello"
```

## Cross-Compilation

### For ARM64
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/arm64-toolchain.cmake
make
```

### For Different Linux Distros
Build in a Docker container:

```bash
docker run -it --rm \
    -v $(pwd):/work \
    -w /work \
    ubuntu:22.04 \
    bash -c "apt-get update && apt-get install -y g++ cmake libcurl4-openssl-dev libsqlite3-dev && mkdir build && cd build && cmake .. && make"
```

## Static Linking (Portable Binary)

For a portable binary:

```bash
cmake .. -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++"
make
```

Note: curl and sqlite will still be dynamically linked unless you build them statically too.

## Performance Optimization

### Release Build with LTO
```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
make
```

### Profile-Guided Optimization
```bash
# 1. Build with profiling
cmake .. -DCMAKE_CXX_FLAGS="-fprofile-generate"
make
./ollamacode "test prompt"

# 2. Rebuild with profile data
cmake .. -DCMAKE_CXX_FLAGS="-fprofile-use"
make
```

## Clean Build

```bash
rm -rf build
mkdir build
cd build
cmake ..
make
```

## Installation Locations

After `sudo make install`:

```
/usr/local/bin/ollamacode           # Binary
~/.config/ollamacode/config.db       # SQLite database
~/.config/ollamacode/history.txt     # Command history
```

## Uninstall

```bash
cd build
sudo make uninstall
# or manually:
sudo rm /usr/local/bin/ollamacode
rm -rf ~/.config/ollamacode
```

## Next Steps

After successful build:

1. Make sure Ollama is running: `ollama serve`
2. Pull a model: `ollama pull llama3`
3. Run ollamaCode: `ollamacode`

## Contributing

When contributing, please:
1. Use Debug build for development
2. Run tests before submitting
3. Follow C++17 standards
4. Use `clang-format` for code formatting

Format code:
```bash
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```
