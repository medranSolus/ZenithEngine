#include "BaseShape.h"

namespace GFX::Shape
{
	BaseShape::BaseShape(Graphics& gfx, const GfxObject& parent, std::shared_ptr<Resource::IndexBuffer> indexBuffer, std::shared_ptr<Resource::VertexBuffer> vertexBuffer)
		: indexBuffer(std::move(indexBuffer)), vertexBuffer(std::move(vertexBuffer))
	{
		topology = Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		transformBuffer = std::make_shared<Resource::ConstBufferTransform>(gfx, parent);
	}

	void BaseShape::Bind(Graphics& gfx) noexcept
	{
		indexBuffer->Bind(gfx);
		vertexBuffer->Bind(gfx);
		topology->Bind(gfx);
		transformBuffer->Bind(gfx);
	}

	void BaseShape::Submit(Pipeline::RenderCommander& renderer) noexcept(!IS_DEBUG)
	{
		for (auto& technique : techniques)
			technique->Submit(renderer, *this);
	}

	void BaseShape::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		probe.VisitShape(gfx, *this);
		for (auto& technique : techniques)
			technique->Accept(gfx, probe);
	}
}