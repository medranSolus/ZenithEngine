#include "PointLight.h"
#include "Math.h"
#include "ImGui/imgui.h"

namespace GFX::Light
{
	PointLight::PointLight(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, float radius)
		: mesh(gfx, position, name, { 1.0f, 1.0f, 1.0f }, 3, 3, radius, radius, radius), buffer(gfx)
	{
		lightBuffer =
		{
			{ 0.05f, 0.05f, 0.05f, 1.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			mesh.GetPos(),
			1.0f,
			1.0f,
			0.045f,
			0.0075f
		};
	}

	void PointLight::ShowWindow() noexcept
	{
		static const float f32Max = FLT_MAX;
		static const float f32Min = -FLT_MAX;
		ImGui::ColorEdit4("Color", (float*)&lightBuffer.diffuseColor,
			ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
		ImGui::DragScalar("Intensity", ImGuiDataType_Float, &lightBuffer.diffuseIntensity, 0.001f, &f32Min, &f32Max, "%.3f");
		ImGui::ColorEdit4("Ambient Color", (float*)&lightBuffer.ambientColor,
			ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
		ImGui::Text("Position:");
		ImGui::DragScalar("X", ImGuiDataType_Float, &mesh.Pos()->x, 0.01f, &f32Min, &f32Max, "%.2f");
		ImGui::DragScalar("Y", ImGuiDataType_Float, &mesh.Pos()->y, 0.01f, &f32Min, &f32Max, "%.2f");
		ImGui::DragScalar("Z", ImGuiDataType_Float, &mesh.Pos()->z, 0.01f, &f32Min, &f32Max, "%.2f");
		ImGui::Text("Attenuation:");
		ImGui::DragScalar("Const", ImGuiDataType_Float, &lightBuffer.atteuationConst, 0.001f, &f32Min, &f32Max, "%.3f");
		ImGui::DragScalar("Linear", ImGuiDataType_Float, &lightBuffer.atteuationLinear, 0.0001f, &f32Min, &f32Max, "%.4f");
		ImGui::DragScalar("Quad", ImGuiDataType_Float, &lightBuffer.attenuationQuad, 0.00001f, &f32Min, &f32Max, "%.5f");
	}

	void PointLight::Bind(Graphics & gfx, const Camera & camera) const noexcept
	{
		DirectX::XMStoreFloat3(&lightBuffer.pos, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&mesh.GetPos()), camera.GetView()));
		buffer.Update(gfx, lightBuffer);
		buffer.Bind(gfx);
	}
}
