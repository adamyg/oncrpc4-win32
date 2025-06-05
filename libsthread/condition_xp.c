/*
 *  Simple win32 threads - conditions_xp
 */

typedef struct Condition {
    CRITICAL_SECTION lock;
    HANDLE wakeone;
    HANDLE broadcast;
    unsigned waiters;
} Condition_t;

typedef void (WINAPI *InitializeConditionVariable_t)(CONDITION_VARIABLE *);
typedef BOOL (WINAPI *SleepConditionVariableCS_t)(CONDITION_VARIABLE *, CRITICAL_SECTION *, DWORD);
typedef void (WINAPI *WakeConditionVariable_t)(CONDITION_VARIABLE *);
typedef void (WINAPI *WakeAllConditionVariable_t)(CONDITION_VARIABLE *);

//#define ENABLE_XPEMULATION
#if defined(ENABLE_XPEMULATION)
static void WINAPI XPInitializeConditionVariable(CONDITION_VARIABLE *cv);
static void WINAPI XPDestroyConditionVariable(CONDITION_VARIABLE *cv);
static BOOL WINAPI XPSleepConditionVariableCS(CONDITION_VARIABLE *cv, CRITICAL_SECTION *cs, DWORD ms);
static void WINAPI XPWakeConditionVariable(CONDITION_VARIABLE *cv);
static void WINAPI XPWakeAllConditionVariable(CONDITION_VARIABLE *cv);
#endif

static HMODULE hKernel32 = NULL;
static InitializeConditionVariable_t fnInitializeConditionVariable;
static SleepConditionVariableCS_t fnSleepConditionVariableCS;
static WakeConditionVariable_t fnWakeConditionVariable;
static WakeAllConditionVariable_t fnWakeAllConditionVariable;


static void
MyInitializeConditionVariable(CONDITION_VARIABLE *cv)
{
    if (NULL == fnInitializeConditionVariable) {
        hKernel32 = LoadLibraryA("Kernel32.dll");
        if (hKernel32) {
            fnInitializeConditionVariable =
                (InitializeConditionVariable_t) GetProcAddress(hKernel32, "InitializeConditionVariable");
            if (fnInitializeConditionVariable) {
                fnSleepConditionVariableCS =
                    (SleepConditionVariableCS_t) GetProcAddress(hKernel32, "SleepConditionVariableCS");
                fnWakeConditionVariable =
                    (WakeConditionVariable_t) GetProcAddress(hKernel32, "WakeConditionVariable");
                fnWakeAllConditionVariable =
                    (WakeAllConditionVariable_t) GetProcAddress(hKernel32, "WakeAllConditionVariable");
            }
        }

        if (NULL == fnInitializeConditionVariable || NULL == fnSleepConditionVariableCS ||
                NULL == fnWakeConditionVariable || NULL == fnWakeAllConditionVariable) {

#if defined(ENABLE_XPEMULATION)
            fnInitializeConditionVariable = XPInitializeConditionVariable;
            fnSleepConditionVariableCS = XPSleepConditionVariableCS;
            fnWakeConditionVariable = XPWakeConditionVariable;
            fnWakeAllConditionVariable = XPWakeAllConditionVariable;
#else
            MessageBox(NULL, "Unable to resolve ConditionVariable functions", "Error", MB_ICONERROR | MB_OK);
            exit(3);
#endif

        }
    }

    fnInitializeConditionVariable(cv);
}


static void
MyDeleteConditionVariable(CONDITION_VARIABLE *cv)
{
#if defined(ENABLE_XPEMULATION)
    if (fnInitializeConditionVariable == XPInitializeConditionVariable) {
        XPDestroyConditionVariable(cv);
    }
#endif //ENABLE_XPEMULATION
}


static BOOL
MySleepConditionVariableCS(CONDITION_VARIABLE *cv, CRITICAL_SECTION *cs, DWORD ms)
{
    return fnSleepConditionVariableCS(cv, cs, ms);
}


static void
MyWakeConditionVariable(CONDITION_VARIABLE *cv)
{
    fnWakeConditionVariable(cv);
}


