#include "GFX/Pipeline/RenderPass/DebugView.h"
#include "GFX/Resource/Constant.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::DebugView
{
	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for DebugView initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::DebugView) };
		desc.InitializeFormats.emplace_back(outputFormat);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.DebugUI = DebugUI;
		return desc;
	}

	ExpectedPassExecuteData  Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();

		Binding::SchemaDesc desc = {};
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Frame
		desc.AddRange({ sizeof(U32), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Mode
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		ZE_EXPECT_RET_FAILED(passData->BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		Resource::PipelineStateDesc psoDesc = {};
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache));
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.PS, "DebugViewPS", buildData.ShaderCache));
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		ZE_PSO_SET_NAME(psoDesc, "DebugView");

		ZE_EXPECT_RET_FAILED(passData->State, Resource::PipelineStateGfx::Create(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex)));

		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

		RID output = INVALID_RID;
		switch (data.ViewMode)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case Mode::None:
			break;
		case Mode::Depth:
			output = ids.Depth;
			break;
		case Mode::Normals:
			output = ids.Normals;
			break;
		case Mode::Albedo:
			output = ids.Albedo;
			break;
		case Mode::Metalness:
		case Mode::Roughness:
			output = ids.MaterialParams;
			break;
		case Mode::Motion:
			output = ids.MotionVectors;
			break;
		case Mode::Reactive:
			output = ids.ReactiveMask;
			break;
		case Mode::DirectLight:
			output = ids.DirectLighting;
			break;
		case Mode::SSAO:
			output = ids.SSAO;
			break;
		case Mode::SSR:
			output = ids.SSR;
			break;
		case Mode::RenderedScene:
			output = ids.RenderedScene;
			break;
		case Mode::UpscaledScene:
			output = ids.UpscaledScene;
			break;
		case Mode::Outline:
			output = ids.Outline;
			break;
		}
		if (output == INVALID_RID)
			return false;

		ZE_PERF_GUARD("DebugView");

		ZE_DRAW_TAG_BEGIN(dev, cl, "DebugView", PixelVal::White);
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, output);
		Resource::Constant<U32> params;
		ZE_EXPECT_RET_FAILED(params, Resource::Constant<U32>::Create(dev, static_cast<U32>(data.ViewMode)));
		params.Bind(cl, ctx);
		renderData.SettingsBuffer.Bind(cl, ctx);
		cl.DrawFullscreen(dev);
		renderData.Buffers.EndRaster(cl);

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("Debug View"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);

			constexpr std::array<const char*, 14> VIEW = { "Present frame", "Depth", "Normals", "Albedo",
				"Metalness", "Roughness", "Motion Vectors", "Reactive mask", "Direct lighting", "SSAO",
				"SSR", "Rendered scene", "Upscaled scene", "Outline" };
			if (ImGui::BeginCombo("Output mode##debug_view", VIEW.at(static_cast<U8>(execData.ViewMode))))
			{
				for (Mode i = Mode::None; const char* item : VIEW)
				{
					const bool selected = i == execData.ViewMode;
					if (ImGui::Selectable(item, selected))
						execData.ViewMode = i;
					if (selected)
						ImGui::SetItemDefaultFocus();
					i = static_cast<Mode>(static_cast<U8>(i) + 1);
				}
				ImGui::EndCombo();
			}

			ImGui::NewLine();
		}
	}
}