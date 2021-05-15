#include "GFX/Primitive/CameraFrame.h"

namespace ZE::GFX::Primitive
{
	Data::VertexBufferData CameraFrame::MakeFrustumVertex(const Camera::ProjectionData& data) noexcept
	{
		Data::VertexBufferData vertices(GetLayoutFrustum(), 8);
		const float zRatio = data.farClip / data.nearClip;
		const float nearY = data.nearClip * tanf(data.fov / 2.0f);
		const float nearX = nearY * data.screenRatio;
		const float farY = nearY * zRatio;
		const float farX = nearX * zRatio;
		vertices.SetBox({ farY, -farY, -farX, farX, data.nearClip, data.farClip });

		vertices[0].SetByIndex(0, Float3(-nearX, nearY, data.nearClip));
		vertices[1].SetByIndex(0, Float3(nearX, nearY, data.nearClip));
		vertices[2].SetByIndex(0, Float3(nearX, -nearY, data.nearClip));
		vertices[3].SetByIndex(0, Float3(-nearX, -nearY, data.nearClip));
		vertices[4].SetByIndex(0, Float3(-farX, farY, data.farClip));
		vertices[5].SetByIndex(0, Float3(farX, farY, data.farClip));
		vertices[6].SetByIndex(0, Float3(farX, -farY, data.farClip));
		vertices[7].SetByIndex(0, Float3(-farX, -farY, data.farClip));

		return vertices;
	}

	std::vector<U32> CameraFrame::MakeFrustumIndex() noexcept
	{
		return
		{
			0,1, 1,2, 2,3, 3,0, // Back rectangle
			4,5, 5,6, 6,7, 7,4, // Front rectangle
			0,4, 1,5, 2,6, 3,7  // Rear lines
		};
	}

	Data::VertexBufferData CameraFrame::MakeIndicatorVertex() noexcept
	{
		Data::VertexBufferData vertices(GetLayoutIndicator(), 8);

		constexpr float LENGTH = 1.0f, width = 1.0f, height = 0.7f;
		vertices.SetBox({ height * 2.0f, -height, -width, width, 0.0f, LENGTH });

		vertices[0].SetByIndex(0, Float3(0.0f, 0.0f, 0.0f));
		vertices[1].SetByIndex(0, Float3(-width, height, LENGTH));
		vertices[2].SetByIndex(0, Float3(width, height, LENGTH));
		vertices[3].SetByIndex(0, Float3(width, -height, LENGTH));
		vertices[4].SetByIndex(0, Float3(-width, -height, LENGTH));
		vertices[5].SetByIndex(0, Float3(width / 2.0f, height + 0.15f, LENGTH));
		vertices[6].SetByIndex(0, Float3(0.0f, height * 2.0f, LENGTH));
		vertices[7].SetByIndex(0, Float3(width / -2.0f, height + 0.15f, LENGTH));

		return vertices;
	}

	std::vector<U32> CameraFrame::MakeIndicatorIndex() noexcept
	{
		return
		{
			0,4, 0,1, 0,2, 0,3, // Back lines
			4,1, 1,2, 2,3, 3,4, // Front rectangle
			5,6, 6,7, 7,5 // Top triangle
		};
	}
}