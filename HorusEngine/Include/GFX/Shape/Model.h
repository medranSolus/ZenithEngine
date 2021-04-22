#pragma once
#include "ModelParams.h"
#include "ModelNode.h"
#include "GFX/Visual/Visuals.h"

namespace GFX::Shape
{
	class Model : public IObject
	{
		std::unique_ptr<std::string> name = nullptr;
		std::unique_ptr<ModelNode> root = nullptr;
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::vector<std::shared_ptr<Visual::Material>> materials; // TODO: Place inside codex
		bool isOutline = false;

		std::shared_ptr<Mesh> ParseMesh(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& path, aiMesh& mesh);
		std::unique_ptr<ModelNode> ParseNode(const aiNode& node, U64& id);

	public:
		Model(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& file, const ModelParams& params);
		Model(Model&&) = default;
		Model(const Model&) = delete;
		Model& operator=(Model&&) = default;
		Model& operator=(const Model&) = delete;
		virtual ~Model() = default;

		constexpr bool IsOutline() const noexcept { return isOutline; }
		constexpr void Submit(U64 channelFilter) const noexcept override { root->Submit(channelFilter); }

		constexpr const Float3& GetAngle() const noexcept override { return root->GetAngle(); }
		constexpr void SetAngle(const Float3& meshAngle) noexcept override { root->SetAngle(meshAngle); }

		constexpr float GetScale() const noexcept { return root->GetScale(); }
		constexpr void SetScale(float newScale) noexcept { root->SetScale(newScale); }

		constexpr const Float3& GetPos() const noexcept override { return root->GetPos(); }
		constexpr void SetPos(const Float3& position) noexcept override { root->SetPos(position); }

		const std::string& GetName() const noexcept override { return *name; }
		void SetName(const std::string& newName) noexcept override { *name = newName; }

		constexpr void Update(const Float3& delta, const Float3& deltaAngle) noexcept override { root->Update(delta, deltaAngle); }
		constexpr void UpdatePos(const Float3& delta) noexcept override { root->UpdatePos(delta); }
		constexpr void UpdateAngle(const Float3& deltaAngle) noexcept override { root->UpdateAngle(deltaAngle); }

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return root->Accept(gfx, probe); }
		bool Accept(Graphics& gfx, Probe::ModelProbe& probe) noexcept override { return probe.Visit(gfx, *this, *root); }

		void SetOutline() noexcept override;
		void DisableOutline() noexcept override;
	};
}