#include "GFX/Probe/BaseProbe.h"
#include "GFX/Shape/BaseShape.h"

#define Tag(label) MakeTag(label).c_str()

namespace GFX::Probe
{
	std::string BaseProbe::MakeTag(std::string&& label) const noexcept
	{
		return label + "##" + (technique == nullptr ? "None" : technique->GetName());
	}

	void BaseProbe::SetTechnique(Pipeline::Technique* currentTechnique) noexcept
	{
		technique = currentTechnique;
		auto& style = ImGui::GetStyle();
		auto colorMark = style.Colors[ImGuiCol_CheckMark];
		auto colorText = style.Colors[ImGuiCol_Text];
		style.Colors[ImGuiCol_CheckMark] = style.Colors[ImGuiCol_Text] = { 0.4f, 1.0f, 0.6f, 1.0f };
		ImGui::Checkbox(Tag("-- " + technique->GetName() + " --"), &technique->Active());
		style.Colors[ImGuiCol_CheckMark] = colorMark;
		style.Colors[ImGuiCol_Text] = colorText;
	}

	bool BaseProbe::Visit(Data::CBuffer::DynamicCBuffer& buffer) noexcept
	{
		bool dirty = false;
		dirty |= VisitLight(buffer);
		dirty |= VisitObject(buffer);
		dirty |= VisitMaterial(buffer);
		return dirty;
	}

	bool BaseProbe::VisitObject(Data::CBuffer::DynamicCBuffer& buffer) noexcept
	{
		constexpr float SLIDER_WIDTH = -15.0f;
		constexpr float HALF_SLIDER_WIDTH = -80.0f;
		constexpr float INPUT_WIDTH = -HALF_SLIDER_WIDTH;
		bool dirty = false;
		//if (auto offset = buffer["offset"]; offset.Exists())
		//{
		//	dirty |= ImGui::DragFloat(Tag("Offset"), &offset, 0.001f, 0.001f, FLT_MAX, "%.3f");
		//}
		if (auto scale = buffer["scale"]; scale.Exists())
		{
			dirty |= ImGui::InputFloat(Tag("Scale"), &scale, 0.01f, 0.0f, "%.3f");
			if (static_cast<float>(scale) < 0.001f)
				scale = 0.001f;
		}
		if (auto position = buffer["position"]; position.Exists())
		{
			ImGui::Columns(2, "##mesh_options", false);
			Float3& pos = static_cast<Float3&>(position);
			ImGui::Text("Position");
			ImGui::SetNextItemWidth(SLIDER_WIDTH);
			dirty |= ImGui::InputFloat(Tag("X##position"), &pos.x, 0.1f, 0.0f, "%.2f");
			ImGui::SetNextItemWidth(SLIDER_WIDTH);
			dirty |= ImGui::InputFloat(Tag("Y##position"), &pos.y, 0.1f, 0.0f, "%.2f");
			ImGui::SetNextItemWidth(SLIDER_WIDTH);
			dirty |= ImGui::InputFloat(Tag("Z##position"), &pos.z, 0.1f, 0.0f, "%.2f");
			ImGui::NextColumn();
		}
		if (auto rotor = buffer["rotation"]; rotor.Exists())
		{
			Float3 rotation = Math::ToDegrees(Math::GetEulerAngles(rotor));
			ImGui::Text("Rotation");
			ImGui::Text("-");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(HALF_SLIDER_WIDTH);
			ImGui::SliderInt(Tag("+ X##rot"), &rotationX, -ROTATION_MAX_SPEED, ROTATION_MAX_SPEED, "", ImGuiSliderFlags_NoInput);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(INPUT_WIDTH);
			bool angleSet = ImGui::InputFloat(Tag("##angle_x"), &rotation.x, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::Text("-");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(HALF_SLIDER_WIDTH);
			ImGui::SliderInt(Tag("+ Y##rot"), &rotationY, -ROTATION_MAX_SPEED, ROTATION_MAX_SPEED, "", ImGuiSliderFlags_NoInput);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(INPUT_WIDTH);
			angleSet |= ImGui::InputFloat(Tag("##angle_y"), &rotation.y, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::Text("-");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(HALF_SLIDER_WIDTH);
			ImGui::SliderInt(Tag("+ Z##rot"), &rotationZ, -ROTATION_MAX_SPEED, ROTATION_MAX_SPEED, "", ImGuiSliderFlags_NoInput);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(INPUT_WIDTH);
			angleSet |= ImGui::InputFloat(Tag("##angle_z"), &rotation.z, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::Columns(1);
			if (angleSet)
			{
				rotationX = rotationY = rotationZ = 0;
				rotation = Math::ToRadians(rotation);
				Math::XMStoreFloat4(&rotor,
					Math::XMQuaternionRotationRollPitchYawFromVector(Math::XMLoadFloat3(&rotation)));
				dirty = true;
			}
			else if (rotationX || rotationY || rotationZ)
			{
				Math::XMStoreFloat4(&rotor,
					Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&rotor),
						Math::XMQuaternionRotationRollPitchYaw(Math::ToRadians(rotationX / 2.0f),
							Math::ToRadians(rotationY / 2.0f), Math::ToRadians(rotationZ / 2.0f)))));
				dirty = true;
			}
		}
		return dirty;
	}

