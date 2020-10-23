#pragma once
#include "Object.h"
#include "Mesh.h"

namespace GFX::Shape
{
	class ModelNode : public Object
	{
		unsigned long long id;
		DirectX::XMFLOAT4X4 baseTransform;
		mutable std::shared_ptr<DirectX::XMFLOAT4X4> currentTransform = nullptr;
		std::vector<std::unique_ptr<ModelNode>> children;
		std::vector<std::shared_ptr<Mesh>> meshes;
		bool isMesh = false;

	public:
		ModelNode(unsigned long long id, const std::string& name, std::vector<std::shared_ptr<Mesh>>&& nodeMeshes,
			const DirectX::FXMMATRIX& nodeTransform) noexcept;
		ModelNode(const ModelNode&) = default;
		ModelNode& operator=(const ModelNode&) = default;
		virtual ~ModelNode() = default;

		constexpr bool IsMesh() const noexcept { return isMesh; }
		constexpr unsigned long long GetID() const noexcept { return id; }
		constexpr DirectX::XMFLOAT4X4* GetBaseTransform() noexcept { return &baseTransform; }

		inline bool HasChildren() const noexcept { return children.size(); }
		inline void ReserveChildren(size_t capacity) noexcept { children.reserve(capacity); }
		inline void AddChild(std::unique_ptr<ModelNode> child) noexcept(!IS_DEBUG) { assert(child); children.emplace_back(std::move(child)); }

		inline void Submit(uint64_t channelFilter) noexcept override { Submit(channelFilter, DirectX::XMMatrixIdentity()); }

		void SetOutline() noexcept override;
		void DisableOutline() noexcept override;
		void Submit(uint64_t channelFilter, const DirectX::FXMMATRIX& higherTransform) noexcept;
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
		bool Accept(Graphics& gfx, Probe::ModelProbe& probe) noexcept override;

		void SetMesh(Graphics& gfx, bool meshOnly) noexcept;
	};
}