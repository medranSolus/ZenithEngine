#pragma once
#include "IDrawable.h"
#include "Vertex.h"
#include "GfxObject.h"
#include "GfxResources.h"

namespace GFX::Shape
{
	class BaseShape : public virtual IDrawable
	{
		Resource::IndexBuffer* indexBuffer = nullptr;
		std::vector<std::shared_ptr<Resource::IBindable>> binds;

	protected:
		BaseShape(Graphics& gfx, const GfxObject& parent);
		BaseShape(const BaseShape&) = delete;
		BaseShape& operator=(const BaseShape&) = delete;
		virtual ~BaseShape() = default;

		inline void SetTopology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY topology) noexcept { SetResource(Resource::Topology::Get(gfx, topology)); }

		template<typename R>
		R* GetResource() noexcept;
		template<typename R>
		void SetResource(std::shared_ptr<R> resource) noexcept;

		void AddBind(std::shared_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG);

	public:
		inline void SetTopologyPlain(Graphics& gfx) noexcept { SetResource(Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)); }
		virtual inline void SetTopologyMesh(Graphics& gfx) noexcept { SetResource(Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_LINELIST)); }

		void Draw(Graphics& gfx) const noexcept override;
		void ShowWindow(Graphics& gfx) noexcept override;
	};

	template<typename R>
	R* BaseShape::GetResource() noexcept
	{
		for (auto& bind : binds)
			if (auto res = dynamic_cast<R*>(&bind))
				return res;
		return nullptr;
	}

	template<typename R>
	void BaseShape::SetResource(std::shared_ptr<R> resource) noexcept
	{
		for (size_t i = 0; i < binds.size(); ++i)
		{
			if (dynamic_cast<R*>(binds.at(i).get()) != nullptr)
			{
				binds.at(i) = resource;
				return;
			}
		}
	}
}