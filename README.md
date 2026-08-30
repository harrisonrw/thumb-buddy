# thumb-buddy

## Building

### Requirements

- CMake 4.2 or newer
- A C++20 compiler (Clang, GCC, or AppleClang)

### Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The CLI is written to `build/cli/thumbbuddy`. Use `-DCMAKE_BUILD_TYPE=Release`
for an optimised build.

### Test

```sh
ctest --test-dir build --output-on-failure
```

Tests use GoogleTest, which CMake downloads during the first configure, so that
step needs network access. Configure with `-DBUILD_TESTING=OFF` to skip both the
download and the tests.

### Install

```sh
cmake --install build --prefix /usr/local
```

Installs the `thumbbuddy` binary, the `thumbbuddy_core` static library, and the
public headers under `thumbbuddy/`.

### Package

```sh
cpack --config build/CPackConfig.cmake
```

Produces a `.tar.gz` named for the platform, e.g. `thumbbuddy-0.1-Darwin.tar.gz`.

## License

Released under the [MIT License](LICENSE).
