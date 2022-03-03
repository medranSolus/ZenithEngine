# Zenith Engine

3D graphics engine.

Before contributing read [coding guidelines](Doc/CodeGuidelines.md) for style conventions and formatting. It is recomended to use [CodeMaid](http://www.codemaid.net/) for auto formatting.

Working with project from command prompt is described [here](Doc/CMakeCLI.md).

**External libraries:**
  - [FXC shader compiler 10.0.19041.685](https://docs.microsoft.com/en-us/windows/win32/direct3dtools/fxc)
  - [WinPixEventRuntime 1.0.220124001](https://www.nuget.org/packages/WinPixEventRuntime)
  - [JSON for modern C++ 3.10.4](https://github.com/nlohmann/json)
  - [stb_sprintf 1.10](https://github.com/nothings/stb/blob/master/stb_sprintf.h)
  - [Dear ImGui](https://github.com/ocornut/imgui)
  - [FreeType](https://gitlab.freedesktop.org/freetype/freetype)
  - [DirectXMath](https://github.com/microsoft/DirectXMath)
  - [DirectXTex](https://github.com/microsoft/DirectXTex)
  - [Assimp](https://github.com/assimp/assimp)
  - [EnTT](https://github.com/skypjack/entt)

## License

Zenith Engine project is divided into 4 distinct parts, each with license on it's own.
Details about each subproject's license can be found in [Doc/License](Doc/License) directory:
  - `Engine` - [Engine.md](Doc/License/Engine.md), main engine static library.
  - `Demo` - [Demo.md](Doc/License/Demo.md), technological demo.
  - `EditTool` - [EditTool.md](Doc/License/EditTool.md), CLI utility tool.
  - `Common` - unlicensed, usage outside of projects stated above is prohibited.