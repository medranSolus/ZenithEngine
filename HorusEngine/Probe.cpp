#include "Probe.h"
#include "Technique.h"
#include "BasicObject.h"

namespace GFX
{
	void Probe::SetTechnique(Pipeline::Technique* currentTechnique) noexcept
	{
		technique = currentTechnique;
		ImGui::BeginChild(("##" + technique->GetName()).c_str());
		ImGui::TextColored({ 0.4f, 1.0f, 0.6f, 1.0f }, ("--Technique " + technique->GetName() + "--").c_str());
		ImGui::Checkbox("Active", &technique->IsActive());
	}

	void Probe::Release() noexcept
	{
		technique = nullptr;
		ImGui::EndChild();
	}

	bool Probe::VisitBuffer(Data::CBuffer::DynamicCBuffer& buffer) noexcept(!IS_DEBUG)
	{
		bool dirty = false;

		// Material variables
		if (auto color = buffer["materialColor"]; color.Exists())
		{
			dirty |= ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&static_cast<Data::ColorFloat4&>(color)),
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

	bool Probe::VisitObject(BasicObject& object) noexcept
	{
		bool dirty = false;
		dirty |= ImGui::DragFloat("Scale", &object.GetScale(), 0.01f, 0.01f, FLT_MAX, "%.2f");
		dirty |= ImGui::DragFloat3("Position", (float*)&object.GetPos(), 0.01f, -FLT_MAX, FLT_MAX, "%.2f");
		ImGui::Text("Rotation:");
		auto& angle = object.GetAngle();
		dirty |= ImGui::SliderAngle("X##Rotation", &angle.x, 0.0f, 360.0f, "%.2f");
		dirty |= ImGui::SliderAngle("Y##Rotation", &angle.y, 0.0f, 360.0f, "%.2f");
		dirty |= ImGui::SliderAngle("Z##Rotation", &angle.z, 0.0f, 360.0f, "%.2f");
		return dirty;
	}

	bool Probe::VisitShape(Shape::BaseShape& shape) noexcept
	{
		// Mesh
		return false;
	}
}