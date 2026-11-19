#!/usr/bin/env python3
"""Build standalone JavaScriptEngine from an existing pinned WebCore Ninja JSC build.

Reuse JSC compiler/link settings without vendoring JSC or linking WebCore.
This is a reproducible cross-build entry point, NOT guest qualification.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
LIB = ROOT / "src/jslib"
JS_CPP = ("jsl_core", "jsl_cache")
JS_C = ("jsl_os_exec", "jsl_engine", "jsimage_host")

def sha(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024*1024), b""):
            h.update(block)
    return h.hexdigest()

def run(argv, cwd=None):
    p = subprocess.run(argv, cwd=cwd, text=True, capture_output=True)
    if p.returncode:
        raise RuntimeError(f"Command failed ({p.returncode}): {argv[0]}\n{p.stderr[-5000:]}")
    return p.stdout

def clone_compile(template, compiler, name, out, ndk):
    argv = shlex.split(template)
    # The browser's ob-ccache wrapper takes a compiler as its first argument.
    if Path(argv[0]).name == "ob-ccache":
        argv.pop(0)
    argv[0] = str(compiler)
    argv = [w for w in argv if w not in ("-MD", "-Winvalid-pch")]
    for flag in ("-MT", "-MF", "-include"):
        if flag in argv:
            i = argv.index(flag)
            del argv[i:i+2]
    i = argv.index("-o")
    argv[i+1] = str(out / (name + ".o"))
    i = argv.index("-c")
    argv[i+1] = str(LIB / (name + ".cpp"))
    argv[i:i] = ["-I" + str(LIB), "-I" + str(LIB/"sdk/Include_H"), "-I" + str(ndk)]
    return argv

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--webkit-build", type=Path, required=True)
    p.add_argument("--toolchain-bin", type=Path, required=True)
    p.add_argument("--ndk-include", type=Path, required=True)
    p.add_argument("--out", type=Path, required=True)
    p.add_argument("--jsc-sha256", help="Expected pinned JSC archive checksum")
    args = p.parse_args()
    b, out = args.webkit_build.resolve(), args.out.resolve()
    cross = args.toolchain_bin.resolve()
    ndk = args.ndk_include.resolve()
    archive = b/"lib/libJavaScriptCore.a"
    if not archive.is_file() or not ndk.is_dir():
        p.error("Missing pinned JSC archive or target NDK")
    if args.jsc_sha256 and sha(archive) != args.jsc_sha256.lower():
        p.error("JSC archive SHA-256 differs from requested pinned build")
    out.mkdir(parents=True, exist_ok=True)
    compile_lines = run(["ninja", "-C", str(b), "-t", "commands",
                         "lib/libJavaScriptCore.a"]).splitlines()
    candidate = next((line for line in compile_lines
                      if "UnifiedSource-root-1.cpp" in line and " -c " in line),
                     None)
    if not candidate:
        p.error("Cannot identify pinned JavaScriptCore C++ compiler settings")
    for name in JS_CPP:
        run(clone_compile(candidate, cross/"m68k-amigaos-g++",
                          name, out, ndk), cwd=b)
        print("COMPILED", name, flush=True)
    for name in JS_C:
        argv = [str(cross/"m68k-amigaos-gcc"),
                "-m68040", "-m68881", "-mcrt=nix20", "-O2",
                "-Wall", "-Wextra", "-Wno-unused-parameter", "-fno-common",
                "-I"+str(LIB), "-I"+str(LIB/"sdk/Include_H"), "-I"+str(ndk),
                "-c", str(LIB/(name+".c")), "-o", str(out/(name+".o"))]
        run(argv, cwd=b)
        print("COMPILED", name, flush=True)
    links = run(["ninja", "-C", str(b), "-t", "commands",
                 "OpenBrowser.engine"]).splitlines()
    if not links:
        p.error("No pinned WebCore linker command for JavaScriptCore libraries")
    original = shlex.split(links[-1])
    if original[:2] != [":", "&&"] or original[-2:] != ["&&", ":"]:
        p.error("Unexpected Ninja linker command format")
    parts = original[2:-2]
    objects = [str(out/(n+".o")) for n in JS_CPP+JS_C]
    parts[0] = str(cross/"m68k-amigaos-g++")
    linked, inserted = [], False
    for item in parts:
        if item.endswith(".obj"):
            if not inserted:
                linked += objects
                inserted = True
            continue
        if item in ("lib/libOBCore.a", "lib/libWebCore.a"):
            continue
        if item.startswith("-Wl,-Map="):
            item = "-Wl,-Map=" + str(out/"JavaScriptEngine.map")
        linked.append(item)
    if not inserted or "lib/libJavaScriptCore.a" not in linked:
        p.error("Link did not identify JSC archive or browser objects")
    i = linked.index("-o")
    linked[i+1] = str(out/"JavaScriptEngine")
    run(linked, cwd=b)
    run([str(cross/"m68k-amigaos-strip"), "-o",
         str(out/"JavaScriptEngine.stripped"), str(out/"JavaScriptEngine")])
    filetype = run([str(cross/"m68k-amigaos-objdump"), "-f",
                    str(out/"JavaScriptEngine.stripped")])
    if "file format amiga" not in filetype:
        p.error("Cross-link output is not an AmigaOS loadseg binary")
    manifest = {
        "qualification": "cross-build only; native guest tests still required",
        "source": str(LIB),
        "webkit_build": str(b),
        "toolchain_bin": str(cross),
        "jsc_archive_sha256": sha(archive),
        "jsl_core_sha256": sha(LIB/"jsl_core.cpp"),
        "jsl_cache_sha256": sha(LIB/"jsl_cache.cpp"),
        "engine_sha256": sha(out/"JavaScriptEngine.stripped"),
        "engine_size_bytes": (out/"JavaScriptEngine.stripped").stat().st_size,
        "format": "AmigaOS loadseg",
    }
    (out/"engine-build-manifest.json").write_text(json.dumps(manifest, indent=2)+"\n")
    print(json.dumps(manifest, indent=2))
    return 0

if __name__ == "__main__":
    try:
        sys.exit(main())
    except (RuntimeError, OSError, ValueError) as exc:
        print("BUILD_FAILED:", str(exc), file=sys.stderr)
        sys.exit(2)
