#include "GFX/Pipeline/RenderPass/DepthOfField.h"
#include "GFX/External/Error.h"
#include "GFX/External/FfxBackendInterface.h"
#include "Data/Camera.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::DepthOfField
{
	ExecuteData::~ExecuteData()
	{
		if (IsInitialized())
		{
			ZE_FFX_CHECK(ffxDofContextDestroy(&Ctx), "Error destroying DOF context!");
		}
	}

	static Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, PassExecuteData* passData, const std::vector<PixelFormat>& formats) noexcept
	{
		return Update(dev, buildData, *static_cast<ExecuteData*>(passData));
	}

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, PassInitData* initData) noexcept
	{
		return Initialize(dev, buildData);
	}

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::DepthOfField) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept
	{
		if (passData.RenderSize != Settings::RenderSize || buildData.FrameBufferUpdatePending || passData.UpdateSettings())
		{
			if (passData.IsInitialized())
			{
				ZE_FFX_LOG_RET_FAILED_EXPECT(ffxDofContextDestroy(&passData.Ctx), "Error destroying DOF context!");
				passData.SetInitialized(false);
			}
			passData.RenderSize = Settings::RenderSize;
			passData.SetUpdateSettings(false);

			FfxDofContextDescription dofDesc = {};
			dofDesc.flags = FFX_DOF_REVERSE_DEPTH | FFX_DOF_OUTPUT_PRE_INIT | (passData.DisableRingMerge() ? FFX_DOF_DISABLE_RING_MERGE : 0);
			dofDesc.quality = passData.RingsCount;
			dofDesc.resolution = { passData.RenderSize.X, passData.RenderSize.Y };
			dofDesc.backendInterface = buildData.FfxInterface;
			dofDesc.cocLimitFactor = passData.CocLimit;
			ZE_FFX_LOG_RET_FAILED_EXPECT(ffxDofContextCreate(&passData.Ctx, &dofDesc), "Error creating DOF context!");
			passData.SetInitialized(true);

			return UpdateOperation::InternalOnly;
		}
		return UpdateOperation::NoUpdate;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();

		auto operation = Update(dev, buildData, *passData);
		if (!operation)
			return std::unexpected(operation.error());
		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_PERF_GUARD("DepthOfField");

		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

		ZE_DRAW_TAG_BEGIN(dev, cl, "DepthOfField", Pixel(0x89, 0xCF, 0xF0));

		FfxDofDispatchDescription desc = {};
		desc.commandList = External::FFX::GetCommandList(cl);
		desc.color = External::FFX::GetResource(renderData.Buffers, ids.Color, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.depth = External::FFX::GetResource(renderData.Buffers, ids.Depth, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.output = desc.color;

		const float viewFocalLenght = static_cast<float>(data.FocalLength) / 1000.0f;
		const float aperture = viewFocalLenght / (2.0f * data.F_Stop);

		const auto& proj = Settings::DataBank.GetWorldData().get<Data::Camera>(renderData.GraphData.CurrentCamera).Projection;
		const float sensorSize = 2.0f * viewFocalLenght * std::tan(proj.FOV * 0.5f) * proj.ViewRatio;
		const float conversion = static_cast<float>(renderData.Buffers.GetDimmensions(ids.Color).X) / data.SensorSize;

		desc.cocScale = ffxDofCalculateCocScale(aperture, data.Focus, viewFocalLenght, conversion,
			renderData.GraphData.Projection._33, renderData.GraphData.Projection._34, renderData.GraphData.Projection._43);
		desc.cocBias = ffxDofCalculateCocBias(aperture, data.Focus, viewFocalLenght, conversion,
			renderData.GraphData.Projection._33, renderData.GraphData.Projection._34, renderData.GraphData.Projection._43);
		ZE_FFX_LOG_RET_FAILED_EXPECT(ffxDofContextDispatch(&data.Ctx, &desc), "Error performing DOF!");

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("Depth of Field"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);

			ImGui::Text("Version " ZE_STRINGIFY_VERSION(ZE_DEPAREN(FFX_DOF_VERSION_MAJOR), ZE_DEPAREN(FFX_DOF_VERSION_MINOR), ZE_DEPAREN(FFX_DOF_VERSION_PATCH)));

			bool update = false;
			ImGui::Columns(2, "##dof_options", false);
			{
				ImGui::Text("F-stop");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.1f, FLT_MAX, execData.F_Stop,
					ImGui::InputFloat("##dof_fstop", &execData.F_Stop, 0.1f, 1.0f, "%.1f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Aperture f-stop number of desired camera");

				ImGui::Text("Focus distance");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.01f, FLT_MAX, execData.Focus,
					ImGui::InputFloat("##dof_focus", &execData.Focus, 0.1f, 10.0f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance to the focused area");

				ImGui::Text("Focal length");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(1U, UINT32_MAX, execData.FocalLength,
					ImGui::InputInt("##dof_focal", reinterpret_cast<int*>(&execData.FocalLength), 1, 10));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Focal length of the desired camera [mm]");
			}
			ImGui::NextColumn();
			{
				ImGui::Text("CoC Limit");
				ImGui::SetNextItemWidth(-1.0f);
				update |= GUI::InputClamp(0.01f, FLT_MAX, execData.CocLimit,
					ImGui::InputFloat("##dof_coc_limt", &execData.CocLimit, 0.01f, 1.0f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The limit to apply to circle of confusion size as a factor for resolution height");

				ImGui::Text("Quality");
				ImGui::SetNextItemWidth(-1.0f);
				update |= GUI::InputClamp(1U, 50U, execData.RingsCount,
					ImGui::InputInt("##dof_ring_count", reinterpret_cast<int*>(&execData.RingsCount), 1, 10));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The number of rings to be used in the DoF blur kernel");

				ImGui::NewLine();
				ImGui::SetNextItemWidth(-1.0f);
				bool disableMerge = execData.DisableRingMerge();
				update |= ImGui::Checkbox("Disable ring merge##dof_ring_merge", &disableMerge);
				execData.SetDisableRingMerge(disableMerge);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Disable merging kernel rings");
				execData.SetUpdateSettings(update);
			}
			ImGui::Columns(1);

			ImGui::NewLine();
		}
	}
}