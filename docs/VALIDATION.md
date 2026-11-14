# Manual Windows validation

Status: **pending for the repository cleanup changes**. This checklist is not a
record of successful execution. Existing screenshots show earlier lab runs.

## Build

1. Record the Windows version, compiler version and target architecture.
2. Build using the CMake commands in the README.
3. Review compiler warnings; record any unresolved warnings.
4. If documenting the direct `cl` build as validated, run that command separately.

## Runtime cases

| Case | Expected result |
| --- | --- |
| Token does not contain `SeDebugPrivilege` | Clear failure and non-zero exit status; no success claim. |
| Input reaches EOF without a process name | Clear input failure and non-zero exit status. |
| Nonexistent process name | No accessible matching process reported; no child launched. |
| Accessible running parent, sufficient rights | Notepad is created; selected parent PID and new child PID are printed. |
| Selected parent cannot be opened for creation | Failure reported; no claim that changing the parent succeeded. |

For a successful run, compare the selected PID and the new PID with a process
viewer and available process-creation telemetry. Record what each source shows;
do not infer an EDR bypass from the visible tree. Close the Notepad window after
the check. Protected targets and process-exit races can legitimately fail.

## Record a result

```text
Commit tested:
Windows version:
Compiler / SDK:
Architecture:
Build command and warnings:
Runtime case:
Exit status:
Observed parent / child PIDs:
Telemetry source and observation:
Remaining limitations:
```
