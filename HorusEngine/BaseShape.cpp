#include "BaseShape.h"
#include "ImGui/imgui.h"

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
		if (visible)
		{
			for (auto& technique : techniques)
				technique->Submit(renderer, *this);
		}
	}

	void BaseShape::ShowWindow(Graphics& gfx) noexcept
	{
		static bool previousMesh = false;
		static bool meshOnly = false;
		ImGui::Checkbox("Mesh-only", &meshOnly);
		if (previousMesh != meshOnly)
		{
			previousMesh = meshOnly;
			if (meshOnly)
				SetTopologyMesh(gfx);
			else
				SetTopologyPlain(gfx);
		}
		ImGui::SameLine();
		ImGui::Checkbox("Object visible", &visible);
	}
}