#pragma once
#include <DirectXMath.h>
#include <string>

namespace GFX::Shape
{
	class ModelParams
	{
	public:
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
		std::string name = "Model";
		float scale = 1.0f;;

		inline ModelParams() noexcept {}
		inline ModelParams(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation, const std::string& name, float scale) noexcept
			: position(position), rotation(rotation), name(name), scale(scale) {}
		inline ModelParams(DirectX::XMFLOAT3&& position, DirectX::XMFLOAT3&& rotation, std::string&& name, float scale) noexcept
			: position(std::move(position)), rotation(std::move(rotation)), name(std::move(name)), scale(scale) {}
		~ModelParams() = default;

		inline void Reset() noexcept { position = { 0.0f, 0.0f, 0.0f }; rotation = { 0.0f, 0.0f, 0.0f }; name = "Model"; scale = 1.0f; }
	};
}