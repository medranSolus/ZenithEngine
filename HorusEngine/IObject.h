#pragma once
#include "IDrawable.h"
#include "Graphics.h"

namespace GFX
{
	class IObject : public virtual IDrawable
	{
	public:
		virtual const DirectX::XMFLOAT3 & GetAngle() const noexcept = 0;
		virtual void SetAngle(const DirectX::XMFLOAT3 & meshAngle) noexcept = 0;

		virtual const DirectX::XMFLOAT3 & GetPos() const noexcept = 0;
		virtual void SetPos(const DirectX::XMFLOAT3 & position) noexcept = 0;

		virtual const std::string & GetName() const noexcept = 0;
		virtual void SetName(const std::string & newName) noexcept = 0;

		virtual void Update(const DirectX::XMFLOAT3 & delta, const DirectX::XMFLOAT3 & deltaAngle = { 0.0f,0.0f,0.0f }) noexcept = 0;
		virtual void ShowWindow() noexcept = 0;
	};
}
