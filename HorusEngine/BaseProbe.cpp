#include "BaseProbe.h"
#include "Technique.h"

namespace GFX::Probe
{
	void BaseProbe::SetTechnique(Pipeline::Technique* currentTechnique) noexcept
	{
		technique = currentTechnique;
		ImGui::BeginChild(("##" + technique->GetName()).c_str());
		ImGui::TextColored({ 0.4f, 1.0f, 0.6f, 1.0f }, ("--Technique " + technique->GetName() + "--").c_str());
		ImGui::Checkbox("Active", &technique->IsActive());
	}

	void BaseProbe::ReleaseTechnique() noexcept
	{
		technique = nullptr;
		ImGui::EndChild();
	}

	bool BaseProbe::Visit(Data::CBuffer::DynamicCBuffer& buffer) noexcept(!IS_DEBUG)
	{
		bool dirty = false;
		dirty |= VisitObject(buffer);
		dirty |= VisitMaterial(buffer);
		return dirty;
	}

	bool BaseProbe::VisitObject(Data::CBuffer::DynamicCBuffer& buffer) noexcept(!IS_DEBUG)
	{
		bool dirty = false;
		if (auto scale = buffer["scale"]; scale.Exists())
		{
			dirty |= ImGui::DragFloat("Scale", &scale, 0.01f, 0.01f, FLT_MAX, "%.2f");
		}
		if (auto position = buffer["position"]; position.Exists())
		{
			ImGui::Text("Position: (X,Y,Z)");
			dirty |= ImGui::DragFloat3("##Position", reinterpret_cast<float*>(&static_cast<DirectX::XMFLOAT3&>(position)), 0.01f, -FLT_MAX, FLT_MAX, "%.2f");
		}
		if (auto angle = buffer["angle"]; angle.Exists())
		{
			DirectX::XMFLOAT3& rotation = static_cast<DirectX::XMFLOAT3&>(angle);
			ImGui::Text("Rotation:");
			dirty |= ImGui::SliderAngle("X##Rotation", &rotation.x, 0.0f, 360.0f, "%.2f");
			dirty |= ImGui::SliderAngle("Y##Rotation", &rotation.y, 0.0f, 360.0f, "%.2f");
			dirty |= ImGui::SliderAngle("Z##Rotation", &rotation.z, 0.0f, 360.0f, "%.2f");
		}
		return dirty;
	}

	bool BaseProbe::VisitMaterial(Data::CBuffer::DynamicCBuffer& buffer) noexcept(!IS_DEBUG)
	{
		bool dirty = false;
		if (auto color = buffer["materialColor"]; color.Exists())
		{
			dirty |= ImGui::ColorEdit4("Material color", reinterpret_cast<float*>(&static_cast<Data::ColorFloat4&>(color)),
				ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
		}
		if (auto color = buffer["specularColor"]; color.Exists())
		{
			dirty |= ImGui::ColorEdit3("Specular color", reinterpret_cast<float*>(&static_cast<Data::ColorFloat3&>(color)), ImGuiColorEditFlags_Float);
		}
		if (auto intensity = buffer["specularIntensity"]; intensity.Exists())
		{
			dirty |= ImGui::DragFloat("Specular intensity", (float*)&intensity, 0.01f, 0.0f, FLT_MAX, "%.2f");
		}
		if (auto useSpecularAlpha = buffer["useSpecularPowerAlpha"]; useSpecularAlpha.Exists())
		{
			dirty |= ImGui::Checkbox("Use map alpha as specular power", (bool*)&useSpecularAlpha);
		}
		if (auto power = buffer["specularPower"]; power.Exists())
		{
			dirty |= ImGui::DragFloat("Specular power", (float*)&power, 0.01f, 0.0f, FLT_MAX, "%.2f");
		}
		if (auto weight = buffer["normalMapWeight"]; weight.Exists())
		{
			dirty |= ImGui::DragFloat("Normal map weight", (float*)&weight, 0.01f, 0.0f, FLT_MAX, "%.2f");
		}
		return dirty;
	}

	bool BaseProbe::VisitShape(Shape::BaseShape& shape) noexcept
	{
		// Mesh
		return false;
	}
}