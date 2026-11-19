/* Native AmigaOS 3.x javascript.library v2 release gate.
 * Copyright (c) 2026 AmiCradle Team. MIT, see LICENSE.
 * Run with a MATCHED v2 JavaScriptEngine installed.
 * A successful compile is NOT a successful native test.
 */
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/javascript.h>
#include <libraries/javascript.h>
#include <stddef.h>
#include <stdio.h>

struct Library *JavaScriptBase;

struct TestVector { LONG x; };

static const struct JSHostSlot vector_slots[] = {
    {"x", JSS_FIELD, JSF_LONG, NULL, NULL,
        (LONG)offsetof(struct TestVector, x), 0},
    {NULL, 0, 0, NULL, NULL, 0, 0}
};
static const struct JSHostClass vector_class = {
    "Vector3DProbe", NULL, vector_slots, NULL, NULL
};

static int evaluate_true(JSTask task, const char *src)
{
    JSValue r = NULL;
    LONG code = JS_Evaluate(task, src, -1, "class_prototype_probe.js", &r);
    int good = (code == JSERR_OK && r && JS_ToBool(task, r));
    if (r) JS_Free(task, r);
    return good;
}

int main(void)
{
    JSTask task = NULL, foreign = NULL;
    JSClass cls = NULL;
    JSValue global = NULL, proto = NULL, wrapped = NULL, again = NULL;
    struct TestVector v = {42};
    int good = 0;

    JavaScriptBase = OpenLibrary(JAVASCRIPTNAME,
                                 JAVASCRIPT_CLASS_PROTOTYPE_VERSION);
    if (!JavaScriptBase) {
        puts("FAIL: javascript.library v2 not installed");
        return 20;
    }
    task = JS_NewTask(NULL);
    if (!task) {
        puts("FAIL: JavaScriptEngine task unavailable");
        goto done;
    }
    cls = JS_RegisterClass(task, &vector_class);
    if (!cls) {
        puts("FAIL: native class registration");
        goto done;
    }
    proto = JS_ClassPrototype(task, cls);
    if (!proto) {
        puts("FAIL: matching v2 engine/prototype callback missing");
        goto done;
    }
    global = JS_Global(task);
    wrapped = JS_Wrap(task, cls, &v);
    again = JS_Wrap(task, cls, &v);
    if (!global || !wrapped || !again) {
        puts("FAIL: global or wrapping");
        goto done;
    }
    if (JS_SetProp(task, global, "vecProto", proto) != JSERR_OK ||
        JS_SetProp(task, global, "vec", wrapped) != JSERR_OK ||
        JS_SetProp(task, global, "vec2", again) != JSERR_OK) {
        puts("FAIL: publish native objects");
        goto done;
    }
    if (!evaluate_true(task,
        "Object.getPrototypeOf(vec)===vecProto && vec.x===42 && vec===vec2")) {
        puts("FAIL: native prototype identity, field, or wrapping");
        goto done;
    }
    foreign = JS_NewTask(NULL);
    if (!foreign) {
        puts("FAIL: second task");
        goto done;
    }
    {
        JSValue invalid = JS_ClassPrototype(foreign, cls);
        if (invalid) {
            JS_Free(foreign, invalid);
            puts("FAIL: foreign task accepted native class handle");
            goto done;
        }
    }
    good = 1;
    puts("PASS: v2 prototype, native fields, wrapping and task isolation");
done:
    if (foreign) JS_EndTask(foreign);
    if (again) JS_Free(task, again);
    if (wrapped) JS_Free(task, wrapped);
    if (proto) JS_Free(task, proto);
    if (global) JS_Free(task, global);
    if (task) JS_EndTask(task);
    CloseLibrary(JavaScriptBase);
    return good ? 0 : 10;
}
