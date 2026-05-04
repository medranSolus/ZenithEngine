#include "GFX/Pipeline/RenderPass/TonemapReinhard.h"
#include "GFX/Resource/Constant.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapReinhard
{
	static Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, PassExecuteData* passData, const std::vector<PixelFormat>& formats) noexcept
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapReinhard initialization formats!");
		return Update(dev, buildData, *static_cast<ExecuteData*>(passData), formats.front());
	}

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapReinhard initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	static std::string GetPsoName(TonemapperType tonemapper) noexcept
	{
		std::string base = "TonemapPS_";
		switch (tonemapper)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case TonemapperType::Reinhard:
			base += 'R';
			break;
		case TonemapperType::ReinhardLuma:
			base += "RL";
			break;
		case TonemapperType::ReinhardLumaJodie:
			base += "RJ";
			break;
		}
		return base;
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::TonemapReinhard) };
		desc.InitializeFormats.emplace_back(outputFormat);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat) noexcept
	{
		if (passData.CurrentTonemapper != Settings::Tonemapper)
		{
			passData.CurrentTonemapper = Settings::Tonemapper;

			Resource::PipelineStateDesc psoDesc = {};
			ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache));
			ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.PS, GetPsoName(passData.CurrentTonemapper), buildData.ShaderCache));
			psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
			psoDesc.Culling = Resource::CullMode::Back;
			psoDesc.RenderTargetsCount = 1;
			psoDesc.FormatsRT[0] = outputFormat;
			ZE_PSO_SET_NAME(psoDesc, GetPsoName(passData.CurrentTonemapper));

			ZE_EXPECT_RET_FAILED(passData.State, Resource::PipelineStateGfx::Create(dev, psoDesc, buildData.BindingLib.GetSchema(passData.BindingIndex)));

			return UpdateOperation::InternalOnly;
		}
		return UpdateOperation::NoUpdate;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Frame
		desc.AddRange({ sizeof(Float2), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AppendSamplers(buildData.Samplers);
		ZE_EXPECT_RET_FAILED(passData->BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		passData->CurrentTonemapper = TonemapperType::None;
		auto operation = Update(dev, buildData, *passData, outputFormat);
		if (!operation)
			return std::unexpected(operation.error());
		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_PERF_GUARD("TonemapReinhard");
		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

		ZE_DRAW_TAG_BEGIN(dev, cl, "TonemapReinhard", PixelVal::Cobalt);
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		Resource::Constant<Float2> params;
		ZE_EXPECT_RET_FAILED(params, Resource::Constant<Float2>::Create(dev, data.Params));
		params.Bind(cl, ctx);
		cl.DrawFullscreen(dev);
		renderData.Buffers.EndRaster(cl);

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("Reinhard Tonemappers"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);

			ImGui::Columns(2, "##tonemap_params_reinhard", false);
			{
				ImGui::Text("Exposure value");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.01f, FLT_MAX, execData.Params.x,
					ImGui::InputFloat("##exposure_value", &execData.Params.x, 0.1f, 0.0f, "%.2f"));
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Reinhard offset");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.01f, FLT_MAX, execData.Params.y,
					ImGui::InputFloat("##reihnard_offset", &execData.Params.y, 0.01f, 0.1f, "%.2f"));
			}
			ImGui::Columns(1);
			ImGui::NewLine();
		}
	}
}