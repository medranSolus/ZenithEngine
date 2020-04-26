#include "BasicObject.h"

namespace GFX
{
	void BasicObject::Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle) noexcept
	{
		DirectX::XMStoreFloat3(&pos, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&delta)));
		DirectX::XMStoreFloat3(&angle,
			DirectX::XMVectorModAngles(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&angle),
				DirectX::XMLoadFloat3(&deltaAngle))));
	}

	void BasicObject::ShowWindow(Graphics& gfx) noexcept
	{
		static constexpr float f32Max = FLT_MAX;
		static constexpr float f32Min = -FLT_MAX;
		static constexpr float scaleMin = 0.01f;
		ImGui::DragScalar("Scale", ImGuiDataType_Float, &scale, scaleMin, &scaleMin, &f32Max, "%.2f");
		ImGui::Text("Position:");
		ImGui::DragScalar("X", ImGuiDataType_Float, &pos.x, 0.01f, &f32Min, &f32Max, "%.2f");
		ImGui::DragScalar("Y", ImGuiDataType_Float, &pos.y, 0.01f, &f32Min, &f32Max, "%.2f");
		ImGui::DragScalar("Z", ImGuiDataType_Float, &pos.z, 0.01f, &f32Min, &f32Max, "%.2f");
		ImGui::Text("Rotation:");
		ImGui::SliderAngle("X##Rot", &angle.x, 0.0f, 360.0f, "%.2f");
		ImGui::SliderAngle("Y##Rot", &angle.y, 0.0f, 360.0f, "%.2f");
		ImGui::SliderAngle("Z##Rot", &angle.z, 0.0f, 360.0f, "%.2f");
	}
}