#include "GFX/Pipeline/PassDesc.h"

namespace ZE::GFX::Pipeline
{
	PassDesc& PassDesc::operator=(const PassDesc& desc) noexcept
	{
		Type = desc.Type;
		if (desc.InitData)
			InitData = desc.InitData->Clone();
		else
			InitData = nullptr;
		InitializeFormats = desc.InitializeFormats;
		Init = desc.Init;
		Prepare = desc.Prepare;
		Evaluate = desc.Evaluate;
		Execute = desc.Execute;
		Update = desc.Update;
		DebugUI = desc.DebugUI;
		return *this;
	}
}