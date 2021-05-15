#pragma once
#include "Types.h"
#include <string>

namespace ZE::GFX::Shape
{
	class ModelParams final
	{
	public:
		Float3 position = { 0.0f, 0.0f, 0.0f };
		Float4 rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		std::string name = "Model";
		float scale = 1.0f;;

		ModelParams() = default;
		ModelParams(const Float3& position, const Float4& rotation, const std::string& name, float scale) noexcept
			: position(position), rotation(rotation), name(name), scale(scale) {}
		ModelParams(Float3&& position, Float4&& rotation, std::string&& name, float scale) noexcept
			: position(std::move(position)), rotation(std::move(rotation)), name(std::move(name)), scale(scale) {}

		ModelParams(ModelParams&&) = default;
		ModelParams(const ModelParams&) = delete;
		ModelParams& operator=(ModelParams&&) = default;
		ModelParams& operator=(const ModelParams&) = delete;
		~ModelParams() = default;

		void Reset() noexcept
		{
			position = { 0.0f, 0.0f, 0.0f };
			rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
			name = "Model";
			scale = 1.0f;
		}
	};
}