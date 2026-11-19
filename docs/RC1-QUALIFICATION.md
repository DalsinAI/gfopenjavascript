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
