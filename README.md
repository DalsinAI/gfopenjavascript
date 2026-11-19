# GFOpenJavaScript

GFOS's standalone JavaScript runtime for AmigaOS 3.2.3 on 68040/FPU: the `javascript.library` public ABI, its matching `JavaScriptEngine` (JavaScriptCore), and the `JS` shell client.

**Release target:** `2.0.0-rc1` — **qualification in progress; do not publish binaries as verified yet**.

## Canonical code and dependency handoff

This repository is the intended independent release home, not a forked, competing interpreter. Until the source migration is completed, the **verified canonical source** remains [DalsinAI/gfopenbrowser/src/jslib](https://github.com/DalsinAI/gfopenbrowser/tree/main/src/jslib) at its append-only ABI v2 merge, [`c731d89f6f018ffb2ea4091cd00595b5fbba9e9a`](https://github.com/DalsinAI/gfopenbrowser/commit/c731d89f6f018ffb2ea4091cd00595b5fbba9e9a). The matching SDK release contract is [DalsinAI/gfopensdk](https://github.com/DalsinAI/gfopensdk) at `9433cc8d2b1b363b095f0af14e3c114f2aebb22a`.

Do not independently modify duplicated copies of `jsl_core.cpp`, `javascript_lib.c`, the SFD or headers. The extraction must preserve history/attribution, consistent ABI vector ordering and the existing GFOpenBrowser build until both projects can build from the standalone source.

| Component | Canonical location during migration |
| --- | --- |
| Library dispatcher and FD/SFD | `gfopenbrowser/src/jslib/` |
| JavaScriptCore engine bridge and bytecode cache | `gfopenbrowser/src/jslib/jsl_core.cpp`, `jsl_cache.cpp`, `jsl_engine.*` |
| Public `javascript.library` SDK | `gfopensdk/Include`, `SFD`, `FD` |
| JSC/WebKit toolchain and patches | `gfopenbrowser/webkit`, build integration |
| Browser web UI and WebCore | Remain in `gfopenbrowser` |
| Standalone versioned release artifacts and qualification | **This repository**, once certified |

## Proposed 2.0.0-rc1 acceptance gates

1. Build a matching **v2 library and v2 JavaScriptEngine** with GCC 16.2 and the pinned JSC build, recording SHA-256 of each.
2. Verify the 30 v1 ABI vectors remain unchanged; new `JS_ClassPrototype` is appended at offset **-210**. Reject engine/library mismatches without unsafe calls.
3. Boot **named isolated AmiCradle/AC090 68040 AmigaOS guest**, test class-prototype identity, host class inheritance, GC ownership, old v1 callers, new v2 caller and source/bytecode caching.
4. Verify standalone `JS` shell client and GFOpenBrowser integration against the same pair; then test Oolite `Vector3D` `new` and `instanceof`.
5. Publish SDK material, source provenance, reproducible build instructions, license notices, test evidence, changelog, hashes and a GitHub **prerelease** only when all gates pass.

**Known state (2026-11-19):** The 3,672-byte GCC 16.2 v2 library loadseg cross-built and v2 SDK merged, but an installed, matching v2 engine and native `JS_ClassPrototype` guest qualification are not yet verified. This is not a release-ready claim.

See the [browser migration contract](https://github.com/DalsinAI/gfopenbrowser) for how its current local build transitions without a breaking source move.
