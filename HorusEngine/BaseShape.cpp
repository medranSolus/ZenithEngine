#include "BaseShape.h"

namespace GFX::Shape
{
	BaseShape::BaseShape(Graphics& gfx, std::shared_ptr<Resource::IndexBuffer> indexBuffer, std::shared_ptr<Resource::VertexBuffer> vertexBuffer)
		: indexBuffer(std::move(indexBuffer)), vertexBuffer(std::move(vertexBuffer))
	{
		topology = Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void BaseShape::Bind(Graphics& gfx)
	{
		indexBuffer->Bind(gfx);
		vertexBuffer->Bind(gfx);
		topology->Bind(gfx);
	}

	void BaseShape::SetOutline() noexcept
	{
		for (auto& technique : techniques)
			if (technique->GetName().find("Outline") != std::string::npos)
			{
				technique->Activate();
				isOutline = true;
				break;
			}
	}

	void BaseShape::DisableOutline() noexcept
	{
		for (auto& technique : techniques)
			if (technique->GetName().find("Outline") != std::string::npos)
			{
				technique->Dectivate();
				isOutline = false;
				break;
			}
	}

	void BaseShape::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		probe.VisitShape(gfx, *this);
		JobData::Accept(gfx, probe);
	}
}