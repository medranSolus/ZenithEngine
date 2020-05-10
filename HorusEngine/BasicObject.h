#pragma once
#include "IObject.h"

namespace GFX
{
	class BasicObject : public IObject
	{
	protected:
		mutable DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };
		float scale = 1.0f;
		mutable DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
		std::string name = "";

	public:
		BasicObject() = default;
		inline BasicObject(const DirectX::XMFLOAT3& position) : pos(position) {}
		inline BasicObject(const std::string& name) : name(name) {}
		inline BasicObject(const DirectX::XMFLOAT3& position, const std::string& name, float scale = 1.0f) : pos(position), name(name), scale(scale) {}
		BasicObject(const BasicObject&) = default;
		BasicObject& operator=(const BasicObject&) = default;
		virtual ~BasicObject() = default;

		constexpr DirectX::XMFLOAT3& GetAngle() noexcept { return angle; }
		inline const DirectX::XMFLOAT3& GetAngle() const noexcept override { return angle; }
		inline void SetAngle(const DirectX::XMFLOAT3& meshAngle) noexcept override { angle = meshAngle; }

		constexpr float& GetScale() noexcept { return scale; }
		inline float GetScale() const noexcept override { return scale; }
		inline void SetScale(float newScale) noexcept override { scale = newScale; }

		constexpr DirectX::XMFLOAT3& GetPos() noexcept { return pos; }
		inline const DirectX::XMFLOAT3& GetPos() const noexcept override { return pos; }
		inline void SetPos(const DirectX::XMFLOAT3& position) noexcept override { pos = position; }

		inline const std::string& GetName() const noexcept override { return name; }
		inline void SetName(const std::string& newName) noexcept override { name = newName; }

		inline void Accept(Probe& probe) noexcept override { probe.VisitObject(*this); }
		void Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle = { 0.0f,0.0f,0.0f }) noexcept override;
	};
}