# PetrelTT — initial code review

A first-pass checklist of concrete findings, ordered by severity. Each item points to a file:line so you can jump to it.

Tick items off as we discuss or fix them. Items marked **(?)** are questions where I want your confirmation before recommending a fix.

---

## A. Likely correctness bugs

- [x] **A1 — Missing braces in `SetState(AutoStopped)`.** `PetrelTT.cpp:177-183`. The `if (oldState != St::AutoRunning)` has no braces, so only the first line (`takeTopLevelItem(0)`) is conditional. The next two lines (`insertTopLevelItem` + `ExpandTestTree`) run *every* time `AutoStopped` is entered — including from `AutoRunning`, which is supposed to skip the rebuild. Wrap all three in `{ ... }`.

- [x] **A2 — `SLOT(slotMessageResult)` missing parentheses.** `PetrelTT.cpp:375`. The `SLOT` macro stringifies its arg, so this becomes `"1slotMessageResult"` and won't match the connected `slotMessageResult(int)`. The `disconnect` silently fails (look for a `QObject::disconnect: Cannot ...` warning in qDebug). Should be `SLOT(slotMessageResult(int))`. Switching to the new-style pointer-to-member `connect`/`disconnect` would have caught this at compile time — see B1.

- [x] **A3 — Dead disconnects in `tPetrelTT::CloseTestProcedure`.** `PetrelTT.cpp:367-378`. The function calls `Project.CloseTestProcedure()` first (which nulls `Project.TP`), then immediately tests `if (Project.TP == nullptr) return;`. Every line after that — the three `disconnect` calls and the two `DockLeft->Remove` calls — is unreachable. Either move the disconnects *before* `Project.CloseTestProcedure()`, or do the dock cleanup unconditionally.

