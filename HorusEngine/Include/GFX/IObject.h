#pragma once
#include "Pipeline/IRenderable.h"

namespace GFX
{
	class IObject : public virtual Pipeline::IRenderable
	{
	public:
		virtual ~IObject() = default;

		virtual const Float3& GetAngle() const noexcept = 0;
		virtual void SetAngle(const Float3& meshAngle) noexcept = 0;

		virtual float GetScale() const noexcept = 0;
		virtual void SetScale(float newScale) noexcept = 0;

		virtual const Float3& GetPos() const noexcept = 0;
		virtual void SetPos(const Float3& position) noexcept = 0;

		virtual const std::string& GetName() const noexcept = 0;
		virtual void SetName(const std::string& newName) noexcept = 0;

		virtual void Update(const Float3& delta, const Float3& deltaAngle) noexcept { UpdatePos(delta); UpdateAngle(deltaAngle); }
		virtual void UpdatePos(const Float3& delta) noexcept = 0;
		virtual void UpdateAngle(const Float3& delta) noexcept = 0;
	};
}