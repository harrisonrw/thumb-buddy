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

The build, CLI scaffolding, media inspection, and frame extraction are in place.
`thumbbuddy --info <path>` reports duration, resolution, frame rate, color
depth, bit rate, and codec. `thumbbuddy <path>` extracts a frame every 5 seconds
and writes each one as a PNG. The analysis pipeline and candidate ranking are
work in progress.

## Usage

```sh
thumbbuddy --info video.mp4        # print metadata
thumbbuddy video.mp4               # write video-01.png, video-02.png, ... next to the video
thumbbuddy -o frames video.mp4     # write the PNGs into ./frames instead
```

Frames are converted to 8-bit RGBA. If a video does not signal its color space,
BT.709 is assumed at 720 lines or more and BT.601 below that, both limited range.
The output directory must already exist.

## Building

### Requirements

- CMake 4.2 or newer
- A C++20 compiler (Clang, GCC, or AppleClang)
- FFmpeg 8.0 or newer, with development headers, found through `pkg-config`

On macOS:

```sh
brew install pkg-config ffmpeg
```

On Ubuntu 26.04 (earlier releases do not ship FFmpeg 8 in apt):

```sh
sudo apt-get install pkg-config libavformat-dev libavcodec-dev libavutil-dev libswscale-dev
```

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

Some tests decode the small clips in `tests/fixtures/` and compare a hash of the
extracted pixels against a stored value. Frame conversion must give identical
pixels on every platform, so CI runs these tests on both macOS (ARM) and Linux
(x86). If one of them fails, the conversion output has changed on that platform.
Do not re-encode the fixtures: a new encode gives new pixels, and the stored
hashes would need to be updated.

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
