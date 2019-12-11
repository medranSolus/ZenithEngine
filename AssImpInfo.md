# AssImp dependency info

There are precompiled (MSVC++ 14.16, toolset v141) DLLs for AssImp 5.0.0 available in *LibsExternal*.
DLLs are create to suit both **x86/x64** configurations and **Debug/Release** targets.
You can get them only by cloning the repository since they are stored with **Git LFS**.
In order to create them on your own read the following guide.

---

### Building AssImp

In order to create them yourself you need CMake tool for Windows. Here are steps for graphical installation:

1. Run CMake and set both source and binaries directory to local location of repository folder *AssImp*.
2. Click button *Configure* and select installed Visual Studio and desired platform version.
3. Click *Generate* to create project files and then select *Open Project*.
4. Select desired target (recomended providing both Debug and Release) and build solution.
5. Go to *AssImp/bin* folder and one of targets subfolders.
    - For **Release** configuration delete *.exe files and change name of assimp dll file to **assimp.dll** and then copy it and remaining dlls to right directory in *LibsExternal*.
    - For **Debug** configuration perform same steps as for Release but also delete corresponding *.pbd and *.ilk files for executables.
  Change names of assimp dll debug files to **assimp.pbp** and **assimp.ilk** and move them and other remaining debug files to right directory in *LibsExternal*.
6. To get set of DLLs for other platforms delete contents of *AssImp*, redownload missing source files and then follow steps 1-5 for other platforms as desired.
7. After everything delete contents of *AssImp* since it will be redownloaded automatically.

---

### Note

If you find more efficient solution to get multiple platforms at once please let me now.