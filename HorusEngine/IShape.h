#pragma once
#include "IDrawable.h"
#include "IndexBuffer.h"

namespace GFX::Shape
{
	class IShape : public virtual IDrawable
	{
		template<typename T>
		friend class BaseShape;

		virtual const std::vector<std::unique_ptr<Resource::IBindable>> & GetBinds() const noexcept = 0;
		virtual const std::vector<std::unique_ptr<Resource::IBindable>> & GetStaticBinds() const noexcept = 0;
		virtual const Resource::IndexBuffer * GetStaticIndexBuffer() const noexcept = 0;
		virtual const Resource::IndexBuffer * GetIndexBuffer() const noexcept = 0;

	public:
		virtual ~IShape() = default;
		
		void Draw(Graphics & gfx) const noexcept override;
	};
}
