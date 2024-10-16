# Changelog

**Tags:**
- **[ARCH]** Engine architectural change
- **[FTR]** New feature
- **[OPT]** Optimization
- **[BUG]** Bugs to fix
- **[REL]** Planned for future release
- **[REL-X.X]** Release version
- **[OTHR]** Other project subject

---

## v0.4 *[2024-03-05]*

PBR update pre-release. Changed whole pipeline to PBR and introduced new FidelityFX SDK features, along with upscaling.
As of this version only DirectX 12 is working correctly due to missing backends for other APIs.
To correctly start technological demo it's needed to pass `/dx12 /threadsCount=255` command line arguments.

**List of changes:**
- **[FTR]** Added FidelityFX SDK 1.0 with effects:
  - CACAO
  - FSR 1
  - FSR 2
- **[FTR]** Added upscalers:
  - FSR 1
  - FSR 2
  - NIS
  - XeSS v1.2.0
- **[BUG]** All meshes inside nodes moves on command now.
- **[FTR]** Added usage of Direct Storage v1.2.2 for loading data from disk. For now only DirectX 12 version works since no backend created for DirectX 11 and Vulkan yet.
  Multithreading disabled till better way of adding new components is introduced.
- **[ARCH] [FTR] [REL-0.4]** Changed pipeline into [PBR](https://learnopengl.com/PBR/Theory). Metalness workflow chosen, option to add support for specular in the future.

---

## v0.3 *[2022-09-29]*

Master's degree pre-release. Rewritten engine to support multiple graphic's APIs (currently DirectX 11 & 12) and based it's architecture on ECS model.

**List of changes:**
- **[ARCH]** Consider changing similar classes into templated ones for better readability.
- **[BUG]** Icon file is not correctly embedded into executable.
- **[OPT]** Test if some fullscreen work can be implemented as compute shader (it is said that they are faster for image processing than pixel shaders). Passes converted to CS:
  - SSAO (blur merged).
- **[ARCH]** Convert shaders to ubershader approach (maybe faster due to all the switching of shader code?).
  Desired in better transition to usage of [PSOs](https://docs.microsoft.com/en-us/windows/win32/direct3d12/managing-graphics-pipeline-state-in-direct3d-12)
  rather than explicit Bind calls. Also required splitting work into passes with different vertex buffer organizations.
- **[ARCH] [FTR]** Add support for DirectX 12.
- **[ARCH] [FTR] [REL-0.3]** Rewrite design for multithreaded ECS model with API abstraction layer.
  Components can have hashmaps over them with indicies into arrays via object id for quick access.

---

## v0.2 *[2021-05-15]*

Library decomposition pre-release. Zenith Engine consists now of static library and technological demo that is provided here along with CLI edit tool.

**List of changes:**
- **[BUG]** Don't select dirs when searching for models.
- **[ARCH]** Add Assimp as submodule instead of holding whole structure inside.
  ~~Fork Assimp for stable source for project in case original repo could be deleted (not likely but still).~~
- **[ARCH]** Transform Euler angle representation to Quaternions.
- **[ARCH] [REL-0.2]** Decompose engine into library and tech demo.

---

## v0.1 *[2020-11-21]*

Engineering degree pre-release with demo scene.

**List of changes:**
- **[FTR]** Add Light Volumes.
- **[FTR]** Add multiple light sources.
- **[FTR]** Add SSAO.
- **[FTR]** Add spot light.
- **[FTR]** Add directional light.
- **[FTR] [REL-0.1]** Added frustum culling.