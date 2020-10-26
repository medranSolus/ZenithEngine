#include "BaseProbe.h"
#include "BaseShape.h"
#include "Math.h"

#define Tag(label) MakeTag(label).c_str()

namespace GFX::Probe
{
	inline std::string BaseProbe::MakeTag(const std::string& label) const noexcept
	{
		return label + "##" + (technique == nullptr ? "None" : technique->GetName());
	}

	void BaseProbe::SetTechnique(Pipeline::Technique* currentTechnique) noexcept
	{
		technique = currentTechnique;
		ImGui::TextColored({ 0.4f, 1.0f, 0.6f, 1.0f }, ("--Technique " + technique->GetName() + "--").c_str());
		ImGui::Checkbox(Tag("Active"), &technique->IsActive());
	}

	bool BaseProbe::Visit(Data::CBuffer::DynamicCBuffer& buffer) const noexcept
	{
		bool dirty = false;
		dirty |= VisitObject(buffer);
		dirty |= VisitMaterial(buffer);
		dirty |= VisitLight(buffer);
		return dirty;
	}

	bool BaseProbe::VisitObject(Data::CBuffer::DynamicCBuffer& buffer) const noexcept
	{
		bool dirty = false;
		if (auto offset = buffer["offset"]; offset.Exists())
		{
			dirty |= ImGui::DragFloat(Tag("Offset"), &offset, 0.001f, 0.001f, FLT_MAX, "%.3f");
		}
		if (auto scale = buffer["scale"]; scale.Exists())
		{
			dirty |= ImGui::DragFloat(Tag("Scale"), &scale, 0.01f, 0.01f, FLT_MAX, "%.2f");
		}
		if (auto position = buffer["position"]; position.Exists())
		{
			ImGui::Text("Position: [X|Y|Z]");
			dirty |= ImGui::DragFloat3(Tag("##Position"), reinterpret_cast<float*>(&static_cast<DirectX::XMFLOAT3&>(position)), 0.01f, -FLT_MAX, FLT_MAX, "%.2f");
		}
		if (auto angle = buffer["angle"]; angle.Exists())
		{
			DirectX::XMFLOAT3& rotation = static_cast<DirectX::XMFLOAT3&>(angle);
			ImGui::Text("Rotation:");
			dirty |= ImGui::SliderAngle(Tag("X##Rotation"), &rotation.x, 0.0f, 360.0f, "%.2f");
			dirty |= ImGui::SliderAngle(Tag("Y##Rotation"), &rotation.y, 0.0f, 360.0f, "%.2f");
			dirty |= ImGui::SliderAngle(Tag("Z##Rotation"), &rotation.z, 0.0f, 360.0f, "%.2f");
		}
		return dirty;
	}