	bool BaseProbe::VisitMaterial(Data::CBuffer::DynamicCBuffer& buffer) const noexcept
	{
		const ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_HDR
			| ImGuiColorEditFlags_Float | (compact ? ImGuiColorEditFlags_NoInputs : 0);
		bool dirty = false;

		if (auto color = buffer["solidColor"]; color.Exists())
		{
			dirty |= ImGui::ColorEdit3(Tag("Color##solid"),
				reinterpret_cast<float*>(&static_cast<ColorF3&>(color)), colorFlags);
		}
		if (auto color = buffer["materialColor"]; color.Exists())
		{
			dirty |= ImGui::ColorEdit4(Tag("Material color"), reinterpret_cast<float*>(&static_cast<ColorF4&>(color)),
				colorFlags | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
		}
		if (auto color = buffer["specularColor"]; color.Exists())
		{
			dirty |= ImGui::ColorEdit3(Tag("Specular color"),
				reinterpret_cast<float*>(&static_cast<ColorF3&>(color)), colorFlags);
		}
		if (!compact)
			ImGui::Columns(2, "##probe_specular", false);
		if (auto intensity = buffer["specularIntensity"]; intensity.Exists())
		{
			if (compact)
			{
				ImGui::Text("Specular intensity");
				ImGui::SetNextItemWidth(-1.0f);
			}
			dirty |= ImGui::InputFloat(Tag(compact ? "##spec_int" : "Specular intensity"), &intensity, 0.1f, 0.0f, "%.2f");
			if (static_cast<float>(intensity) < 0.01f)
				intensity = 0.01f;
			if (!compact)
				ImGui::NextColumn();
		}
		if (auto power = buffer["specularPower"]; power.Exists())
		{
			if (compact)
			{
				ImGui::Text("Specular power");
				ImGui::SetNextItemWidth(-1.0f);
			}
			dirty |= ImGui::InputFloat(Tag(compact ? "##spec_pow" : "Specular power"), &power, 0.001f, 0.0f, "%.3f");
			if (static_cast<float>(power) < 0.001f)
				power = 0.001f;
			else if (static_cast<float>(power) > 1.0f)
				power = 1.0f;
		}
		if (!compact)
			ImGui::Columns(1);
		if (auto useSpecularAlpha = buffer["useSpecularPowerAlpha"]; useSpecularAlpha.Exists())
		{
			dirty |= ImGui::Checkbox(Tag("Use specular map alpha"), &useSpecularAlpha);
		}
		if (auto parallax = buffer["parallaxScale"]; parallax.Exists())
		{
			if (compact)
			{
				ImGui::Text("Bump scaling");
				ImGui::SetNextItemWidth(-1.0f);
			}
			dirty |= ImGui::InputFloat(Tag(compact ? "##bump_scale" : "Bump scaling"), &parallax, 0.01f, 0.0f, "%.2f");
			if (static_cast<float>(parallax) < 0.0f)
				parallax = 0.0f;
		}
		return dirty;
	}

	bool BaseProbe::VisitLight(Data::CBuffer::DynamicCBuffer& buffer) const noexcept
	{
		constexpr ImGuiColorEditFlags COLOR_FLAGS = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float;
		bool dirty = false;
		if (auto lightIntensity = buffer["lightIntensity"]; lightIntensity.Exists())
		{
			dirty |= ImGui::InputFloat(Tag("Intensity"), &lightIntensity, 0.001f, 0.0f, "%.3f");
			if (static_cast<float>(lightIntensity) < 0.0f)
				lightIntensity = 0.0f;
			ImGui::Columns(1);
		}
		if (auto lightColor = buffer["lightColor"]; lightColor.Exists())
		{
			ImGui::Text("Color:");
			dirty |= ImGui::ColorEdit3(Tag("Light"),
				reinterpret_cast<float*>(&static_cast<ColorF3&>(lightColor)), COLOR_FLAGS);
		}
		if (auto shadowColor = buffer["shadowColor"]; shadowColor.Exists())
		{
			dirty |= ImGui::ColorEdit3(Tag("Shadow"),
				reinterpret_cast<float*>(&static_cast<ColorF3&>(shadowColor)), COLOR_FLAGS);
		}
		if (auto direction = buffer["direction"]; direction.Exists())
		{
			ImGui::Text("Direction [X|Y|Z]");
			ImGui::SetNextItemWidth(-5.0f);
			if (ImGui::SliderFloat3(Tag("##Direction"), reinterpret_cast<float*>(&static_cast<Float3&>(direction)), -1.0f, 1.0f, "%.2f"))
			{
				Math::NormalizeStore(direction);
				dirty = true;
			}
		}
		if (auto outerAngle = buffer["outerAngle"]; outerAngle.Exists())
		{
			ImGui::Columns(2, "##probe_spotlight", false);
			ImGui::Text("Outer angle");
			ImGui::SetNextItemWidth(-1.0f);
			dirty |= ImGui::SliderAngle(Tag("##outer"), &outerAngle, 0.0f, 45.0f, "%.2f");
			if (auto innerAngle = buffer["innerAngle"]; innerAngle.Exists())
			{
				if (static_cast<float>(innerAngle) > static_cast<float>(outerAngle))
				{
					innerAngle = static_cast<float>(outerAngle);
					buffer["innerAngle"] = innerAngle;
					dirty = true;
				}
				ImGui::NextColumn();
				ImGui::Text("Inner angle");
				ImGui::SetNextItemWidth(-1.0f);
				dirty |= ImGui::SliderAngle(Tag("##inner"), &innerAngle, 0.0f, Math::ToDegrees(static_cast<float>(outerAngle)), "%.2f");
			}
			ImGui::Columns(1);
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
	}

	bool BaseProbe::VisitCamera(Camera::ProjectionData& projection) const noexcept
	{
		ImGui::Text("FOV");
		ImGui::SetNextItemWidth(-1.0f);
		bool dirty = ImGui::SliderAngle("##fov", &projection.fov, 1.0f, 179.0f, "%.1f");
		ImGui::NextColumn();
		ImGui::Text("Ratio");
		ImGui::SetNextItemWidth(-1.0f);
		dirty |= ImGui::SliderFloat("##screen_ratio", &projection.screenRatio, 0.1f, 5.0f, "%.2f");
		ImGui::Columns(2, "##probe_camera", false);
		ImGui::Text("Near clip");
		ImGui::SetNextItemWidth(-1.0f);
		dirty |= ImGui::InputFloat("##near_clip", &projection.nearClip, 0.01f, 0.0f, "%.3f");
		if (projection.nearClip < 0.01f)
			projection.nearClip = 0.01f;
		else if (projection.nearClip > 10.0f)
			projection.nearClip = 10.0f;
		ImGui::NextColumn();
		ImGui::Text("Far clip");
		ImGui::SetNextItemWidth(-1.0f);
		dirty |= ImGui::InputFloat("##far_clip", &projection.farClip, 0.1f, 0.0f, "%.1f");
		if (projection.farClip < projection.nearClip + 0.01f)
			projection.farClip = projection.nearClip + 0.01f;
		else if (projection.farClip > 50000.0f)
			projection.farClip = 50000.0f;
		return dirty;
	}
}