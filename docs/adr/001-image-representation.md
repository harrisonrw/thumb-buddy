# ADR 001: Image representation

- Status: Accepted
- Date: 2026-10-02
- Applies to: version 0.1

## Context

Core decodes video frames with FFmpeg, analyzes them, and hands the selected
frames to its consumers. The CLI writes them as JPEG and PNG files. The planned
macOS app will display them.

The decoder produces frames in whatever pixel format the stream uses. This is
usually planar YUV 4:2:0 at 8 or 10 bits, held in an FFmpeg-owned `AVFrame`.
Core needs one image type of its own. That type must:

- keep FFmpeg types out of the public API (requirement A1)
- have a single, fully specified layout, so results are deterministic and
  comparable across platforms (Q1, Q2)
- be usable from Swift and by image encoders without platform-specific types
  (A3)
- be simple enough to build by hand in a unit test

Version 0.1 supports SDR input only, so 8 bits per channel is enough.

## Options considered

1. **Expose the decoder's format.** This means planar YUV, through `AVFrame` or
   a wrapper. There is no conversion cost, and luma is available directly for
   analysis. The problem is that every consumer would have to handle many pixel
   formats, bit depths, and color matrices. FFmpeg concepts would also leak
   through the API.
2. **RGB8, 3 bytes per pixel.** This is the smallest RGB layout and a natural
   fit for JPEG. However, pixels are not 4-byte aligned, and Core Graphics has
   no 24-bit RGB bitmap format. The macOS app would have to repack every image.
3. **RGBA8, 4 bytes per pixel.** FFmpeg's scaler can output this format
   directly (`AV_PIX_FMT_RGBA`). Core Graphics and PNG encoders accept it
   as-is, and pixels are 4-byte aligned. It uses a third more memory than RGB8,
   and the alpha channel carries no information.
4. **BGRA8.** This is the other common native layout on Apple platforms. It
   offers nothing over RGBA8 for Core, and it is less conventional for file
   encoders and on Linux.
5. **16-bit or floating-point RGB.** This keeps the full 10-bit precision and
   would suit HDR. It also doubles or quadruples memory and complicates every
   consumer. There is no benefit while input is SDR and output is 8-bit JPEG
   or PNG.

## Decision

Core represents images as **RGBA8 image buffers**:

- Four 8-bit channels per pixel, in R, G, B, A byte order.
- Alpha is fixed at 255.
- The first pixel is the top-left pixel. Rows are stored top-to-bottom and
  pixels left-to-right.
- Rows are tightly packed. The row stride is `width * 4` bytes.
- Width and height are the final display dimensions, after rotation and
  pixel-aspect-ratio correction.
- No gamut conversion is applied. Values keep the source's primaries and
  transfer function.

The image is a value type that owns its bytes. The caller owns the image
object. The buffer is valid for the lifetime of that object, even after the
engine that produced it is destroyed.

Conversion from the decoder's format happens once, inside Core, behind this
type. That includes the color matrix, the range, and the 10-bit to 8-bit
reduction.

The authoritative definition is the *image buffer* entry in
`docs/requirements-v0.1.md`. If the two documents disagree, the requirements
document is correct.

## Consequences

The benefits:

- Consumers handle exactly one pixel format.
- The macOS app can wrap a buffer in a `CGImage` without repacking. No
  platform-specific pixel-buffer type appears in the Core API.
- Tests can build images directly from bytes, without FFmpeg.

The costs:

- Memory is a third higher than RGB8. A 1080p frame is about 8 MB and a 4K
  frame is about 33 MB. This counts against the peak-memory targets, so Core
  should not hold many full-resolution frames at once.
- Every sampled frame pays for a YUV-to-RGB conversion. Metrics that need luma
  have to derive it from RGB, because the decoder's luma plane is gone by then.
- JPEG output has to drop the alpha channel when encoding.
- The fixed stride of `width * 4` rules out row padding. A buffer cannot wrap
  memory that has a different stride without a copy.
- Precision beyond 8 bits is discarded. 10-bit SDR input loses its extra bits.

## What would change this decision

- **HDR or wide-gamut support.** Both need more than 8 bits per channel, and
  explicit color-space metadata on the image.
- **Profiling evidence** that color conversion or RGB-derived luma dominates
  processing time. In that case, analysis could run on the decoder's luma plane
  internally, and RGBA8 would remain the public format for selected frames
  only.
- **A zero-copy requirement** from the Swift boundary in 0.2. This could
  require a caller-supplied buffer or a variable stride.
