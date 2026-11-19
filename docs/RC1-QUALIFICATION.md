# javascript.library 2.0.0-rc1 qualification

## Status

**Candidate source extraction only. Not an AmigaOS-qualified release.**

The standalone repository owns the RC package. The canonical donor is
DalsinAI/gfopenbrowser at c731d89f6f018ffb2ea4091cd00595b5fbba9e9a,
with the matching SDK contract DalsinAI/gfopensdk at
9433cc8d2b1b363b095f0af14e3c114f2aebb22a. Changes after extraction
must be explicitly synchronized and tested as a matched set.

## Executable release gates

- [ ] Build `javascript.library` and the standalone `JS` shell with GCC 16.2;
      record toolchain identity, source SHA, binary SHA-256 and link/map evidence.
- [ ] Build a matching `JavaScriptEngine` from pinned JavaScriptCore/WebKit
      source and patches; record the exact JSC identity and binary SHA-256.
- [ ] Verify every v1 vector (-30 to -204) against the published ABI;
      verify `JS_ClassPrototype` at -210, library v2 and old-engine fallback.
- [ ] Run a named AmigaOS 3.2.3 AC090 68040/FPU guest with the **matching
      library/engine pair**; confirm resident version and successful engine startup.
- [ ] Run native prototype-before-wrapper, inheritance, identity and
      foreign-task class rejection; verify `JS_Free`, GC and finalizers.
- [ ] Run old-v1-client and v2-client interoperability, standalone JS CLI,
      errors/recovery, cache enabled/disabled/version mismatch, cross-task calls.
- [ ] Run GFOpenBrowser consumer smoke test and Oolite Vector3D
      `new Vector3D(...)` and `instanceof` with original OXP scripts.
- [ ] Publish reproducible build instructions, SDK package, license notices,
      provenance and hashes; publish GitHub **prerelease** only after all pass.

## Build entry points

`src/jslib` is the authoritative extracted runtime source in this repo.
`scripts/build-javascript-library.sh` builds the loader and `JS` with
`TOOLCHAIN_DIR` or `STOVE`; `toolchain/m68k-soft-m68040.specs` is required
by JS. `scripts/build-javascript-engine.sh` requires `WEBKIT_BUILD` and
`OS32_GCC16`/a compatible stove containing the pinned JSC build. It does
**not** build WebKit itself. `tests/jslib/run.sh` requires a separate pinned
m68k-linux JSC test build and QEMU; static CI is **not native qualification**.

Do not deploy a v2 loader paired with an unknown v1 engine and infer that
`JS_ClassPrototype` is available; a version check plus non-NULL callback
result is required. The engine and loader must be deployed and tested together.

No claim is made here that a guest run or release binary has passed.


## CI evidence (2026-11-19)

Standalone source extraction commit: `684f9ed19a1572fffa2f57ea81685d45a74b13d2`.
Executable mode correction: `807c354270cb786c6673627069fabb70a6c9f2f1`.

- Static ABI run `44477193553`: successful.
- Cross-build run `44477193592`: successful.
- Library: 3,696 bytes, SHA-256 `d8920e912507b5b8e25a30e6b04599362fc915273fccb371915304d13e0e4d80`.
- JS: 6,892 bytes, SHA-256 `9bb362d8bc147b124cfddc9197e632ac14286eaad0f95f354d3d1aa7dcd91ffe`.
- CI artifact: `gfopenjavascript-v2-68040-loader-unqualified`.

The CI job tests the 68k loader, JS shell, engine dispatcher compilation
and ABI metadata. It **does not** link a JavaScriptCore-backed
JavaScriptEngine or run the library in a guest. Keep RC1 unreleased.

## Real JavaScriptEngine v2 and Oolite native guest evidence (2026-11-19)

A reproducible JavaScriptCore engine cross-build now exists:
scripts/build-javascript-engine-webcore-ninja.py consumes a pinned existing
WebCore Ninja/JavaScriptCore build and canonical standalone GFOpenJavaScript
source. It compiles the JSC bridge C++ files, AmigaOS dispatcher and jsimage
host, then links real JSC/WTF/PAL/ICU without WebCore or OBCore.

A second independent rebuild produced **the identical**
37,918,816-byte AmigaOS loadseg JavaScriptEngine, SHA-256:
d35f5a3846786549dc8b8a453eb813f71bd7bf32bf5564d89cfd3b4dcc69c923.

Build provenance:
- GFOpenJavaScript source at c8085afbfcbf15db49a904dac50abf44bfcce370
- JSC archive SHA-256 9f61f1c2ddb03e90dd5b8004ebe034957003e3ea9e6f23ef00b4c329b22e57bf
- JSC build fingerprint 42092cdfd6b93cc3ecca29538856b0ab138517ff8d75719e568c0f2520dd54ff
- GCC 16.2, target AmigaOS 68040/FPU

**Native guest qualification:** the standalone CI loader (SHA-256
d8920e912507b5b8e25a30e6b04599362fc915273fccb371915304d13e0e4d80)
and new v2 engine were deployed as a pair in AmiCradle Team 1 - Oolite
JSAPI RC1, AmigaOS 3.2.3 AC090 68040.

Guest transcript shows:
- Oolite Vector3D strict native prototype identity: v2=1, install=0,
  eval=0, answer=42, post=0, PASS, RC 0
- Full Vector3D constructor/arithmetic/array regression: PASS, RC 0
- Original Oolite Ship.prototype, instanceof and unconstructable-call
  bootstrap: PASS, RC 0, following a fix to the Oolite adapter
- Class prototype, native field, wrapping and task-isolation probe:
  PASS, RC 0
- Standalone JS CLI returned RC 0 and printed 67 from an EVAL quoting
  invocation; an explicit script file (print(6 * 7)) subsequently printed
  exactly 42 and returned RC 0. Output is preserved in the Oolite evidence
  file oolite-gfopenjavascript-standalone-v2-cli-42-guest-20261119.log.

Raw guest transcript:
DalsinAI/amigaoolite/tests/evidence/oolite-gfopenjavascript-standalone-v2-pair-guest-20261119.log.

These qualify the specific engine/loader pair in the native guest. The
browser consumer, cache concurrency and the original complete Oolite game
link are NOT fully qualified. The 101 unresolved SpiderMonkey JS symbols
in the original Oolite objects remain a separate source migration gate.

Rebuild using the pinned JSC archive:
    python3 scripts/build-javascript-engine-webcore-ninja.py \
      --webkit-build /path/to/pinned/build-Amiga \
      --toolchain-bin /path/to/GCC16/prefix/bin \
      --ndk-include /path/to/GCC16/prefix/m68k-amigaos/ndk-include.vendor \
      --jsc-sha256 9f61f1c2ddb03e90dd5b8004ebe034957003e3ea9e6f23ef00b4c329b22e57bf \
      --out /path/to/isolated/output

The engine-build-manifest.json records input/source and output hashes.
Do NOT publish RC1 until the remaining release gates above are completed.
