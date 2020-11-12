#pragma once
#include "Math.h"
#include <string>

namespace GFX::Light
{
	enum LightType : uint8_t { Directional = 0, Point = 1, Spot = 2 };

	class LightParams
	{
	public:
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		float intensity = 1.0f;
		DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };
		float size = 0.5f;
		Data::ColorFloat3 color = { 1.0f, 1.0f, 1.0f };
		float innerAngle = Math::ToRadians(15.0f);
		size_t range = 100;
		float outerAngle = Math::ToRadians(40.0f);
		std::string name = "Light";
		LightType type = LightType::Directional;

		inline LightParams() noexcept {}
		LightParams(const DirectX::XMFLOAT3& position, float intensity, const DirectX::XMFLOAT3& direction, float size,
			const Data::ColorFloat3& color, float innerAngle, size_t range, float outerAngle, const std::string& name) noexcept
			: position(position), intensity(intensity), direction(direction), size(size), color(color),
			innerAngle(innerAngle), range(range), outerAngle(outerAngle), name(name) {}

		LightParams(DirectX::XMFLOAT3&& position, float intensity, DirectX::XMFLOAT3&& direction, float size,
			Data::ColorFloat3&& color, float innerAngle, size_t range, float outerAngle, std::string&& name) noexcept
			: position(std::move(position)), intensity(intensity), direction(std::move(direction)), size(size),
			color(std::move(color)), innerAngle(innerAngle), range(range), outerAngle(outerAngle), name(std::move(name)) {}

		~LightParams() = default;

		void Reset() noexcept
		{
			position = { 0.0f, 0.0f, 0.0f }; intensity = 1.0f;
			direction = { 0.0f, -1.0f, 0.0f }; size = 0.5f;
			color = { 1.0f, 1.0f, 1.0f }; innerAngle = Math::ToRadians(15.0f);
			range = 100; outerAngle = Math::ToRadians(40.0f);
			name = "Light"; type = LightType::Directional;
		}
	};
}