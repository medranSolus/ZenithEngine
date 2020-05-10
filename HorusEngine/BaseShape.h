#pragma once
#include "IRenderable.h"
#include "GfxResources.h"
#include "Technique.h"

namespace GFX::Shape
{
	class BaseShape : public virtual IRenderable
	{
		std::shared_ptr<Resource::IndexBuffer> indexBuffer = nullptr;
		std::shared_ptr<Resource::VertexBuffer> vertexBuffer = nullptr;
		std::shared_ptr<Resource::Topology> topology = nullptr;
		std::shared_ptr<Resource::ConstBufferTransform> transformBuffer = nullptr; // Maybe move to TechniqueStep
		std::vector<std::shared_ptr<Pipeline::Technique>> techniques;

	protected:
		BaseShape(Graphics& gfx, const GfxObject& parent, std::shared_ptr<Resource::IndexBuffer> indexBuffer = nullptr,
			std::shared_ptr<Resource::VertexBuffer> vertexBuffer = nullptr);
		BaseShape(const BaseShape&) = delete;
		BaseShape& operator=(const BaseShape&) = delete;
		virtual ~BaseShape() = default;

		inline void SetIndexBuffer(std::shared_ptr<Resource::IndexBuffer> index) noexcept { indexBuffer = std::move(index); }
		inline void SetVertexBuffer(std::shared_ptr<Resource::VertexBuffer> vertex) noexcept { vertexBuffer = std::move(vertex); }
		inline void SetTopology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY newTopology) noexcept { topology = Resource::Topology::Get(gfx, newTopology); }
		inline void SetTechniques(std::vector<std::shared_ptr<Pipeline::Technique>>&& newTechniques) noexcept { techniques = std::move(newTechniques); }

	public:
		constexpr UINT GetIndexCount() const noexcept { return indexBuffer->GetCount(); }

		inline void AddTechnique(std::shared_ptr<Pipeline::Technique> technique) noexcept { techniques.emplace_back(std::move(technique)); }
		inline void SetTopologyPlain(Graphics& gfx) noexcept { topology = Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); }
		virtual inline void SetTopologyMesh(Graphics& gfx) noexcept { topology = Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST); }

		void Bind(Graphics& gfx) noexcept;
		void Submit(Pipeline::RenderCommander& renderer) noexcept(!IS_DEBUG) override;
		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}