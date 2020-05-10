#include "Object.h"

namespace GFX
{
	DirectX::FXMMATRIX Object::CreateTransformMatrix() const noexcept
	{
		return DirectX::XMMatrixScaling(scale, scale, scale) *
			DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&angle)) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos));
	}

	Object::Object(const DirectX::XMFLOAT3& position) : BasicObject(position)
	{
		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos)));
	}

	Object::Object(const std::string& name) : BasicObject(name)
	{
		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixIdentity());
	}

	Object::Object(const DirectX::XMFLOAT3& position, const std::string& name, float scale) : BasicObject(position, name, scale)
	{
		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixScaling(scale, scale, scale) * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos)));
	}

	void Object::SetAngle(const DirectX::XMFLOAT3& meshAngle) noexcept
	{
		BasicObject::SetAngle(meshAngle);
		UpdateTransformMatrix();
	}

	void Object::SetScale(float newScale) noexcept
	{
		BasicObject::SetScale(newScale);
		UpdateTransformMatrix();
	}

	void Object::SetPos(const DirectX::XMFLOAT3& position) noexcept
	{
		BasicObject::SetPos(position);
		UpdateTransformMatrix();
	}

	void Object::Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle) noexcept
	{
		BasicObject::Update(delta, deltaAngle);
		UpdateTransformMatrix();
	}

	void Object::Accept(Probe& probe) noexcept
	{
		if (probe.VisitObject(*this))
			UpdateTransformMatrix();
	}

	void Object::UpdateTransformMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(), CreateTransformMatrix());
	}
}