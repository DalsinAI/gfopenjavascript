/* Native AmigaOS 68040 task lifecycle qualification.
 * JavascriptCore remains shared while client stops ONE busy JSTask.
 * A separate JSTask must remain healthy; stopped task must recover.
 * MIT, Copyright (c) 2026 AmiCradle Team.
 */
#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/javascript.h>
#include <libraries/javascript.h>
#include <stdio.h>
struct Library *JavaScriptBase;
struct TaskJob {
    volatile LONG started;
    volatile LONG finished;
    volatile LONG rc;
};
static LONG endless(JSTask task,APTR opaque)
{
    struct TaskJob *j=(struct TaskJob*)opaque;
    j->started=1;
    j->rc=JS_Evaluate(task,
        (CONST_STRPTR)"for (;;) { Math.sqrt(16); }",
        -1,(CONST_STRPTR)"javascript-v3-endless.js",NULL);
    j->finished=1;
    return j->rc;
}
static LONG evaluate42(JSTask task,LONG *answer)
{
    JSValue value=NULL;
    LONG rc=JS_Evaluate(task,(CONST_STRPTR)"6*7",-1,
                        (CONST_STRPTR)"javascript-v3-recovery.js",&value);
    if(rc==JSERR_OK&&value)rc=JS_ToLong(task,value,answer);
    if(value)JS_Free(task,value);
    return rc;
}
int main(void)
{
    JSTask busy=NULL,other=NULL;
    struct TaskJob job={0,0,-1};
    LONG pre=-1,running=-1,stop=-1,idle=-1,recovery=-1,otherRC=-1;
    LONG recoveryValue=-1,otherValue=-1;
    int tick,ok;
    JavaScriptBase=OpenLibrary((CONST_STRPTR)JAVASCRIPTNAME,
                               JAVASCRIPT_TASK_CONTROL_VERSION);
    if(!JavaScriptBase){
        puts("JAVASCRIPT_V3_CONTROL status=SKIP_NO_V3_LIBRARY");
        return 77;
    }
    busy=JS_NewTask(NULL);
    other=JS_NewTask(NULL);
    if(!busy||!other){puts("JAVASCRIPT_V3_CONTROL status=NEW_TASK_FAILED");return 75;}
    pre=JS_Running(busy);
    if (pre < 0) {
        puts("JAVASCRIPT_V3_CONTROL status=SKIP_OLD_RUNNING_ENGINE");
        JS_EndTask(other);
        JS_EndTask(busy);
        CloseLibrary(JavaScriptBase);
        return 77;
    }
    if(JS_Post(busy,endless,&job,0)!=JSERR_OK)return 75;
    for(tick=0;tick<350 && JS_Running(busy)==JSTASK_IDLE;tick++)
        Delay(1);
    running=JS_Running(busy);
    stop=JS_Stop(busy);
    for(tick=0;tick<350 && !job.finished;tick++)
        Delay(1);
    idle=JS_Running(busy);
    if(job.finished) {
        recovery=evaluate42(busy,&recoveryValue);
        otherRC=evaluate42(other,&otherValue);
    }
    ok=pre==JSTASK_IDLE && running==JSTASK_RUNNING &&
       stop==JSERR_OK && job.finished &&
       job.rc==JSERR_EXCEPTION &&
       idle==JSTASK_IDLE && recovery==JSERR_OK &&
       recoveryValue==42 && otherRC==JSERR_OK && otherValue==42;
    /* If Stop did not interrupt the loop, avoid calling blocking EndTask:
     * the isolated test instance can be reset without harming shared clients.
     * Print a clear failure to the runner instead of pretending cleanup. */
    printf("JAVASCRIPT_V3_CONTROL pre=%ld running=%ld stop=%ld finished=%ld interrupted=%ld idle=%ld recovery=%ld/%ld other=%ld/%ld status=%s\n",
       (long)pre,(long)running,(long)stop,(long)job.finished,
       (long)job.rc,(long)idle,(long)recovery,(long)recoveryValue,
       (long)otherRC,(long)otherValue,ok?"PASS":"FAIL");
    if(job.finished) {
        JS_EndTask(other);
        JS_EndTask(busy);
    }
    CloseLibrary(JavaScriptBase);
    return ok?0:2;
}
