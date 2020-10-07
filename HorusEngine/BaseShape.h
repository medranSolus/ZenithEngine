#pragma once
#include "GfxResources.h"
#include "JobData.h"

namespace GFX::Shape
{
	class BaseShape : public Pipeline::JobData
	{
		std::shared_ptr<Resource::IndexBuffer> indexBuffer = nullptr;
		std::shared_ptr<Resource::VertexBuffer> vertexBuffer = nullptr;
		std::shared_ptr<Resource::Topology> topology = nullptr;
		bool isMesh = false;
		bool isOutline = false;

		void SetMesh(bool mesh) noexcept;

	protected:
		BaseShape(Graphics& gfx, std::shared_ptr<Resource::IndexBuffer> indexBuffer = nullptr,
			std::shared_ptr<Resource::VertexBuffer> vertexBuffer = nullptr);
		BaseShape(const BaseShape&) = delete;
		BaseShape& operator=(const BaseShape&) = delete;
		virtual ~BaseShape() = default;

		inline void SetIndexBuffer(std::shared_ptr<Resource::IndexBuffer> index) noexcept { indexBuffer = std::move(index); }
		inline void SetVertexBuffer(std::shared_ptr<Resource::VertexBuffer> vertex) noexcept { vertexBuffer = std::move(vertex); }
		inline void SetTopology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY newTopology) noexcept { topology = Resource::Topology::Get(gfx, newTopology); SetMesh(true); }

	public:
		inline UINT GetIndexCount() const noexcept override { return indexBuffer->GetCount(); }

		constexpr bool IsMesh() const noexcept { return isMesh; }
		constexpr bool IsOutline() const noexcept { return isOutline; }

		virtual inline void SetTopologyPlain(Graphics& gfx) noexcept { topology = Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); SetMesh(false); }
		virtual inline void SetTopologyMesh(Graphics& gfx) noexcept { topology = Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST); SetMesh(true); }

		void Bind(Graphics& gfx) override;
		void SetOutline() noexcept override;
		void DisableOutline() noexcept override;
		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}