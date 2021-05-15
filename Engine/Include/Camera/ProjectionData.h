#pragma once

namespace ZE::Camera
{
	struct ProjectionData
	{
		float fov;
		float screenRatio;
		float nearClip;
		float farClip;
	};
}