#pragma once

#include "RenderPass/CACAO.h"
#include "RenderPass/ClearBuffer.h"
#include "RenderPass/CopyBuffer.h"
#include "RenderPass/DearImGui.h"
#include "RenderPass/DirectionalLight.h"
#include "RenderPass/HorizontalBlur.h"
#include "RenderPass/Lambertian.h"
#include "RenderPass/LambertianComputeCopy.h"
#include "RenderPass/LightCombine.h"
#include "RenderPass/LoadLightmapsDiffuse.h"
#include "RenderPass/LoadLightmapsSpecular.h"
#include "RenderPass/LoadSkybox.h"
#include "RenderPass/OutlineDraw.h"
#include "RenderPass/PointLight.h"
#include "RenderPass/Skybox.h"
#include "RenderPass/SpotLight.h"
#include "RenderPass/SSSR.h"
#include "RenderPass/TonemapExposure.h"
#include "RenderPass/TonemapLPM.h"
#include "RenderPass/TonemapReinhard.h"
#if _ZE_DLSS_ENABLED
#	include "RenderPass/UpscaleDLSS.h"
#endif
#if _ZE_FFXAPI_ENABLED
#	include "RenderPass/UpscaleFfxFSR.h"
#endif
#include "RenderPass/UpscaleFSR1.h"
#include "RenderPass/UpscaleFSR2.h"
#include "RenderPass/UpscaleFSR3.h"
#include "RenderPass/UpscaleNIS.h"
#if _ZE_XESS_ENABLED
#	include "RenderPass/UpscaleXeSS.h"
#endif
#include "RenderPass/VerticalBlur.h"
#include "RenderPass/Wireframe.h"
#include "RenderPass/XeGTAO.h"