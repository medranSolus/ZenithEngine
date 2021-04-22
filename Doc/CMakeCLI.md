# Building with CMake from command line

Before using provided tool you must allow running user scripts by **Powershell**. To do that open PS command prompt as an admin and run `Set-ExecutionPolicy unrestricted`

Project contains following configurations needed by provided script (case insensitive):
  - **Debug** - pass `D` or `Debug`,
  - **Release with debug info** - pass `Ri` or `ReleaseInfo`,
  - **Release** - pass `R` or `Release`.

Script by default bulds the project, but supports also following commands:
  - `gen` - generation of CMake build system,
  - `run` - running builded application.

Tool syntax:
`build.ps1 <CONFIGURATION> <COMMAND>`