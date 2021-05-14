param($mode, $command) 
$build_type

Switch ($mode.ToLower())
{
    {($_ -eq "r") -or ($_ -eq "release")}
    {
        $build_type="Release"
        break
    }
    {($_ -eq "ri") -or ($_ -eq "releaseinfo")}
    {
        $build_type="RelWithDebInfo"
        break
    }
    {($_ -eq "d") -or ($_ -eq "debug")}
    {
        $build_type="Debug"
        break
    }
    default
    {
        Write-Output "`nNot specified correct build mode! Syntax:"
        Write-Output "    build.ps1`n        <MODE: R|Release; Ri|ReleaseInfo; D|Debug>"
        Write-Output "        [<COMMAND: gen - generate build system; run - run application; (default - build application)>]`n"
        exit -1
    }
}

$obj_dir="Build/$build_type"
$bin_dir="Bin/$build_type"
Switch ($command)
{
    "gen"
    {
        cmake -S . -B "$obj_dir" -D CMAKE_BUILD_TYPE=$build_type
        break
    }
    "run"
    {
        if (Test-Path -Path "$bin_dir/zenith.exe" -PathType Leaf)
        {
            cd "$bin_dir"
            Start-Process -NoNewWindow "./zenith.exe"
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