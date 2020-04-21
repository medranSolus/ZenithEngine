#include "BaseShape.h"
#include "ImGui/imgui.h"

namespace GFX::Shape
{
	BaseShape::BaseShape(Graphics& gfx, const GfxObject& parent)
	{
		binds.emplace_back(Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		binds.emplace_back(Resource::DepthStencil::Get(gfx, Resource::DepthStencil::StencilMode::Off));
		binds.emplace_back(std::make_shared<Resource::ConstBufferTransform>(gfx, parent));
	}

	void BaseShape::AddBind(std::shared_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG)
	{
		assert(bind != nullptr);
		if (typeid(*bind) == typeid(Resource::IndexBuffer))
		{
			assert("Attempting to add index buffer a second time" && indexBuffer == nullptr);
			indexBuffer = &static_cast<Resource::IndexBuffer&>(*bind);
		}
		binds.emplace_back(bind);
	}

	void BaseShape::Draw(Graphics& gfx) const noexcept
	{
		for (auto& b : binds)
			b->Bind(gfx);
		gfx.DrawIndexed(indexBuffer->GetCount());
	}
}