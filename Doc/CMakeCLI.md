# Building with CMake from command line

Before using provided tool you must allow running user scripts by **Powershell**. To do that open PS command prompt as an admin and run `Set-ExecutionPolicy unrestricted`.

Project is based upon **Ninja** generator and requires its binary to be accessible inside `PATH`. If you have Visual Studio installed on your machine, simply append `PATH` with value:
`%VS`**<VERSION>**`INSTALLDIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja`, where <VERSION> corresponds to installed Visual Studio, ex. for VS 2019 simply use **2019** in its place.
Also make sure that CMake can access compiler of your choice.

Project contains following configurations needed by provided script (case insensitive):
  - **Debug** - pass `D` or `Debug`,
  - **Release with debug info** - pass `Ri` or `ReleaseInfo`,
  - **Release** - pass `R` or `Release`.

Script by default bulds the project, but supports also following commands:
  - `gen` - generation of CMake build system,
  - `run` - running builded application.

Tool syntax:
`build.ps1 <CONFIGURATION> <COMMAND>`