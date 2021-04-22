#pragma once
#include "GfxObject.h"
#include "BasicObject.h"

namespace GFX
{
	class Object : public GfxObject, public BasicObject
	{
	protected:
		Matrix CreateTransformMatrix() const noexcept;

	public:
		Object(std::string&& name = "") noexcept;
		Object(const Float3& position) noexcept;
		Object(const Float3& position, std::string&& name, float scale = 1.0f) noexcept;
		Object(Object&&) = default;
		Object(const Object&) = default;
		Object& operator=(Object&&) = default;
		Object& operator=(const Object&) = default;
		virtual ~Object() = default;

		virtual void UpdateTransformMatrix() noexcept { Math::XMStoreFloat4x4(transform.get(), CreateTransformMatrix()); }

		void SetAngle(const Float3& meshAngle) noexcept override;
		void SetScale(float newScale) noexcept override;
		void SetPos(const Float3& position) noexcept override;
		void Update(const Float3& delta, const Float3& deltaAngle) noexcept override;
		void UpdatePos(const Float3& delta) noexcept override;
		void UpdateAngle(const Float3& deltaAngle) noexcept override;
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}