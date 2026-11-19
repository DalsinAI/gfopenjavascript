# JavaScript task control v3 — experimental / NOT release-qualified

19 November 2026. This work is intentionally on an experimental branch,
not the main release branch. It does NOT implement reliable cancellation yet.

## Requested semantics

- A program can open one shared javascript.library and create dozens of
  owned, isolated JSTasks, each a separate JSC VM/context and execution thread.
- JS_Running(JSTask) returns IDLE 0 / RUNNING 1 / STOPPING 2 / invalid -1.
  Must never wait behind a stuck VM.
- JS_Stop(JSTask) requests current execution to unwind
  without destroying JSC context, other tasks, or shared engine.
  Stop when idle must not cancel a future job.
- JS_EndTask(JSTask) remains the task destructor. Caller coordinates its
  own handles and callback lifetimes, not a process-wide kill.
- Legacy v1/v2 clients see unchanged vectors; v3 appended
  -216/-222 and corresponding size-gated engine table callbacks.

## Verified

- ABI SFD/generator check: 33 vectors -30 to -222, append-only.
- Standalone 68040 javascript.library v3 cross-linked: 3756 bytes,
  SHA-256 6066f7eb939c694c4e0c1cb9847ef2caf08e342225eaae0534a42972704e0000.
- Real pinned JSC/WebKit 68040 engine with JSC VMTrap adapter cross-built
  as actual AmigaOS loadseg: SHA-256
  f9c4d0b71821053be50d653eebccafdb40775e43f207cde5e47cd43d3486a7c1.
- Native AC090 AmigaOS 3.2.3 guest can create v3 JSTasks. From a separate
  AmigaOS task, JS_Running reported busy (1) and JS_Stop returned
  JSERR_OK (0) without waiting.

## FAILED NATIVE SAFETY GATE — BLOCKS MERGE

The test posts a tight JavaScript loop to task A, requests JS_Stop(task A),
requires an interrupt result, then evaluates 6*7=42 on both task A and
unaffected task B and ends them. Neither the public JSC execution-time-limit
callback nor direct CONCURRENT_SAFE VM::notifyNeedTermination delivered an
interrupt on the current 68040 JSC VM for this test. Both experiments:
finished=0, idle=2, JS task A still running after timeout, RC2 FAIL.
The test deliberately does not block in JS_EndTask after a timeout.

Artifacts:
- tests/evidence/javascript-v3-watchdog-native-failed-20261119.log
- tests/evidence/javascript-v3-concurrent-trap-native-failed-20261119.log
- tests/javascript_v3_task_control_gate.c

A possible root cause is this target's LLInt/68k interpreter or JIT dispatch
not checking concurrent VMTraps in an optimized tight loop. Investigate
trap polling in generated 68k code. This is a hypothesis, not yet verified.

Do not ship, tag or replace the known-good v2 engine, and do not switch
Oolite's original watchdog until a native infinite-loop, cancel-and-reuse,
cross-task isolation and concurrent close gate passes.
The test guest's known-good v2 pair was restored after failed tests.

There is no JS_EndAllTasks in this experimental patch. The owner must
explicitly track every task; automatic cleanup on library close requires
a separately proven ownership registry.
