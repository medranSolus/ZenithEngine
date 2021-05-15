#pragma once
#include "GFX/Resource/GfxResources.h"
#include "GFX/Pipeline/JobData.h"

namespace ZE::GFX::Shape
{
	class BaseShape : public virtual Pipeline::JobData
	{
		GfxResPtr<Resource::IndexBuffer> indexBuffer;
		GfxResPtr<Resource::VertexBuffer> vertexBuffer;
		GfxResPtr<Resource::Topology> topology;
		bool isMesh = false;
		bool isOutline = false;

		void SetMesh(bool mesh) noexcept;

	protected:
		BaseShape(Graphics& gfx, GfxResPtr<Resource::IndexBuffer>&& indexBuffer = {},
			GfxResPtr<Resource::VertexBuffer>&& vertexBuffer = {});
		BaseShape(const BaseShape&) = delete;
		BaseShape& operator=(const BaseShape&) = delete;
		virtual ~BaseShape() = default;

		void SetIndexBuffer(GfxResPtr<Resource::IndexBuffer>&& index) noexcept { indexBuffer = std::move(index); }
		void SetVertexBuffer(GfxResPtr<Resource::VertexBuffer>&& vertex) noexcept { vertexBuffer = std::move(vertex); }
		void SetTopology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY newTopology) noexcept { topology = Resource::Topology::Get(gfx, newTopology); SetMesh(true); }

	public:
		constexpr bool IsMesh() const noexcept { return isMesh; }
		constexpr bool IsOutline() const noexcept { return isOutline; }
		constexpr U32 GetIndexCount() const noexcept override { return indexBuffer->GetCount(); }

		const Data::BoundingBox& GetBoundingBox() const noexcept override { return vertexBuffer->GetBox(); }
		virtual void SetTopologyPlain(Graphics& gfx) noexcept { topology = Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); SetMesh(false); }
		virtual void SetTopologyMesh(Graphics& gfx) noexcept { topology = Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST); SetMesh(true); }

		void Bind(Graphics& gfx) const override;
		void SetOutline() noexcept override;
		void DisableOutline() noexcept override;
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}