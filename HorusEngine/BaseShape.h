#pragma once
#include "IDrawable.h"
#include "IndexBuffer.h"
#include "Vertex.h"

namespace GFX::Shape
{
	class BaseShape : public virtual IDrawable
	{
		Resource::IndexBuffer* indexBuffer = nullptr;
		std::vector<std::shared_ptr<Resource::IBindable>> binds;

	protected:
		BaseShape() = default;
		BaseShape(const BaseShape&) = delete;
		BaseShape& operator=(const BaseShape&) = delete;
		virtual ~BaseShape() = default;

		template<typename R>
		R* GetResource() noexcept;

		void AddBind(std::shared_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG);

	public:
		void Draw(Graphics& gfx) const noexcept override;
	};

	template<typename R>
	R* BaseShape::GetResource() noexcept
	{
		for (auto& bind : binds)
			if (auto res = dynamic_cast<R*>(&bind))
				return res;
		return nullptr;
	}
}
