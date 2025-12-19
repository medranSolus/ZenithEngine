# Zenith Engine

3D graphics engine.

Before contributing read [coding guidelines](Doc/CodeGuidelines.md) for style conventions and formatting. It is recomended to use [CodeMaid](http://www.codemaid.net/) for auto formatting.

Working with project from command prompt is described [here](Doc/CMakeCLI.md).

**Requirements:**
 - Windows SDK 10.0.20348.0
 - Vulkan SDK 1.3.250.0
 - CMake 3.22

## Development progress

Current tasks and issues are tracked with associated [project page](https://github.com/users/medranSolus/projects/5) where all issues are grouped into corresponding categories.
Additional list of in-code TODO actions is located [here](Doc/TODO.md) for purpose of tracking possible changes and improvements not large enough to be put into separate issue.

## License

Zenith Engine project is divided into 4 distinct parts, each with license on it's own.
Details about each subproject's license can be found in [Doc/License](Doc/License) directory:
 - `Engine` - [Engine.md](Doc/License/Engine.md), main engine static library.
 - `Demo` - [Demo.md](Doc/License/Demo.md), technological demo.
 - `EditTool` - [EditTool.md](Doc/License/EditTool.md), CLI utility tool.
 - `Common` - unlicensed, usage outside of projects stated above is prohibited.
 
**External libraries:**
 - [AMD GPU Services](https://github.com/GPUOpen-LibrariesAndSDKs/AGS_SDK) - [MIT License](Doc/License/ThirdParty/AGS.txt)
 - [Assimp](https://github.com/assimp/assimp) - [Modified 3-clause BSD-License](Doc/License/ThirdParty/Assimp.txt)
 - [Brotli](https://github.com/google/brotli) - [The MIT License](Doc/License/ThirdParty/brotli.txt)
 - [bzip2](https://gitlab.com/bzip2/bzip2) - [libbzip2 License v1.0.6](Doc/License/ThirdParty/bzip2.txt)
 - [Dear ImGui](https://github.com/ocornut/imgui) - [The MIT License](Doc/License/ThirdParty/Dear_ImGui.txt)
 - [DirectXMath](https://github.com/microsoft/DirectXMath) - [The MIT License (MIT)](Doc/License/ThirdParty/DirectXMath.txt)
 - [DLSS](https://github.com/NVIDIA/DLSS) - [NVIDIA RTX SDKs LICENSE](Doc/License/ThirdParty/DLSS.txt)
 - [EnTT](https://github.com/skypjack/entt) - [The MIT License (MIT)](Doc/License/ThirdParty/EnTT.txt)
 - [FreeType](https://gitlab.freedesktop.org/freetype/freetype) - [FREETYPE LICENSES](Doc/License/ThirdParty/FreeType.txt)
 - [FidelityFX SDK](https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK) - [MIT License](Doc/License/ThirdParty/FidelityFXSDK.md)
 - [HarfBuzz](https://github.com/harfbuzz/harfbuzz) - [Old MIT license](Doc/License/ThirdParty/HarfBuzz.txt)
 - [libpng](https://github.com/glennrp/libpng) - [PNG Reference Library License](Doc/License/ThirdParty/libpng.txt)
 - [libspng](https://github.com/randy408/libspng) - [BSD 2-Clause License](Doc/License/ThirdParty/libspng.txt)
 - [NVIDIA Image Scaling SDK](https://github.com/NVIDIAGameWorks/NVIDIAImageScaling) - [The MIT License(MIT)](Doc/License/ThirdParty/NvidiaImageScaling.txt), [Third Party Notices](Doc/License/ThirdParty/NvidiaImageScalingThirdPartyNotices.txt)
 - [qoixx](https://github.com/wx257osn2/qoixx) - [MIT](Doc/License/ThirdParty/qoixx.txt)
 - [XeSS](https://github.com/intel/xess) - [Intel Simplified Software License](Doc/License/ThirdParty/XeSS.txt), [Third Party Notices](Doc/License/ThirdParty/XeSSThirdPartyNotices.txt)
 - [zlib](https://github.com/madler/zlib) - [zlib License](Doc/License/ThirdParty/zlib.txt)
 - [AgilitySDK v1.618.5](https://devblogs.microsoft.com/directx/directx12agility/) - Binary: [MICROSOFT SOFTWARE LICENSE TERMS](Doc/License/ThirdParty/AgilitySDK.txt), Code: [MIT License](Doc/License/ThirdParty/AgilitySDK-code.txt)
 - [DirectStorage v1.3.0](https://devblogs.microsoft.com/directx/directstorage-api-downloads/) - [MICROSOFT SOFTWARE LICENSE TERMS](Doc/License/ThirdParty/DirectStorage.txt), Code: [MIT License](Doc/License/ThirdParty/DirectStorage-code.txt), [Third Party Notices](Doc/License/ThirdParty/DirectStorageThirdPartyNotices.rtf)
 - [DirectXShaderCompiler v1.8.2505.1](https://github.com/microsoft/DirectXShaderCompiler) - [General MIT License](Doc/License/ThirdParty/DirectXShaderCompiler-Project.txt), [Microsoft Project License](Doc/License/ThirdParty/DirectXShaderCompiler-Project.txt), [LLVM Release License](Doc/License/ThirdParty/DirectXShaderCompiler-LLVM.txt), [HLSL Standard Header Library](Doc/License/ThirdParty/DirectXShaderCompiler-HLSL_Standard_Header_Library.txt)
 - [FidelityFX SDK v1.1.4](https://github.com/medranSolus/FidelityFX-SDK-v1.1.4) - [MIT License](Doc/License/ThirdParty/FidelityFXSDK_Legacy.txt)
 - [FXC shader compiler v10.0.26100.4188](https://docs.microsoft.com/en-us/windows/win32/direct3dtools/fxc) - [MICROSOFT SOFTWARE LICENSE](Doc/License/ThirdParty/WindowsSdk.rtf), [Third Party Notices](Doc/License/ThirdParty/WindowsSdkThirdPartyNotices.rtf)
 - [JSON for modern C++ v3.12.0](https://github.com/nlohmann/json) - [MIT LICENSE](Doc/License/ThirdParty/json.txt)
 - [stb_image v2.30](https://github.com/nothings/stb/blob/master/stb_image.h) - [Public Domain](Doc/License/ThirdParty/stb.txt)
 - [stb_image_write v1.16](https://github.com/nothings/stb/blob/master/stb_image_write.h) - [Public Domain](Doc/License/ThirdParty/stb.txt)
 - [stb_sprintf v1.10](https://github.com/nothings/stb/blob/master/stb_sprintf.h) - [Public Domain](Doc/License/ThirdParty/stb.txt)
 - [volk v1.3.250](https://github.com/zeux/volk) - [MIT License](Doc/License/ThirdParty/volk.txt)
 - [WinPixEventRuntime v1.0.240308001](https://www.nuget.org/packages/WinPixEventRuntime) - [MICROSOFT SOFTWARE LICENSE](Doc/License/ThirdParty/WinPixEventRuntime.txt), [Third Party Notices](Doc/License/ThirdParty/WinPixEventRuntimeThirdPartyNotices.txt)
 - [XeGTAO v1.30](https://github.com/GameTechDev/XeGTAO) - [MIT LICENSE](Doc/License/ThirdParty/XeGTAO.txt)

 ## [Changelog](Doc/Changelog.md)