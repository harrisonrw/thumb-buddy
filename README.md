# thumb-buddy

[![Development](https://github.com/harrisonrw/thumb-buddy/actions/workflows/development.yml/badge.svg?branch=main&event=push)](https://github.com/harrisonrw/thumb-buddy/actions/workflows/development.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Thumb Buddy is a cross-platform C++ video thumbnail engine. It decodes and
samples frames with FFmpeg, scores them on image-quality metrics such as
sharpness, brightness, and motion, and ranks a small set of strong, visually
distinct thumbnail candidates. A later, optional AI layer will re-rank those
candidates semantically and explain its choices.

The portable core carries no Apple-specific dependencies and is consumed by a
CLI on macOS and Linux, with a native SwiftUI macOS app planned on top of the
same engine.

### Status

The build, CLI scaffolding, and FFmpeg-backed media inspection are in place.
A CLI command, `thumbbuddy --info <path>`, reports duration, resolution, frame rate, color
depth, bit rate, and codec. Frame extraction, the analysis pipeline, and
candidate ranking are work in progress.

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
