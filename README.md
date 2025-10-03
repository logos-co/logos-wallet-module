# Logos Wallet Module

A Logos module that provides wallet functionality, including balance fetching and transaction management via the go-wallet-sdk.

## Prerequisites

- CMake 3.14 or later
- Qt 6 (or Qt 5)
- Go compiler (for building go-wallet-sdk)
- C++ compiler with C++17 support

## Building

### Using the compile script (recommended)

```bash
# Build with default settings (Release mode)
./scripts/compile.sh

# Build in Debug mode
./scripts/compile.sh --debug

# Clean build
./scripts/compile.sh --clean

# Specify Qt installation directory
./scripts/compile.sh --qt-dir /path/to/Qt/6.x.x/macos
```

### Manual build

```bash
# Initialize submodules
git submodule update --init --recursive

# Build the wallet library from go-wallet-sdk
./build_wallet_lib.sh

# Configure and build
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DLOGOS_WALLET_MODULE_USE_VENDOR=ON
cmake --build . --target wallet_module_plugin
```

## Building with Nix

```bash
# Build
nix build

# Enter development shell
nix develop
```

## Module Structure

- `wallet_module_plugin.cpp/h` - Main plugin implementation
- `wallet_module_interface.h` - Module interface definition
- `metadata.json` - Module metadata
- `lib/` - Contains the built wallet library
- `vendor/` - Vendored dependencies (logos-liblogos, logos-cpp-sdk, go-wallet-sdk)

## Output

The built plugin will be in `build/modules/wallet_module_plugin.dylib` (macOS) or `build/modules/wallet_module_plugin.so` (Linux).

## Usage

The wallet module can be loaded by the Logos core system and provides wallet-related capabilities to applications.

## Dependencies

- **logos-liblogos** - Core Logos library interface
- **logos-cpp-sdk** - C++ SDK for building Logos modules
- **go-wallet-sdk** - Go-based wallet SDK (compiled to C library)

