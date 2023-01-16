param($command, $mode)

function Display-Info
{
    Write-Output "`n>Zenith Engine CLI build tool syntax:"
    Write-Output "    build.ps1"
    Write-Output "        <COMMAND: (default - build project)"
    Write-Output "            help - display tool syntax (MODE not required)"
    Write-Output "            init - initialize submodules (MODE not required)"
    Write-Output "            clean/clear - clear the build system (MODE not required)"
    Write-Output "            up - update submodules (MODE not required)"
    Write-Output "            gen - generate build system"
    Write-Output "            run - run tech demo (you can specify additional arguments after MODE parameter that will be passed to the application)>"
    Write-Output "        <MODE: D|Debug; Dev|Development; P|Profile; R|Release; CI (static analysis setup)>"
    Write-Output "        <ARGS: additional parameters for run command>`n"
}

Switch ($command)
{
    {($_ -eq "clean") -or ($_ -eq "clear")}
    {
        Get-ChildItem Bin -Recurse | Remove-Item -Recurse
        Get-ChildItem Build -Recurse | Remove-Item -Recurse
        exit 0
    }
    "init"
    {
        git submodule update --init
        exit 0
    }
    "up"
    {
        git submodule update --remote --merge
        Get-ChildItem External/Bin -Recurse | Remove-Item -Recurse
        exit 0
    }
    "help"
    {
        Display-Info
        exit 0
    }
    default
    {
        if (!$mode)
        {
            $mode=$command
        }
    }
}

$build_type
$build_preset
Switch ($mode)
{
    {($_ -eq "r") -or ($_ -eq "release") -or ($_ -eq "ci")}
    {
        $build_type="Release"
        $build_preset="Release-Win"
        break
    }
    {($_ -eq "p") -or ($_ -eq "profile")}
    {
        $build_type="Profile"
        $build_preset="Profile-Win"
        break
    }
    {($_ -eq "dev") -or ($_ -eq "development")}
    {
        $build_type="Development"
        $build_preset="Development-Win"
        break
    }
    {($_ -eq "d") -or ($_ -eq "debug")}
    {
        $build_type="Debug"
        $build_preset="Debug-Win"
        break
    }
    default
    {
        Write-Output "`nNot specified correct build mode!"
        Display-Info
        exit -1
    }
}

$obj_dir="Build/$build_type"
$bin_dir="Bin/$build_type"
Switch ($command)
{
    "gen"
    {
        $args=""
        if ($mode -eq "ci")
        {
            $args="-DZE_CI_JOB=ON"
        }
        cmake $args -S ./ --preset=$build_preset
        break
    }
    "run"
    {
        if (Test-Path -Path "$bin_dir/ZenithDemo.exe" -PathType Leaf)
        {
            cd "$bin_dir"
            Start-Process -NoNewWindow "./ZenithDemo.exe" -ArgumentList $args
            cd ../..
        }
        else
        {
            Write-Output "`nApplication must be built first!`n"
        }
        break
    }
    default
    {
        if (Test-Path -Path "$obj_dir" -PathType Container)
        {
            cmake --build "$obj_dir" --config $build_type
        }
        else
        {
            Write-Output "`nBuild system must be generated first!`n"
        }
        break
    }
}