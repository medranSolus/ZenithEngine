# Zenith Engine

3D graphics engine.

Before contributing read [coding guidelines](Doc/CodeGuidelines.md) for style conventions and formatting. It is recomended to use [CodeMaid](http://www.codemaid.net/) for auto formatting.

Working with project from command prompt is described [here](Doc/CMakeCLI.md).

**External libraries:**
  - [zlib](https://github.com/madler/zlib) - [zlib License](Doc/License/ThirdParty/zlib.txt)
  - [EnTT](https://github.com/skypjack/entt) - [The MIT License (MIT)](Doc/License/ThirdParty/EnTT.txt)
  - [DirectXMath](https://github.com/microsoft/DirectXMath) - [The MIT License (MIT)](Doc/License/ThirdParty/DirectXMath.txt)
  - [DirectXTex](https://github.com/microsoft/DirectXTex) - [The MIT License (MIT)](Doc/License/ThirdParty/DirectXTex.txt)
  - [Assimp](https://github.com/assimp/assimp) - [Modified 3-clause BSD-License](Doc/License/ThirdParty/Assimp.txt)
  - [Dear ImGui](https://github.com/ocornut/imgui) - [The MIT License](Doc/License/ThirdParty/Dear_ImGui.txt)
  - [FreeType](https://gitlab.freedesktop.org/freetype/freetype) - [FREETYPE LICENSES](Doc/License/ThirdParty/FreeType.txt)
  - [stb_sprintf 1.10](https://github.com/nothings/stb/blob/master/stb_sprintf.h) - [Public Domain](Doc/License/ThirdParty/stb_printf.txt)
  - [XeGTAO 1.30](https://github.com/GameTechDev/XeGTAO) - [MIT LICENSE](Doc/License/ThirdParty/XeGTAO.txt)
  - [JSON for modern C++ 3.10.5](https://github.com/nlohmann/json) - [MIT LICENSE](Doc/License/ThirdParty/json.txt)
  - [WinPixEventRuntime 1.0.220124001](https://www.nuget.org/packages/WinPixEventRuntime) - [MICROSOFT SOFTWARE LICENSE](Doc/License/ThirdParty/WinPixEventRuntime.txt), [Third Party Notices](WinPixEventRuntimeThirdPartyNotices.txt)
  - [FXC shader compiler 10.0.20348.1](https://docs.microsoft.com/en-us/windows/win32/direct3dtools/fxc) - [MICROSOFT SOFTWARE LICENSE](Doc/License/ThirdParty/WindowsSdk.rtf), [Third Party Notices](Doc/License/ThirdParty/WindowsSdkThirdPartyNotices.rtf)

## License

Zenith Engine project is divided into 4 distinct parts, each with license on it's own.
Details about each subproject's license can be found in [Doc/License](Doc/License) directory:
  - `Engine` - [Engine.md](Doc/License/Engine.md), main engine static library.
  - `Demo` - [Demo.md](Doc/License/Demo.md), technological demo.
  - `EditTool` - [EditTool.md](Doc/License/EditTool.md), CLI utility tool.
  - `Common` - unlicensed, usage outside of projects stated above is prohibited.