static void
MyWakeAllConditionVariable(CONDITION_VARIABLE *cv)
{
    fnWakeAllConditionVariable(cv);
}


/////////////////////////////////////////////////////////
//  Local implementation

#if defined(ENABLE_XPEMULATION)

static void WINAPI
XPInitializeConditionVariable(CONDITION_VARIABLE *cv)
{
    Condition_t *cond;
    cond = (Condition_t *)calloc(1, sizeof(Condition_t));
    if (cond) {
        InitializeCriticalSectionAndSpinCount(&cond->lock, 20000);
        cond->broadcast = CreateEvent(NULL, TRUE, FALSE, NULL); // manual-reset
        cond->wakeone = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset
    }
    cv->Ptr = cond;
}


static void WINAPI
XPDestroyConditionVariable(CONDITION_VARIABLE *cv)
{
    Condition_t *cond;

    if (cv && NULL != (cond = cv->Ptr)) {
        XPWakeAllConditionVariable(cv);
        if (cond->broadcast) CloseHandle(cond->broadcast);
        if (cond->wakeone) CloseHandle(cond->wakeone);
        DeleteCriticalSection(&cond->lock);
        free((void *)cond);
        cv->Ptr = NULL;
    }
}


/*
 *  XPSleepConditionVariableCS ---
 *      Sleeps on the specified condition variable and releases the specified critical section as an atomic operation.
 */
static BOOL WINAPI
XPSleepConditionVariableCS(CONDITION_VARIABLE *cv, CRITICAL_SECTION *cs, DWORD ms)
{
    Condition_t *cond;
    BOOL ret = FALSE;

    LeaveCriticalSection(cs);

    if (cv && NULL != (cond = cv->Ptr)) {
        HANDLE events[2] = {
            cond->wakeone,
            cond->broadcast
            };
        DWORD wait_result;

        EnterCriticalSection(&cond->lock);
            ++cond->waiters;
        LeaveCriticalSection(&cond->lock);

        wait_result = WaitForMultipleObjects(_countof(events), events, FALSE, ms);

        EnterCriticalSection(&cond->lock);
            --cond->waiters;
            if (wait_result == (WAIT_OBJECT_0 + 1)) { // broadcast event
                if (0 == cond->waiters) { // no additional waiters
                     ResetEvent(cond->broadcast); // reset trigger
                }
            }
        LeaveCriticalSection(&cond->lock);

        ret = (wait_result != WAIT_TIMEOUT);
    }

    EnterCriticalSection(cs);

    return ret;
}


static void WINAPI
XPWakeConditionVariable(CONDITION_VARIABLE *cv)
{
    Condition_t *cond;

    if (cv && NULL != (cond = cv->Ptr)) {
        unsigned waiters;

        EnterCriticalSection(&cond->lock);
        waiters = cond->waiters;
        LeaveCriticalSection(&cond->lock);
        if (waiters) {
            SetEvent(cond->wakeone);
        }
    }
}


static void WINAPI
XPWakeAllConditionVariable(CONDITION_VARIABLE *cv)
{
    Condition_t *cond = cv->Ptr;

    if (cond) {
        unsigned waiters;

        EnterCriticalSection(&cond->lock);
        waiters = cond->waiters;
        LeaveCriticalSection(&cond->lock);

        if (waiters) {
            if (1 == waiters) {
                SetEvent(cond->wakeone);
            } else {
                SetEvent(cond->broadcast);
            }
        }
    }
}

#endif //ENABLE_XPEMULATION


/////////////////////////////////////////////////////////
//  Function replacement

#define InitializeConditionVariable(__cv)           MyInitializeConditionVariable (__cv)
#define DeleteConditionVariable(__cv)               MyDeleteConditionVariable (__cv)
#define SleepConditionVariableCS(__cv, __cs, __ms)  MySleepConditionVariableCS (__cv, __cs, __ms)
#define WakeConditionVariable(__cv)                 MyWakeConditionVariable (__cv)
#define WakeAllConditionVariable(__cv)              MyWakeAllConditionVariable (__cv)

//end