- [x] **A4 — `LoadTestProcedure` always returns false.** `tPetrelProject.cpp:176-255`. `bool res = false;` is declared and never reassigned, even on the success path. If anything checks the return value (grep for callers — currently I don't see any), it'll think every load failed. Either return a meaningful value or change the signature to `void`.

- [x] **A5 — `tStateMachine.h::tPetrelState::State` is uninitialized.** `tStateMachine.h:11`. `St State;` has no initializer. Any `GetState()` before `SetState()` returns indeterminate data (UB). Fix: `St State = St::Init;`. Also: the enum has only `Init` and `Inited` — is this class actually used anywhere, or dead code? **(?)**

- [x] **A6 — Case mismatch in `BuildDirNames` extension.** `tPetrelProject.cpp:166-168`. Constructs path with `".testProcedure"` (lowercase `t`) but `FindTestProcedures` (line 34) filters for `*.TestProcedure` (uppercase `T`). Works today on NTFS (case-insensitive) but will break on Linux or on a case-sensitive volume. Normalize to one spelling.

- [x] **A7 — `disconnect` on potentially-null `TP`.** `tPetrelProject.cpp:108-120`. `CloseTestProcedure` issues five `disconnect` calls against `TP` without a null check. Qt tolerates `nullptr` sender/receiver in some places, but the SIGNAL/SLOT string form is fragile. Add `if (TP == nullptr) return;` after the existing logic, or guard the disconnect block.

- [x] **A8 — `slotStartTest` switch falls through silently for unhandled states.** `PetrelTT.h:139-150`. The switch only handles `AutoStopped` (and a TODO for `ManualStopped`). No `default:`, no logging for other states. If `slotStartTest` is somehow invoked from another state, you get a silent no-op. Add a `default:` that asserts/logs.

## B. Patterns I'd modernize

- [x] **B1 — Replace string-based SIGNAL/SLOT with pointer-to-member.** Found throughout `PetrelTT.cpp`, `tPetrelProject.cpp`, etc. The new form:
  ```cpp
  connect(PanControl->btnStart, &QPushButton::clicked, this, &tPetrelTT::slotStartTest);
  ```
  ...gives you compile-time signature checking (A2 wouldn't have shipped), lambda support, and better tooling/refactoring. Worth a sweeping refactor.

- [x] **B2 — `using namespace std;` at file scope.** `PetrelTT.cpp:7`. Conventional C++ guidance is to avoid this in `.cpp` files when the file pulls in a lot of headers; you risk name collisions (`distance`, `data`, `swap`...). Either drop it or restrict it to a smaller scope.

- [ ] **B3 — `qDebug()` calls in production flow.** `PetrelTT.cpp:239, 396-419`, `tPetrelProject.cpp:311, 330, 337, 440`, etc. These bypass your `tLogger` — they go to the IDE debug pane, not to the in-app log panel. Route through `Log.LogSystemMessage` (or a new `LogDebug` channel) for consistency, or strip them behind `#ifdef _DEBUG`.

- [x] **B4 — Commented-out code clutter.** Hundreds of `//` lines of dead code in `PetrelTT.h`, `PetrelTT.cpp`, `tPetrelProject.cpp`, `tTestSpec.cpp`. Hard to tell what's load-bearing vs. historical. Suggest a cleanup pass — git history is the right place for archeology.

- [x] **B5 — `tProjectSignaler` stub.** `PetrelTT.cpp:9-14`. Empty `public slots:`, declared `extern` but I don't see a definition. Looks like a forward declaration left over from a refactor. Either wire it up or delete it.

- [x] **B6 — `&(*Children.rbegin())` instead of `&Children.back()`.** `tTestSpec.cpp:25, 34, 43`. Same result, the second form is the idiomatic spelling. Safe today because `std::list` doesn't invalidate references on `push_back`, but `back()` reads more clearly.

- [x] **B7 — `tTimeOut.h` duration math is fragile.** `tTimeOut.h:37, 47`. `(QDateTime - QDateTime).count()` doesn't do what it looks like — Qt's `QDateTime` subtraction doesn't yield a `chrono::duration` in older Qt versions, and even in Qt6 the right call is `Start.msecsTo(QDateTime::currentDateTime())`. Recommend rewriting using `msecsTo` directly. **(?)** Confirm your Qt version so I can suggest exact API.

- [x] **B8 — Magic numbers in `tPetrelTT` ctor.** `PetrelTT.cpp:48`. `400 + 20 + 5` with a comment listing the symbolic names that aren't actually used. Either use the symbolic constants or name a single `kDockWidth` constant.

## C. Style / minor

- [x] **C1 — Recompute of constant.** `tPetrelProject.cpp:35, 92`. `QString(".TestProcedure").length()` is invariant; could be `constexpr int kSuffixLen = sizeof(".TestProcedure") - 1;` or just `chop(14)` with a comment.

- [x] **C2 — Typo "fond" → "found".** `tPetrelProject.cpp:62`.

- [x] **C3 — `closeEvent` doesn't call base or accept.** `PetrelTT.cpp:294-296`. Works because default behavior is to accept, but conventional to either call `QMainWindow::closeEvent(event)` or `event->accept()` explicitly so future readers don't wonder.

- [x] **C4 — Inconsistent enum-value gap.** `PetrelTT.h:50`. `ePanAutoId = 100` floats out alone. If the gap matters (reserved range?), add a comment. If not, give it the next sequential value.

- [x] **C5 — `BuildReportsList` logs size at every level of recursion.** `tPetrelProject.cpp:337`. The `qDebug() << "Linear report size is" << repList.size()` fires inside the recursion, so for a 100-test tree you get 100 log lines saying "1, 2, 3, ...". Move it after the call, in the caller.

## D. Threading & signal/slot safety  *(deeper pass)*

I traced `tTestRunner` + `tTestProcedure` end-to-end. Threading model is:

```
GUI thread                          Worker thread (TestThread)
-----------                         --------------------------
StartAutoTests / StartManualTest    
  -> RunAutoTests / RunManualTests  
       -> TestRunner.RunTest()      
            TP->moveToThread(&TestThread)
            TestThread.start()      [worker event loop begins]
            emit sigRunTest()  --queued-->  TP::slotRunTest()
            POLL LOOP:                         (runs Proc, emits sigs back)
              processEvents()                
              msleep(100)          <--queued sigSetTestInfo, sigSetTestProgress, ...
              check IsNotFinished  
            TestThread.exit/wait                TP::slotRunTest finishes
                                                moveToThread(GUI thread)
```

This design has fundamental problems — items D1, D2, D3 below — and many subtler races on top.

### Severe (must-fix before any concurrent-feature work)

- [x] **D1 — GUI thread is busy-waiting via `msleep+processEvents`.** `tTestRunner.cpp:42-47` and `:60-64`. While a test runs, the GUI thread is stuck in a `do { processEvents(); msleep(100); } while (...)` loop. Pros: re-entrant event processing keeps the window from going *fully* dead. Cons: window updates lag by up to 100ms, every UI event during a test runs in a re-entrant context, and any slot that calls into `StartManualTest`/`StartAutoTests` while one is already running gets you nested polling loops and undefined behaviour. The right fix is event-driven: have `TP` emit `sigTestComplete` when done, drive state via that signal, and never block the GUI thread. Bigger refactor — but the current shape is a recurring source of UI freezes and reentrancy bugs.

- [x] **D2 — `InterruptFlag` is a plain `bool` shared between two threads.** `tTestRunner.h:29` (`bool InterruptFlag`) and `tTestProcedure.h:67` (`bool InterruptFlag`). GUI writes, worker reads (and writes). On x86 a torn read is unlikely, but the compiler is free to hoist the read out of the test's inner loop (no `volatile`, no synchronization → no happens-before). A test that loops on the flag may never observe the change and never interrupt. Fix: `std::atomic<bool>` (and remove `volatile` if you go for it elsewhere — atomic is the right tool).
  - Same applies to `tTestProcedure::CancelTestingFlag` and `IsMessageBoxResultReceived`.

- [x] **D3 — `emit sigSetTestInfo(&CurrentTest.Info)` passes a pointer across threads.** `tTestProcedure.cpp:307, 313`. The signal is `AutoConnection`, becomes queued because sender (TP on worker) and receiver (TestRunner on GUI) live on different threads. By the time the GUI processes it, `CurrentTest` may have been reassigned (line 410: `CurrentTest = TestDict[TestName.toUpper()];`). The receiver dereferences `ti` in `slotSetTestInfo`, racing against the worker mutating `CurrentTest.Info`. Pass `tTestInfo` by value (copy on emit, copy on receive) — or register the type with `qRegisterMetaType` and pass by const-ref-value. Same defect in `sigSetTestInfo` (everywhere it's emitted).

- [x] **D4 — `TestThread.terminate()` is a last-resort weapon used as a normal timeout path.** `tTestRunner.cpp:13`. Qt docs: *"the thread may be terminated at any code point. Threads can be terminated while modifying data. There is no chance for the thread to clean up after itself, unlock any held mutexes, etc."* On a DLL plugin call that's holding a Windows handle, a heap mutex, a Qt internal lock — you can leave the process in an unrecoverable state. Mitigations: a) make tests cooperatively cancellable (the Test_* API already supports it via flags — enforce it); b) only `terminate()` after escalating warnings and never from production paths; c) consider not unloading the DLL after a hard-terminate.

- [ ] **D5 — `Logger` is touched from both threads without synchronization.** `VedroLink\VedroLib\logger\tLogger.h`. `LogMessage` appends to `LogStrings` (QStringList — not thread-safe), writes to `LogFile` (QFile — not thread-safe), and emits `sigLogString`. The signal emission is safe to cross threads (queued auto-connect), but the `LogStrings.append` and `QFile::write` race when called concurrently from GUI and worker. Tests call `Log.LogErrorMessage` during execution; the GUI also calls it from many slots. Either: add a `QMutex` inside `tLogger`, OR route all logging through a single signal+slot pattern where the logger lives on one thread.

### Moderate

- [x] **D6 — `Test_WaitForMessageBoxResult` parameter name mismatch: ms vs sec.** `tTestProcedure.h:175` declares `int timeoutMs`, `tTestProcedure.cpp:286` defines `int timeoutSec` and at line 293 does `WaiterTimer.start(timeoutSec * 1000)`. Compiler doesn't check parameter names — callers reading the header pass milliseconds, get behaviour for seconds. Reconcile.

- [x] **D7 — TP↔Runner signal/slot connections use SIGNAL/SLOT string macros without explicit `Qt::ConnectionType`.** `tPetrelProject.cpp:212-220` (and the mirror in `CloseTestProcedure`). The default `AutoConnection` works *if* the sender and receiver's `thread()` are always different — which is true after `TP->moveToThread`. But: `TP` starts on the GUI thread, gets moved to worker, then moved back. Between tests, both objects live on the GUI thread, so the same signal becomes a *direct call*. That's fine semantically but easy to overlook. Be explicit: `Qt::QueuedConnection` on the cross-thread ones and `Qt::DirectConnection` on the local ones, or convert to the pointer-to-member form which also makes thread semantics easier to reason about (see B1).

- [x] **D8 — `tTestProcedure::slotRunTest` catch-block doesn't catch non-`std::exception`.** `tTestProcedure.cpp:415-419`. If a DLL plugin throws something other than a `std::exception` subclass (a `const char*`, a custom type, an `MSCRT structured exception` translated into a C++ throw, …) it escapes the `try` and `std::terminate()`s the worker thread — taking the whole process with it. Add `catch (...) { Test_AddDetails("Unknown exception in test"); }`.

- [x] **D9 — `tTestProcedure::moveToThread(QApplication::instance()->thread())` from inside `slotRunTest`.** `tTestProcedure.cpp:435`. `moveToThread` must be called from the object's *current* thread — fine here (we're on the worker, TP is on the worker). But this happens *unconditionally at the bottom of `slotRunTest`*, and `slotFinishTest` does it again. If `slotFinishTest` is invoked from the GUI thread (because someone connected `sigFinishTest` GUI-side without queueing), it'll fail. Recommendation: pick *one* place to move TP back, do it from a known thread, and remove the duplicate path. Also: the `slotRunTest` ending makes `TP` change threads while the worker's event loop is still running — that's allowed, but it means later queued events targeting TP will be redispatched to the GUI's event loop mid-test-loop.

- [x] **D10 — `tTestProcedure::Test_WaitForMessageBoxResult` constructs `QTimer` and `QEventLoop` on the worker thread, then connects.** `tTestProcedure.cpp:286-302`. Works because the worker has an event loop (`QThread::run()` default). But the `connect(this, &tTestProcedure::sigMessageResult, &WaiterLoop, &QEventLoop::quit)` happens *after* the worker entered this code. If `sigMessageResult` had fired earlier (e.g. the user clicked the message box very fast, before this connect runs), the event would already be queued and consumed by a non-connected slot — wait would deadlock until timeout. Fix: connect *before* you `emit sigShowMessage`, not inside the waiter.

- [x] **D11 — `tTestRunner::TestThread` is created/destroyed per `RunTest()` call.** `tTestRunner.cpp:37, 70-71`. Every test pays the OS cost of `QThread::start()` and `wait()`. For 100 tests in an auto run, that's 100 thread spin-ups. Keep `TestThread` running for the lifetime of the runner (start it once, exit it in dtor); only the `TP->moveToThread` calls need to happen per test.

- [x] **D12 — Plugin unload while a test is running = process crash.** `tPetrelProject.cpp:152-160` (`ClosePlugin` calls `PluginLib.unload()`). If `CloseTestProcedure` is called while `TestThread` is executing inside the DLL's compiled code, the unload pulls the code out from under the worker. There's no guard. Either: a) refuse to close the plugin while `TestRunner.GetRunningTest()` is true; b) force interrupt-and-wait first, with a timeout.

