#pragma once
#include <DirectXMath.h>
#include <string>

namespace GFX::Shape
{
	class ModelParams
	{
	public:
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		std::string name = "Model";
		float scale = 1.0f;;

		inline ModelParams() noexcept {}
		inline ModelParams(const DirectX::XMFLOAT3& position, const std::string& name, float scale) noexcept
			: position(position), name(name), scale(scale) {}
		inline ModelParams(DirectX::XMFLOAT3&& position, std::string&& name, float scale) noexcept
			: position(std::move(position)), name(std::move(name)), scale(scale) {}
		~ModelParams() = default;

		inline void Reset() noexcept { position = { 0.0f, 0.0f, 0.0f }; name = "Model"; scale = 1.0f; }
	};
}