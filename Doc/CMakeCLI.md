# Building with CMake from command line

Before using provided tool you must allow running user scripts by **Powershell**. To do that open PS command prompt as an admin and run `Set-ExecutionPolicy unrestricted`.

Project is based upon **Ninja** generator and requires its binary to be accessible inside `PATH`. If you have Visual Studio installed on your machine, simply append `PATH` with value:
`%VS`**<VERSION>**`INSTALLDIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja`, where <VERSION> corresponds to installed Visual Studio, ex. for VS 2019 simply use **2019** in its place.
Also make sure that CMake can access compiler of your choice, to use 'cl.exe' from Visual Studio just use **x64 Native Tools Command Prompt**.

Script by default bulds the project, but supports also following commands:
  - `help` - display tool syntax,
  - `init` - initialize external submodules,
  - `clean/clear` - clear the build system,
  - `up` - update external submodules,
  - `gen` - generate of CMake build system,
  - `run` - run builded technological demo.

Project contains following configurations needed by `gen`, `run` and default command (case insensitive):
  - **Debug** - pass `D` or `Debug`,
  - **Development** - pass `Dev` or `Development`,
  - **Profile** - pass `P` or `Profile`,
  - **Release** - pass `R` or `Release`,
  - **CI tools** - pass `CI` to configure only for CI jobs, same as **Release** but without engine data.

Tool syntax:
`./build.ps1 <COMMAND> <MODE> <ARGS>`