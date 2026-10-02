# Core Requirements v0.1

This document lists the requirements for version 0.1 of Thumb Buddy `Core`,
the C++ engine. The CLI and the planned macOS app use the engine. They are not
covered here.

In this document, "must" marks required behavior, "should" marks a target or
recommendation, and "may" marks optional behavior.

## Primary requirement

Given a supported video, Core must return a small set of thumbnail candidates
that are technically usable and temporally distinct. The user must not have to
inspect the whole video to get them.

- **Technically usable** means the frame passes the rejection thresholds (Q3).
- **Temporally distinct** means the candidates are separated in time (Q4).
  Core does not check for visually similar frames in 0.1.

### Acceptance criterion

Run every video in the reference set with the default configuration. Every
returned candidate must pass Q3 and Q4. In addition:

- An **ordinary** reference video must return the default N candidates (5).
  At least 3 of the 5 must be judged acceptable in a manual review.
- A **limited-candidate** reference video may return fewer than N. A majority
  of the returned candidates must be judged acceptable. At least one candidate
  must be returned, unless the reference-set entry records that none is
  expected.

A candidate is **acceptable** if the reviewer would use it as a thumbnail for
that video. This is a subjective judgment. The reviewer and the result for
each candidate are recorded.

Results go in the v0.1 acceptance-results document. That document also records
the number of visually near-duplicate candidates per video. In 0.1 this count
is an observation, not a pass criterion.

## Definitions

- **Selected video stream**: the video stream Core reads. Attached pictures
  (cover art) are skipped. Of the remaining video streams, Core picks the
  lowest-index stream flagged as default. If none is flagged, it picks the
  lowest-index stream. Other video streams are ignored, even if they would be
  supported. If there is no such stream, the input is unsupported.
- **Supported input**: a local file in an MP4 or MOV container. The selected
  video stream must be H.264 or HEVC, 8-bit or 10-bit SDR, with BT.709, BT.601,
  or unsignaled color primaries.
- **Unsupported input**: everything else. This includes other containers and
  codecs, streams signaled as HDR (PQ or HLG transfer), and streams with
  BT.2020 or other wide-gamut primaries. Unsupported inputs are rejected with
  an error (F2), even if FFmpeg could decode them.
- **Limited-candidate input**: a reference video that is not expected to yield
  the default N candidates. A video qualifies automatically if its duration is
  shorter than N × the default minimum separation (Q4). Any other video
  qualifies only if its reference-set entry says so, with the reason, before
  its first acceptance run.
- **Sample point**: a timestamp at which Core extracts and scores a frame.
- **Candidate**: one selected frame, with its timestamp, its per-metric scores,
  its overall score, and its rank.
- **Image buffer**: an in-memory image in the v0.1 pixel format, RGBA8.
  - Four 8-bit channels per pixel, in R, G, B, A byte order.
  - Alpha is 255 (fully opaque) for every pixel.
  - The first pixel is the top-left pixel. Rows are stored top-to-bottom and
    pixels left-to-right.
  - Rows are tightly packed. The row stride is `width * 4` bytes.
  - Width and height are the final display dimensions, after orientation and
    pixel-aspect-ratio correction.
  - No gamut conversion is applied. Values keep the source's primaries and
    transfer function.

  I use the term *image buffer* to avoid confusion with platform-specific
  types such as Apple's `CVPixelBuffer`. The reasons for choosing RGBA8 are in
  `docs/adr/001-image-representation.md`.
- **Configuration**: every setting that influences the result. This covers
  candidate count, sampling, scoring weights, thresholds, output size, and
  output format.