	bool BaseProbe::VisitMaterial(Data::CBuffer::DynamicCBuffer& buffer) const noexcept
	{
		bool dirty = false;
		if (auto color = buffer["solidColor"]; color.Exists())
		{
			dirty |= ImGui::ColorEdit3(Tag("Color##Solid"), reinterpret_cast<float*>(&static_cast<Data::ColorFloat3&>(color)), ImGuiColorEditFlags_Float);
		}
		if (auto color = buffer["materialColor"]; color.Exists())
		{
			dirty |= ImGui::ColorEdit4(Tag("Material color"), reinterpret_cast<float*>(&static_cast<Data::ColorFloat4&>(color)),
				ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
		}
		if (auto color = buffer["specularColor"]; color.Exists())
		{
			dirty |= ImGui::ColorEdit3(Tag("Specular color"), reinterpret_cast<float*>(&static_cast<Data::ColorFloat3&>(color)), ImGuiColorEditFlags_Float);
		}
		if (auto intensity = buffer["specularIntensity"]; intensity.Exists())
		{
			dirty |= ImGui::DragFloat(Tag("Specular intensity"), &intensity, 0.01f, 0.0f, FLT_MAX, "%.2f");
		}
		if (auto useSpecularAlpha = buffer["useSpecularPowerAlpha"]; useSpecularAlpha.Exists())
		{
			dirty |= ImGui::Checkbox(Tag("Use map alpha as specular power"), &useSpecularAlpha);
		}
		if (auto power = buffer["specularPower"]; power.Exists())
		{
			dirty |= ImGui::DragFloat(Tag("Specular power"), &power, 0.001f, 0.0f, 1.0f, "%.3f");
		}
		if (auto parallax = buffer["parallaxScale"]; parallax.Exists())
		{
			dirty |= ImGui::DragFloat(Tag("Bump scaling"), &parallax, 0.01f, 0.0f, FLT_MAX, "%.2f");
		}
		return dirty;
	}

	bool BaseProbe::VisitLight(Data::CBuffer::DynamicCBuffer& buffer) const noexcept
	{
		bool dirty = false;
		if (auto lightColor = buffer["lightColor"]; lightColor.Exists())
		{
			ImGui::Text("Color:");
			dirty |= ImGui::ColorEdit3(Tag("Light"), reinterpret_cast<float*>(&static_cast<Data::ColorFloat3&>(lightColor)),
				ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
		}
		if (auto shadowColor = buffer["shadowColor"]; shadowColor.Exists())
		{
			dirty |= ImGui::ColorEdit3(Tag("Shadow"), reinterpret_cast<float*>(&static_cast<Data::ColorFloat3&>(shadowColor)),
				ImGuiColorEditFlags_Float);
		}
		if (auto lightIntensity = buffer["lightIntensity"]; lightIntensity.Exists())
		{
			dirty |= ImGui::DragFloat(Tag("Intensity"), &lightIntensity, 0.001f, 0.0f, FLT_MAX, "%.3f");
		}
		if (auto direction = buffer["direction"]; direction.Exists())
		{
			ImGui::Text("Direction [X|Y|Z]");
			if (ImGui::DragFloat3(Tag("##Direction"), reinterpret_cast<float*>(&static_cast<DirectX::XMFLOAT3&>(direction)), 0.01f, -1.0f, 1.0f, "%.2f"))
			{
				Math::NormalizeStore(direction);
				dirty = true;
			}
		}
		if (auto outerAngle = buffer["outerAngle"]; outerAngle.Exists())
		{
			dirty |= ImGui::SliderAngle(Tag("Outer angle"), &outerAngle, 0.0f, 45.0f, "%.2f");
			if (auto innerAngle = buffer["innerAngle"]; innerAngle.Exists())
			{
				if (static_cast<float>(innerAngle) > static_cast<float>(outerAngle))
				{
					innerAngle = static_cast<float>(outerAngle);
					if (static_cast<float>(innerAngle) < 0.0f)
						innerAngle = 0.0f;
					buffer["innerAngle"] = innerAngle;
					dirty = true;
				}
				dirty |= ImGui::SliderAngle(Tag("Inner angle"), &innerAngle, 0.0f, static_cast<float>(outerAngle) * 180.0f / static_cast<float>(M_PI) - 0.5f, "%.2f");
			}
		}
		if (auto falloff = buffer["falloff"]; falloff.Exists())
		{
			dirty |= ImGui::DragFloat(Tag("Falloff"), &falloff, 0.01f, -FLT_MAX, FLT_MAX, "%.2f");
		}
		return dirty;
	}

	void BaseProbe::VisitShape(Graphics& gfx, Shape::BaseShape& shape) const noexcept
	{
		bool meshOnly = shape.IsMesh();
		if (ImGui::Checkbox(Tag("Mesh-only"), &meshOnly))
		{
			if (meshOnly)
				shape.SetTopologyMesh(gfx);
			else
				shape.SetTopologyPlain(gfx);
		}
		bool ouline = shape.IsOutline();
		ImGui::SameLine(); ;
		if (ImGui::Checkbox(Tag("Outline"), &ouline))
		{
			if (ouline)
				shape.SetOutline();
			else
				shape.DisableOutline();
		}
	}

	bool BaseProbe::VisitCamera(Camera::ProjectionData& projection) const noexcept
	{
		return ImGui::SliderAngle("FOV", &projection.fov, 1.0f, 179.0f, "%.1f") ||
			ImGui::DragFloat("Near clip", &projection.nearClip, 0.01f, 0.01f, 10.0f, "%.3f") ||
			ImGui::DragFloat("Far clip", &projection.farClip, 0.1f, projection.nearClip + 0.01f, 50000.0f, "%.1f") ||
			ImGui::SliderFloat("Ratio", &projection.screenRatio, 0.1f, 5.0f, "%.2f");
	}
}