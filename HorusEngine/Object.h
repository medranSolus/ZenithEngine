#pragma once
#include "IObject.h"

namespace GFX
{
	class Object : public IObject
	{
	protected:
		DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
		std::string name = "";

	public:
		Object() = default;
		inline Object(const DirectX::XMFLOAT3 & position) : pos(position) {}
		inline Object(const std::string & name) : name(name) {}
		inline Object(const DirectX::XMFLOAT3 & position, const std::string & name) : pos(position), name(name) {}
		Object(const Object&) = default;
		Object & operator=(const Object&) = default;
		virtual ~Object() {}

		inline const DirectX::XMFLOAT3 & GetAngle() const noexcept override { return angle; }
		inline void SetAngle(const DirectX::XMFLOAT3 & meshAngle) noexcept override { angle = meshAngle; }
		inline const DirectX::XMFLOAT3 & GetPos() const noexcept override { return pos; }
		inline void SetPos(const DirectX::XMFLOAT3 & position) noexcept override { pos = position; }
		inline const std::string & GetName() const noexcept override { return name; }
		inline void SetName(const std::string & newName) noexcept override { name = newName; }

		constexpr DirectX::XMFLOAT3 * Pos() noexcept { return &pos; }

		void Update(const DirectX::XMFLOAT3 & delta, const DirectX::XMFLOAT3 & deltaAngle = { 0.0f,0.0f,0.0f }) noexcept override;
		inline void ShowWindow() noexcept override {}
	};
}