### Minor

- [x] **D13 — `tTestRunner::tTestRunner` ctor emits `sigSetProgressBar(0)`.** `tTestRunner.cpp:7`. No receivers yet — dropped silently. Harmless today, but if you later auto-connect by name or someone moves the connection earlier, this will fire before anything is wired up. Remove or move into `RunTest`.

- [x] **D14 — `tTestRunner::slotSetTestInfo` reads `TestTree` without synchronization** even though `TestTree` is set from `SetCurTestTree` on the GUI thread before `RunTest` starts. Today it's safe because the assignment happens-before the worker is started (via `TestThread.start()`'s memory barrier). Worth a one-line comment noting the invariant, or guarding with `Q_ASSERT(thread() == qApp->thread())` at slot entry.

- [x] **D15 — `Test_DelayAndSetProgress` doesn't observe `InterruptFlag`.** `tTestProcedure.cpp:227-241`. A test calling `Test_DelayAndSetProgress(10000)` will sit in the `msleep` loop ignoring soft interrupts. Add a check between iterations: `if (InterruptFlag) return;`.

- [x] **D16 — `tTestProcedure::InterruptFlag` and `tTestRunner::InterruptFlag` are two separate variables.** `tTestRunner.cpp:53` sets the runner's flag; the TP's flag is only set via the queued `sigInterruptTest -> slotInterruptTest` path (`tTestProcedure.cpp:699-701`). If the soft interrupt event is still queued (waiting for worker's processEvents) when the test polls, the test sees `InterruptFlag == false` and keeps running. Either: a) collapse to one atomic flag the worker reads directly, or b) make slotInterruptTest a `Qt::DirectConnection` so the flag flips synchronously (but that requires it to be thread-safe — atomic).

## E. What I'd do first (recommended order)

1. **D2 + D16** — collapse the two `InterruptFlag`s into one `std::atomic<bool>` owned by `tTestProcedure`. Smallest change, fixes a real bug, unblocks #2.
2. **D3** — make signals pass `tTestInfo` by value (and `qRegisterMetaType<tTestInfo>()` if not already). Same one-day effort, removes a UB risk.
3. **A1, A2, A3** — three brain-dead correctness fixes from Section A. Half a day total.
4. **D5** — add a `QMutex` to `tLogger` (one mutex, lock around `LogStrings` and `LogFile` ops). One hour.
5. **D8** — `catch (...)` in `slotRunTest`. Ten minutes.
6. **B1** — incremental migration to pointer-to-member `connect()`. Do it per-file when you next touch each file. Don't bulk-rewrite — sweep over weeks.
7. **D1** — the big one. Plan it: replace polling loop with event-driven completion. This is a multi-day refactor but eliminates a whole class of UI bugs (and would be the right groundwork before D4/D11).

## Open questions still standing

- **Target platforms** — Windows-only forever, or planning Linux/Mac? A6 (case-sensitive paths) and the `.dll` loader change priority depending on the answer.
- **Plugin authoring** — are plugin DLLs written by you, or by third parties? Affects how defensively `tTestProcedure` should be guarding against misbehaving plugins (D8, D12, D15).
- **`tStateMachine.h`** — used anywhere, or dead code? Grep'd briefly and didn't find callers.
