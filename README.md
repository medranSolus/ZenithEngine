# Zenith Engine

3D graphics engine.

Before contributing read [coding guidelines](Doc/CodeGuidelines.md) for style conventions and formatting. It is recomended to use [CodeMaid](http://www.codemaid.net/) for auto formatting.

Working with project from command prompt is described [here](Doc/CMakeCLI.md).

**Requirements:**
 - Windows SDK 10.0.20348.0
 - Vulkan SDK 1.3.250.0
 - CMake 3.22

**External libraries:**
 - [AMD GPU Services](https://github.com/GPUOpen-LibrariesAndSDKs/AGS_SDK) - [MIT License](Doc/License/ThirdParty/AGS.txt)
 - [Assimp](https://github.com/assimp/assimp) - [Modified 3-clause BSD-License](Doc/License/ThirdParty/Assimp.txt)
 - [Dear ImGui](https://github.com/ocornut/imgui) - [The MIT License](Doc/License/ThirdParty/Dear_ImGui.txt)
 - [DirectXMath](https://github.com/microsoft/DirectXMath) - [The MIT License (MIT)](Doc/License/ThirdParty/DirectXMath.txt)
 - [DirectXTex](https://github.com/microsoft/DirectXTex) - [The MIT License (MIT)](Doc/License/ThirdParty/DirectXTex.txt)
 - [EnTT](https://github.com/skypjack/entt) - [The MIT License (MIT)](Doc/License/ThirdParty/EnTT.txt)
 - [FreeType](https://gitlab.freedesktop.org/freetype/freetype) - [FREETYPE LICENSES](Doc/License/ThirdParty/FreeType.txt)
 - [HarfBuzz](https://github.com/harfbuzz/harfbuzz) - [Old MIT license](Doc/License/ThirdParty/HarfBuzz.txt)
 - [libpng](https://github.com/glennrp/libpng) - [PNG Reference Library License](Doc/License/ThirdParty/libpng.txt)
 - [Render Pipeline Shaders](https://github.com/GPUOpen-LibrariesAndSDKs/RenderPipelineShaders) - [AMD Internal Evaluation License](Doc/License/ThirdParty/RenderPipelineShaders.txt)
 - [volk](https://github.com/zeux/volk) - [MIT License](Doc/License/ThirdParty/volk.txt)
 - [zlib](https://github.com/madler/zlib) - [zlib License](Doc/License/ThirdParty/zlib.txt)
 - [AgilitySDK 1.610.4](https://devblogs.microsoft.com/directx/directx12agility/) - Binary: [MICROSOFT SOFTWARE LICENSE TERMS](Doc/License/ThirdParty/AgilitySDK.txt), Code: [MIT License](Doc/License/ThirdParty/AgilitySDK-code.txt)
 - [DirectXShaderCompiler 1.7.2212.1](https://github.com/microsoft/DirectXShaderCompiler) - [LLVM Release License](Doc/License/ThirdParty/DirectXShaderCompiler.txt)
 - [FXC shader compiler 10.0.20348.1](https://docs.microsoft.com/en-us/windows/win32/direct3dtools/fxc) - [MICROSOFT SOFTWARE LICENSE](Doc/License/ThirdParty/WindowsSdk.rtf), [Third Party Notices](Doc/License/ThirdParty/WindowsSdkThirdPartyNotices.rtf)
 - [JSON for modern C++ 3.11.2](https://github.com/nlohmann/json) - [MIT LICENSE](Doc/License/ThirdParty/json.txt)
 - [stb_sprintf 1.10](https://github.com/nothings/stb/blob/master/stb_sprintf.h) - [Public Domain](Doc/License/ThirdParty/stb_printf.txt)
 - [WinPixEventRuntime 1.0.230302001](https://www.nuget.org/packages/WinPixEventRuntime) - [MICROSOFT SOFTWARE LICENSE](Doc/License/ThirdParty/WinPixEventRuntime.txt), [Third Party Notices](Doc/License/ThirdParty/WinPixEventRuntimeThirdPartyNotices.txt)
 - [XeGTAO 1.30](https://github.com/GameTechDev/XeGTAO) - [MIT LICENSE](Doc/License/ThirdParty/XeGTAO.txt)

## License

Zenith Engine project is divided into 4 distinct parts, each with license on it's own.
Details about each subproject's license can be found in [Doc/License](Doc/License) directory:
 - `Engine` - [Engine.md](Doc/License/Engine.md), main engine static library.
 - `Demo` - [Demo.md](Doc/License/Demo.md), technological demo.
 - `EditTool` - [EditTool.md](Doc/License/EditTool.md), CLI utility tool.
 - `Common` - unlicensed, usage outside of projects stated above is prohibited.