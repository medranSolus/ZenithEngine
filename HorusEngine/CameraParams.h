#pragma once
#include <DirectXMath.h>
#include <string>

namespace Camera
{
	enum CameraType : uint8_t { Person = 0, Floating = 1 };

	class CameraParams
	{
	public:
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		std::string name = "Camera";
		float angleHorizontal = 0.0f;
		float angleVertical = 0.0f;
		float fov = 1.047f;
		float nearClip = 0.01f;
		float farClip = 500.0f;
		CameraType type = CameraType::Person;

		inline CameraParams() noexcept {}
		inline CameraParams(const DirectX::XMFLOAT3& position, const std::string& name,
			float angleHorizontal, float angleVertical, float fov, float nearClip, float farClip) noexcept
			: position(position), name(name), angleHorizontal(angleHorizontal),
			angleVertical(angleVertical), fov(fov), nearClip(nearClip), farClip(farClip) {}
		inline CameraParams(DirectX::XMFLOAT3&& position, std::string&& name,
			float angleHorizontal, float angleVertical, float fov, float nearClip, float farClip) noexcept
			: position(std::move(position)), name(std::move(name)), angleHorizontal(angleHorizontal),
			angleVertical(angleVertical), fov(fov), nearClip(nearClip), farClip(farClip) {}
		~CameraParams() = default;

		inline void Reset() noexcept { position = { 0.0f, 0.0f, 0.0f }; name = "Camera"; angleHorizontal = angleVertical = 0.0f; fov = 1.047f; nearClip = 0.01f; farClip = 500.0f; type = CameraType::Person; }
	};
}