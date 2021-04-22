#pragma once
#include "ColorF3.h"
#include "MathExt.h"
#include <string>

namespace GFX::Light
{
	enum LightType : U8 { Directional = 0, Point = 1, Spot = 2 };

	class LightParams final
	{
	public:
		Float3 position = { 0.0f, 0.0f, 0.0f };
		float intensity = 1.0f;
		Float3 direction = { 0.0f, -1.0f, 0.0f };
		float size = 0.5f;
		ColorF3 color = { 1.0f, 1.0f, 1.0f };
		float innerAngle = Math::ToRadians(15.0f);
		U64 range = 100;
		float outerAngle = Math::ToRadians(40.0f);
		std::string name = "Light";
		LightType type = LightType::Directional;

		LightParams() = default;
		LightParams(const Float3& position, float intensity, const Float3& direction, float size,
			const ColorF3& color, float innerAngle, U64 range, float outerAngle, const std::string& name) noexcept
			: position(position), intensity(intensity), direction(direction), size(size), color(color),
			innerAngle(innerAngle), range(range), outerAngle(outerAngle), name(name) {}

		LightParams(Float3&& position, float intensity, Float3&& direction, float size,
			ColorF3&& color, float innerAngle, U64 range, float outerAngle, std::string&& name) noexcept
			: position(std::move(position)), intensity(intensity), direction(std::move(direction)), size(size),
			color(std::move(color)), innerAngle(innerAngle), range(range), outerAngle(outerAngle), name(std::move(name)) {}

		LightParams(LightParams&&) = default;
		LightParams(const LightParams&) = delete;
		LightParams& operator=(LightParams&&) = default;
		LightParams& operator=(const LightParams&) = delete;
		~LightParams() = default;

		void Reset() noexcept
		{
			position = { 0.0f, 0.0f, 0.0f };
			intensity = 1.0f;
			direction = { 0.0f, -1.0f, 0.0f };
			size = 0.5f;
			color = { 1.0f, 1.0f, 1.0f };
			innerAngle = Math::ToRadians(15.0f);
			range = 100;
			outerAngle = Math::ToRadians(40.0f);
			name = "Light";
			type = LightType::Directional;
		}
	};
}