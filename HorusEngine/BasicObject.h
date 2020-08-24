#pragma once
#include "IObject.h"
#include "DynamicCBuffer.h"

namespace GFX
{
	class BasicObject : public IObject
	{
	protected:
		Data::CBuffer::DynamicCBuffer buffer;
		std::string name = "";

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		inline BasicObject() : BasicObject({ 0.0f,0.0f,0.0f }, "") {}
		inline BasicObject(const DirectX::XMFLOAT3& position) : BasicObject(position, "") {}
		inline BasicObject(const std::string& name) : BasicObject({ 0.0f,0.0f,0.0f }, name) {}
		BasicObject(const DirectX::XMFLOAT3& position, const std::string& name, float scale = 1.0f);
		BasicObject(const BasicObject&) = default;
		BasicObject& operator=(const BasicObject&) = default;
		virtual ~BasicObject() = default;

		inline DirectX::XMFLOAT3& GetAngle() noexcept { return buffer["angle"]; }
		inline const DirectX::XMFLOAT3& GetAngle() const noexcept override { return buffer["angle"]; }
		inline void SetAngle(const DirectX::XMFLOAT3& meshAngle) noexcept override { buffer["angle"] = meshAngle; }

		inline float& GetScale() noexcept { return buffer["scale"]; }
		inline float GetScale() const noexcept override { return buffer["scale"]; }
		inline void SetScale(float newScale) noexcept override { buffer["scale"] = newScale; }

		inline DirectX::XMFLOAT3& GetPos() noexcept { return buffer["position"]; }
		inline const DirectX::XMFLOAT3& GetPos() const noexcept override { return buffer["position"]; }
		inline void SetPos(const DirectX::XMFLOAT3& position) noexcept override { buffer["position"] = position; }

		inline const std::string& GetName() const noexcept override { return name; }
		inline void SetName(const std::string& newName) noexcept override { name = newName; }

		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { probe.VisitObject(buffer); }
		void UpdatePos(const DirectX::XMFLOAT3& delta) noexcept override;
		void UpdateAngle(const DirectX::XMFLOAT3& deltaAngle) noexcept override;
	};
}