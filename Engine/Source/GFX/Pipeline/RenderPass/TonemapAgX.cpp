#include "GFX/Pipeline/RenderPass/TonemapAgX.h"
#include "GFX/Resource/Constant.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapAgX
{
	static Expected<std::unique_ptr<PassExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapAgX initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::TonemapAgX) };
		desc.InitializeFormats.emplace_back(outputFormat);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<std::unique_ptr<ExecuteData>>  Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat) noexcept
	{
		auto passData = std::make_unique<ExecuteData>();

		Binding::SchemaDesc desc = {};
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Frame
		desc.AddRange({ sizeof(Float4), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AppendSamplers(buildData.Samplers);
		ZE_EXPECT_RET_FAILED(passData->BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		Resource::PipelineStateDesc psoDesc = {};
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache));
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.PS, "TonemapPS_X", buildData.ShaderCache));
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		ZE_PSO_SET_NAME(psoDesc, "TonemapAgX");

		ZE_EXPECT_RET_FAILED(passData->State, Resource::PipelineStateGfx::Create(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex)));

		return passData;
	}

	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_PERF_GUARD("TonemapAgX");
		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

		ZE_DRAW_TAG_BEGIN(dev, cl, "TonemapAgX", PixelVal::Cobalt);
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		Resource::Constant<Float4> params;
		ZE_EXPECT_RET_FAILED_CODE(params, Resource::Constant<Float4>::Create(dev, data.Params));
		params.Bind(cl, ctx);
		cl.DrawFullscreen(dev);
		renderData.Buffers.EndRaster(cl);

		ZE_DRAW_TAG_END(dev, cl);
		return {};
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("AgX Tonemapper"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);

			ImGui::Columns(2, "##tonemap_params_agx", false);
			{
				ImGui::Text("Exposure value");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.01f, FLT_MAX, execData.Params.x,
					ImGui::InputFloat("##exposure_value", &execData.Params.x, 0.1f, 0.0f, "%.2f"));

				ImGui::Text("Contrast");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##agx_contrast", &execData.Params.z, 0.01f, 0.1f, "%.2f");
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Saturation");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##agx_saturation", &execData.Params.y, 0.01f, 0.1f, "%.2f");

				ImGui::Text("Mid constrast");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##agx_mid_constrast", &execData.Params.w, 0.01f, 0.1f, "%.2f");
			}
			ImGui::Columns(1);
			ImGui::NewLine();
		}
	}
}