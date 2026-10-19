# Test fixtures

These clips are inputs for the tests in `tests/core/FrameExtractorTest.cpp`.
The tests hash the extracted pixels and compare them against stored values, so
the files must stay byte-for-byte unchanged. `.gitattributes` marks this folder
as binary to stop git from altering them.

| File | Codec | Size | Pixel format | Frames | Covers |
|---|---|---|---|---|---|
| `fixture-8bit.mp4` | H.264 High | 320x240 | `yuv420p` | 25 at 25 fps (1 s) | 8-bit conversion, BT.601 default, frame timing |
| `fixture-10bit.mp4` | H.264 High 10 | 320x240 | `yuv420p10le` | 25 at 25 fps (1 s) | 10-bit to 8-bit conversion, BT.601 default |

Both clips leave color space and range unsignaled, so the tests also exercise
the defaults from F6. At 240 lines that means BT.601, limited range.

Frames are 0.04 s apart, so there is no frame at exactly 0.5 s. A request for
0.5 s must return the frame at 0.48 s (the greatest timestamp at or before the
requested time), not the one at 0.52 s. `Fixture8BitTimestamps` checks this.

## How they were made

Both clips use FFmpeg's `testsrc2` pattern, which has sharp color edges and
gradients. That is where different conversion code paths are most likely to
disagree. The 320x240 size is large enough for swscale to use the same SIMD
paths it uses on real video.

```sh
ffmpeg -f lavfi -i testsrc2=size=320x240:rate=25:duration=1 \
  -c:v libx264 -pix_fmt yuv420p -bitexact fixture-8bit.mp4

ffmpeg -f lavfi -i testsrc2=size=320x240:rate=25:duration=1 \
  -c:v libx264 -pix_fmt yuv420p10le -bitexact fixture-10bit.mp4
```

The encoder was x264 core 165 at its default settings (CRF 23). `-bitexact`
keeps version strings out of the files.

## Changing a fixture

Please do not re-encode these files to "refresh" them. A different FFmpeg or x264
version can produce different bytes from the same command, which changes the
decoded pixels and breaks the stored hashes.

If you need a new fixture, add a new file instead. Record the expected hash from
the test's failure message, then confirm that macOS and Linux give the same
value before committing it.
