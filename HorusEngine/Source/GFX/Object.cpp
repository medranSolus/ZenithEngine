#include "GFX/Object.h"

namespace GFX
{
	Matrix Object::CreateTransformMatrix() const noexcept
	{
		float scale = GetScale();
		return Math::XMMatrixScaling(scale, scale, scale) *
			Math::XMMatrixRotationQuaternion(Math::XMLoadFloat4(&GetAngle())) *
			Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&GetPos()));
	}

	Object::Object(std::string&& name) noexcept : BasicObject(std::forward<std::string>(name))
	{
		Math::XMStoreFloat4x4(transform.get(), Math::XMMatrixIdentity());
	}

	Object::Object(const Float3& position) noexcept : BasicObject(position)
	{
		Math::XMStoreFloat4x4(transform.get(), Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&position)));
	}

	Object::Object(const Float3& position, std::string&& name, float scale) noexcept
		: BasicObject(position, std::forward<std::string>(name), scale)
	{
		Math::XMStoreFloat4x4(transform.get(), Math::XMMatrixScaling(scale, scale, scale) *
			Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&position)));
	}

	void Object::SetAngle(const Vector& rotor) noexcept
	{
		BasicObject::SetAngle(rotor);
		UpdateTransformMatrix();
	}

	void Object::SetAngle(const Float4& rotor) noexcept
	{
		BasicObject::SetAngle(rotor);
		UpdateTransformMatrix();
	}

	void Object::SetScale(float newScale) noexcept
	{
		BasicObject::SetScale(newScale);
		UpdateTransformMatrix();
	}

	void Object::SetPos(const Float3& position) noexcept
	{
		BasicObject::SetPos(position);
		UpdateTransformMatrix();
	}

	void Object::Update(const Float3& delta, const Vector& rotor) noexcept
	{
		BasicObject::UpdatePos(delta);
		UpdateAngle(rotor);
	}

	void Object::UpdatePos(const Float3& delta) noexcept
	{
		BasicObject::UpdatePos(delta);
		UpdateTransformMatrix();
	}

	void Object::UpdateAngle(const Vector& rotor) noexcept
	{
		BasicObject::UpdateAngle(rotor);
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
}