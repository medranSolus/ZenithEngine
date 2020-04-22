#include "PointLight.h"
#include "Math.h"
#include "ImGui/imgui.h"

namespace GFX::Light
{
	PointLight::PointLight(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, float radius)
		: mesh(gfx, position, name, { 1.0f, 1.0f, 1.0f }, 3, 3, radius, radius, radius), buffer(gfx, name)
	{
		lightBuffer =
		{
			{ 0.05f, 0.05f, 0.05f },
			1.0f,
			{ 1.0f, 1.0f, 1.0f },
			0.045f,
			mesh.GetPos(),
			0.0075f,
			1.0f
		};
	}

	void PointLight::ShowWindow(Graphics& gfx) noexcept
	{
		static constexpr float f32Max = FLT_MAX;
		static constexpr float f32Min = -FLT_MAX;
		ImGui::ColorEdit3("Color", (float*)&lightBuffer.lightColor,
			ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
		if (mesh.IsVisible())
			mesh.GetMaterial().Update(gfx, { lightBuffer.lightColor });
		ImGui::DragScalar("Intensity", ImGuiDataType_Float, &lightBuffer.lightIntensity, 0.001f, &f32Min, &f32Max, "%.3f");
		ImGui::ColorEdit3("Ambient Color", (float*)&lightBuffer.ambientColor,
			ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
		ImGui::Text("Attenuation:");
		ImGui::DragScalar("Const", ImGuiDataType_Float, &lightBuffer.atteuationConst, 0.001f, &f32Min, &f32Max, "%.3f");
		ImGui::DragScalar("Linear", ImGuiDataType_Float, &lightBuffer.atteuationLinear, 0.0001f, &f32Min, &f32Max, "%.4f");
		ImGui::DragScalar("Quad", ImGuiDataType_Float, &lightBuffer.attenuationQuad, 0.00001f, &f32Min, &f32Max, "%.5f");
		ImGui::NewLine();
		mesh.ShowWindow(gfx);
	}

	void PointLight::Bind(Graphics& gfx, const Camera::ICamera& camera) const noexcept
	{
		DirectX::XMStoreFloat3(&lightBuffer.lightPos, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&mesh.GetPos()), camera.GetView()));
		buffer.Update(gfx, lightBuffer);
		buffer.Bind(gfx);
	}
}