#pragma once
#include "IndexBuffer.h"
#include <DirectXMath.h>

namespace GFX::Object
{
	class IDrawable
	{
		template<typename T>
		friend class ObjectBase;

		std::vector<std::unique_ptr<Resource::IBindable>> binds;

		virtual const std::vector<std::unique_ptr<Resource::IBindable>> & GetStaticBinds() const noexcept = 0;
		virtual const Resource::IndexBuffer * GetStaticIndexBuffer() const noexcept = 0;
	protected:
		void AddBind(std::unique_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG);

	public:
		IDrawable() = default;
		IDrawable(const IDrawable &) = delete;
		IDrawable & operator=(const IDrawable &) = delete;
		virtual ~IDrawable() = default;
		
		virtual DirectX::XMMATRIX GetTransformMatrix() const noexcept = 0;
		virtual void Update(float dX, float dY, float dZ, float angleDZ, float angleDX, float angleDY) noexcept = 0;

		void Draw(Graphics & gfx) const noexcept;
	};
}
