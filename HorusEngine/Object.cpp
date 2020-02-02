#include "Object.h"

namespace GFX
{
	Object::Object()
	{
		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(scaling.get(), DirectX::XMMatrixIdentity());
	}

	Object::Object(const DirectX::XMFLOAT3& position) : BasicObject(position)
	{
		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos)));
		DirectX::XMStoreFloat4x4(scaling.get(), DirectX::XMMatrixIdentity());
	}

	Object::Object(const std::string& name) : BasicObject(name)
	{
		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(scaling.get(), DirectX::XMMatrixIdentity());
	}

	Object::Object(const DirectX::XMFLOAT3& position, const std::string& name, float scale) : BasicObject(position, name, scale)
	{
		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos)));
		DirectX::XMStoreFloat4x4(scaling.get(), DirectX::XMMatrixScaling(scale, scale, scale));
	}

	inline void Object::UpdateMatrices() noexcept
	{
		UpdateTransformMatrix();
		UpdateScalingMatrix();
	}

	void Object::SetAngle(const DirectX::XMFLOAT3& meshAngle) noexcept
	{
		BasicObject::SetAngle(meshAngle);
		UpdateTransformMatrix();
	}

	void Object::SetScale(float newScale) noexcept
	{
		BasicObject::SetScale(newScale);
		UpdateScalingMatrix();
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

	void Object::ShowWindow() noexcept
	{
		BasicObject::ShowWindow();
		UpdateMatrices();
	}

	void Object::UpdateTransformMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&angle)) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos)));
	}

	void Object::UpdateScalingMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(scaling.get(), DirectX::XMMatrixScaling(scale, scale, scale));
	}
}