- **Reference set**: the videos used for acceptance and benchmarking. See
  [Reference set](#reference-set) for its minimum contents.
- **Reference machine**: the machine on which performance targets are checked.
  Its model, CPU, and memory are recorded with the benchmark results.

## Assumptions

- macOS and Linux builds use the same FFmpeg version.
- Decoding is done in software. Hardware decoders give different pixels on
  different machines.
- FFmpeg's default handling of MOV and MP4 edit lists is left on.

## Verification gates

The **Gate** column says when a requirement must be demonstrated.

- **0.1**: the requirement must pass before the 0.1 release, at the end of
  roadmap Phase 5.
- **Phase 6** and **Phase 7**: the design must respect the requirement from the
  start, but it is formally verified later, during cross-platform hardening
  (Phase 6) or performance engineering (Phase 7). It does not block the 0.1
  release.

## Functional requirements

| ID | Requirement | Acceptance criterion | Gate |
|----|-------------|----------------------|------|
| F1 | Core must decode supported inputs. | Every file in the reference set decodes to completion with no crash or hang. By Phase 6, the same runs are clean under AddressSanitizer and UndefinedBehaviorSanitizer, with no leaks. | 0.1; sanitizers Phase 6 |
| F2 | Core must fail cleanly on everything else. | Unsupported, truncated, corrupt, empty, and missing files return an error. Core must not crash, hang, or leak. Every file in the negative set returns the expected error kind (A2). | 0.1 |
| F3 | Core must extract complete frames. | An extracted frame has the stream's display dimensions (after F4). It has no missing or corrupt regions. After a seek, it does not depend on reference frames that were not decoded. | 0.1 |
| F4 | Core must orient frames correctly. | Rotation metadata and non-square pixel aspect ratio are applied. The frame's dimensions and orientation match what `ffplay` displays for the same file. | 0.1 |
| F5 | Core must extract the frame at the requested time. | A request for time *t* returns the frame a player would display at *t*. This holds for constant and variable frame rates. The rules are in [Frame timing](#frame-timing). The reported timestamp is the returned frame's own timestamp, not the requested time. | 0.1 |
| F6 | Core must convert color correctly. | Frames are delivered as image buffers. Conversion uses the stream's signaled color matrix and range. If they are unsignaled, Core assumes BT.709 limited range at 720 lines or more, and BT.601 limited range below that. 10-bit input is reduced to 8-bit by the same deterministic conversion on every platform. | 0.1 |
| F7 | Core must sample the video without decoding all of it. | The number of sample points is bounded by configuration, not by duration. The frames decoded per sample point are bounded by the stream's keyframe interval, plus the neighboring frames needed for the motion score. If a video has fewer frames than requested sample points, Core samples the frames that exist and does not report an error. | 0.1 |
| F8 | Core must score each sampled frame. | Each frame gets a sharpness score, a brightness score, a motion score, and an overall score. The overall score combines the three with configurable weights. Every score is normalized to a documented range. | 0.1 |
| F9 | Core must rank and select candidates. | Core returns up to N candidates (default 5, configurable), ordered by overall score. Equal scores are ordered by earlier timestamp. If fewer than N frames pass Q3 and Q4, Core returns fewer. It must not pad the result with frames that fail. Zero candidates is a successful result, not an error. | 0.1 |
| F10 | Core must write thumbnail images. | Each candidate can be written as JPEG or PNG at a requested size, preserving aspect ratio. Written files carry no color profile. | 0.1 |
| F11 | Core must report results as data. | Timestamps, scores, ranks, and output paths are available through the API. They do not depend on any image files being written. | 0.1 |
| F12 | Core must report run statistics. | Each run reports the number of frames sampled, rejected (by reason), and ranked. It also reports the time spent in decoding, analysis, and selection. | 0.1 |

### Frame timing

These rules define F5.

- Time is in seconds, relative to the first displayed frame of the selected
  video stream. That frame is time 0.
- For 0 < *t* < the end of the stream, Core returns the frame with the greatest
  presentation timestamp ≤ *t*.
- A request at or before time 0 returns the first frame.
- A request at or beyond the end of the stream returns the last complete frame.
- Reported timestamps use the same zero-based timeline.
- The API also reports the stream's start offset in the container. A consumer
  can add it to a reported timestamp to get the time a player shows.

## Quality requirements

| ID | Requirement | Acceptance criterion | Gate |
|----|-------------|----------------------|------|
| Q1 | Results must be deterministic. | On one platform, the same input, configuration, build, and FFmpeg version produce byte-identical candidates, scores, and image files. This holds across repeated runs and across thread counts. | 0.1 |
| Q2 | Results must be equivalent across platforms. | With the same FFmpeg version, macOS and Linux select the same frame timestamps. Normalized scores differ by at most 1e-3. Rank order may differ only between candidates whose scores are within that tolerance. Decoded pixels match at PSNR ≥ 40 dB. | Phase 6 |
| Q3 | Core must reject unusable frames. | A frame that fails the black, white, or blur threshold is never returned as a candidate. The thresholds are configurable and their defaults are documented. This is verified with synthetic frames (all black, all white, blurred) and with examples from the reference set. More detectors, such as fades and transitions, may be added later without changing the API. | 0.1 |
| Q4 | Selections must be temporally distinct. | Any two final candidates are separated by at least a configurable minimum time. Visual deduplication is not required for 0.1. It may be added later behind the same configuration. | 0.1 |
| Q5 | Performance must be measurable. | A repeatable command captures wall-clock time and peak memory for a run on the reference set. | 0.1 |
| Q6 | Core must be portable. | Core is C++20 with no platform-specific dependencies. For 0.1 it builds with AppleClang on macOS and with at least one compiler on Linux. By Phase 6 it builds with Clang, GCC, and AppleClang. | 0.1; full matrix Phase 6 |

## Interface constraints

These constrain the public API. They exist so that the CLI and the macOS app
can use Core without depending on FFmpeg or on platform-specific types.

| ID | Requirement | Acceptance criterion | Gate |
|----|-------------|----------------------|------|
| A1 | FFmpeg must stay behind the API. | No FFmpeg type or header appears in Core's public headers. | 0.1 |
| A2 | Errors must be distinguishable. | The API reports at least these as distinct error kinds: file not found, unsupported format, corrupt or truncated media, and output write failure. | 0.1 |
| A3 | Frames must be available in memory. | A candidate's image can be obtained as an image buffer at full display resolution, without writing a file. The API exposes width, height, row stride, and pixel format. The caller owns the image object, and the image object owns its bytes. The buffer stays valid for the lifetime of the image object, even after the engine that produced it is destroyed. The public API does not expose or depend on platform-specific pixel-buffer types. | 0.1 |

## Reference set

The reference set must contain at least 6 videos, and should not need more
than 10. Together they must cover:

- Content: action sports, talking head, tutorial, landscape, low light, and
  high motion.
- Technical cases: H.264 and HEVC, MP4 and MOV, 10-bit HEVC, 4K, rotated
  (portrait phone footage), non-square pixels, variable frame rate, a clip
  under 5 seconds, and a video of 10 minutes or more.

One video may cover several of these.

Each entry records duration, resolution, frame rate, codec, content type, and
known edge cases. It also records whether the video is an ordinary or a
limited-candidate input, with the reason.

A separate **negative set** exercises F2. It must contain:

- a missing path
- an empty file
- a truncated file
- a corrupt file
- a file with an unsupported container or codec
- an HDR file
- a wide-gamut (BT.2020) SDR file
- an audio-only file

## Performance targets

These targets are provisional. I will confirm or revise them once the first
benchmarks exist. They are measured on the reference machine with a Release
build, and verified in Phase 7.

| ID | Target |
|----|--------|
| P1 | A 10-minute 1080p H.264 file should complete in under 10 seconds. |
| P2 | A 5-minute 4K HEVC file should complete in under 10 seconds. |
| P3 | Peak memory should stay under 500 MB for 1080p and under 1.5 GB for 4K. |
| P4 | Peak memory should not grow with video duration. |
| P5 | For a fixed number of sample points, processing time should not grow in proportion to duration. This follows from F7. |

## Out of scope for version 0.1

- Containers and codecs beyond MP4/MOV with H.264/HEVC. For example: MKV,
  WebM, VP9, AV1, and ProRes.
- HDR input and tone mapping. HDR streams are rejected as unsupported.
- Wide-gamut (BT.2020) SDR input, and any gamut conversion.
- Resized in-memory images. Resizing applies only to written files (F10).
- Hardware-accelerated decoding. It would break Q2.
- Still images, audio-only files, network streams, and DRM-protected content.
- Semantic or AI-based ranking, face and text detection, and explanations of
  choices.
- Cropping, overlays, text, or any other editing of the selected frame.
- Cancellation, progress reporting, and thread-safety guarantees for the API.
  These will be defined with the Swift/C++ boundary for 0.2.
- User interface, CLI argument design, and the macOS app.
- Windows support.

## Open questions

- The reference set and the negative set are not assembled yet.
- The reference machine is not chosen yet.
- The default rejection thresholds for Q3 are not set.
- The default minimum separation for Q4 is not set.
- The score ranges for F8 are not documented yet.
- P5 has no numeric bound. I will set one after the first benchmarks.
