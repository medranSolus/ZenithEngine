#pragma once
#include "GFX/Pipeline/PassDesc.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_dof.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::DepthOfField
{
	struct Resources
	{
		RID Color;
		RID Depth;
	};

	struct ExecuteData final : public PassExecuteData
	{
		FfxDofContext Ctx = {};
		UInt2 RenderSize = { 0, 0 };
		float CocLimit = 0.01f;
		U32 RingsCount = 10;
		std::bitset<3> Flags = 1;
		float F_Stop = 2.8f;
		float Focus = 1.0f;
		U32 FocalLength = 85;

		constexpr bool UpdateSettings() const noexcept { return Flags[0]; }
		constexpr bool DisableRingMerge() const noexcept { return Flags[1]; }
		constexpr bool IsInitialized() const noexcept { return Flags[2]; }

		constexpr void SetUpdateSettings(bool val) noexcept { Flags[0] = val; }
		constexpr void SetDisableRingMerge(bool val) noexcept { Flags[1] = val; }
		constexpr void SetInitialized(bool val) noexcept { Flags[2] = val; }

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData();
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledDOF(); }

	PassDesc GetDesc() noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}