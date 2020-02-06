#pragma once
#include "GfxObject.h"
#include "BasicObject.h"

namespace GFX
{
	class Object : public GfxObject, public BasicObject
	{
	protected:
		DirectX::FXMMATRIX CreateTransformMatrix() const noexcept;

	public:
		Object(const DirectX::XMFLOAT3& position);
		Object(const std::string& name = "");
		Object(const DirectX::XMFLOAT3& position, const std::string& name, float scale = 1.0f);
		Object(const Object&) = default;
		Object& operator=(const Object&) = default;
		virtual ~Object() = default;

		void SetAngle(const DirectX::XMFLOAT3& meshAngle) noexcept override;
		void SetScale(float newScale) noexcept override;
		void SetPos(const DirectX::XMFLOAT3& position) noexcept override;
		void Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle = { 0.0f,0.0f,0.0f }) noexcept override;
		void ShowWindow() noexcept override;

		virtual void UpdateTransformMatrix() noexcept;
	};
}
