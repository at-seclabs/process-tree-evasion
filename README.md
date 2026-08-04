# Process tree evasion lab

Small Windows proof of concept for studying parent process ID (PPID)
spoofing and the resulting process-tree telemetry.

The program starts `notepad.exe` with a user-selected process set as its parent
through `PROC_THREAD_ATTRIBUTE_PARENT_PROCESS`. I wrote it to better understand
Windows process creation, token privileges, and what endpoint monitoring tools
can observe when the parent shown in a process tree is not the process that
actually initiated the operation.

This is a learning project, not an EDR bypass. Modern endpoint products can use
telemetry beyond the reported parent PID, and the technique may generate alerts.

## What it does

1. Requests `SeDebugPrivilege` for the current process token.
2. Finds the oldest accessible instance of a process selected by name.
3. Opens that process with the rights required to use it as a parent.
4. Creates a `STARTUPINFOEX` attribute list containing the selected parent.
5. Starts `%WINDIR%\System32\notepad.exe` with
   `EXTENDED_STARTUPINFO_PRESENT`.
6. Reports the new PID and releases the allocated memory and handles.

The payload is intentionally fixed to Notepad. The repository does not include
shellcode, a beacon, persistence, or a mechanism for injecting code into the
new process.

## Repository layout

```text
.
├── images/     Screenshots from local lab runs
├── main.cpp    Process creation and attribute-list setup
├── proc.cpp    Privilege, process lookup, and handle helpers
└── proc.h      Helper declarations
```

## Requirements

- Windows 10 or 11
- A C++ compiler and Windows SDK, such as Visual Studio with the Desktop
  development with C++ workload
- An elevated terminal for targets that require `SeDebugPrivilege`

The project uses the Windows API and must link against `Advapi32.lib` for the
token privilege functions. No Visual Studio solution or other build definition
is currently included, so the source files need to be added to a Windows console
application manually.

## Running the lab

Run the compiled program from an elevated terminal, then enter the executable
name of a running process when prompted. The program launches Notepad and prints
the selected parent PID and the PID of the newly created process.

Use a disposable Windows test environment with endpoint telemetry enabled. A
process viewer alone shows only part of the story; the useful exercise is to
compare the displayed process tree with the events and alerts recorded by the
monitoring stack.

Example screenshots from local runs are available in [`images/`](./images/).

### Example: `explorer.exe` as the reported parent

![Notepad started with explorer.exe as its reported parent](./images/explorer.png)

### Example: `svchost.exe` as the reported parent

![Notepad started with svchost.exe as its reported parent](./images/svchost.png)

## Implementation notes

The proof of concept uses the documented extended process creation API:

- `OpenProcessToken` and `AdjustTokenPrivileges`
- `CreateToolhelp32Snapshot` and `Process32FirstW` / `Process32NextW`
- `InitializeProcThreadAttributeList`
- `UpdateProcThreadAttribute`
- `CreateProcessA`

Enabling `SeDebugPrivilege` is not privilege escalation. It only enables a
privilege already present in the current token, which is why an elevated process
is normally required for this lab.

## Limitations

- The child executable is fixed to `notepad.exe`.
- Only process names can be entered; when several instances exist, the oldest
  accessible process is selected.
- Cross-architecture behavior has not been validated.
- There are no automated tests or reproducible build files yet.
- Detection results depend on the Windows version and monitoring product.
- The code demonstrates parent selection; it does not demonstrate that an EDR
  has been bypassed.

## Responsible use

This repository is intended for Windows internals study, defensive research,
and authorized security testing. Use it only on systems you own or are explicitly
permitted to assess.
