#pragma once
#include "GFX/Object.h"
#include "Mesh.h"

namespace GFX::Shape
{
	class ModelNode : public Object
	{
		U64 id;
		Float4x4 baseTransform;
		mutable std::shared_ptr<Float4x4> currentTransform = nullptr;
		std::vector<std::unique_ptr<ModelNode>> children;
		std::vector<std::shared_ptr<Mesh>> meshes; // TODO: Change to id
		bool isMesh = false;

	public:
		ModelNode(U64 id, std::string&& name, std::vector<std::shared_ptr<Mesh>>&& nodeMeshes,
			const Matrix& nodeTransform) noexcept;
		ModelNode(ModelNode&&) = default;
		ModelNode(const ModelNode&) = default;
		ModelNode& operator=(ModelNode&&) = default;
		ModelNode& operator=(const ModelNode&) = default;
		virtual ~ModelNode() = default;

		constexpr bool IsMesh() const noexcept { return isMesh; }
		constexpr U64 GetID() const noexcept { return id; }

		bool HasChildren() const noexcept { return children.size(); }
		void ReserveChildren(U64 capacity) noexcept { children.reserve(capacity); }
		void AddChild(std::unique_ptr<ModelNode>&& child) noexcept { assert(child); children.emplace_back(std::move(child)); }

		void Submit(U64 channelFilter) const noexcept override { Submit(channelFilter, Math::XMMatrixIdentity()); }

		void SetOutline() noexcept override;
		void DisableOutline() noexcept override;
		void Submit(U64 channelFilter, const Matrix& higherTransform) const noexcept;
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		bool Accept(Graphics& gfx, Probe::ModelProbe& probe) noexcept override;

		void SetMesh(Graphics& gfx, bool meshOnly) noexcept;
	};
}