#include "Object.h"

namespace GFX
{
	DirectX::FXMMATRIX Object::CreateTransformMatrix() const noexcept
	{
		float scale = GetScale();
		return DirectX::XMMatrixScaling(scale, scale, scale) *
			DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&GetAngle())) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&GetPos()));
	}

	Object::Object(const DirectX::XMFLOAT3& position) noexcept : BasicObject(position)
	{
		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&position)));
	}

	Object::Object(const std::string& name) noexcept : BasicObject(name)
	{
		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixIdentity());
	}

	Object::Object(const DirectX::XMFLOAT3& position, const std::string& name, float scale) noexcept
		: BasicObject(position, name, scale)
	{
		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixScaling(scale, scale, scale) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&position)));
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

	void Object::UpdatePos(const DirectX::XMFLOAT3& delta) noexcept
	{
		BasicObject::UpdatePos(delta);
		UpdateTransformMatrix();
	}

	void Object::UpdateAngle(const DirectX::XMFLOAT3& deltaAngle) noexcept
	{
		BasicObject::UpdateAngle(deltaAngle);
		UpdateTransformMatrix();
	}

	bool Object::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		if (probe.VisitObject(buffer))
		{
			UpdateTransformMatrix();
			return true;
		}
		return false;
	}

	void Object::UpdateTransformMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(), CreateTransformMatrix());
	}
}