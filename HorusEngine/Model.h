#pragma once
#include "ModelParams.h"
#include "ModelNode.h"
#include "Visuals.h"
#include "BasicException.h"

namespace GFX::Shape
{
	class Model : public IObject
	{
		std::string name = "";
		std::unique_ptr<ModelNode> root = nullptr;
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::vector<std::shared_ptr<Visual::Material>> materials; // TODO: Place inside codex
		bool isOutline = false;

		std::shared_ptr<Mesh> ParseMesh(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& path, aiMesh& mesh);
		std::unique_ptr<ModelNode> ParseNode(const aiNode& node, uint64_t& id);

	public:
		inline Model(Model&& model) noexcept { *this = std::forward<Model&&>(model); }

		Model(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& file, const ModelParams& params);
		Model& operator=(Model&& model) noexcept;
		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		virtual ~Model() = default;

		constexpr bool IsOutline() const noexcept { return isOutline; }
		inline void Submit(uint64_t channelFilter) noexcept override { root->Submit(channelFilter); }

		inline const DirectX::XMFLOAT3& GetAngle() const noexcept override { return root->GetAngle(); }
		inline void SetAngle(const DirectX::XMFLOAT3& meshAngle) noexcept override { root->SetAngle(meshAngle); }

		inline float GetScale() const noexcept { return root->GetScale(); }
		inline void SetScale(float newScale) noexcept { root->SetScale(newScale); }

		inline const DirectX::XMFLOAT3& GetPos() const noexcept override { return root->GetPos(); }
		inline void SetPos(const DirectX::XMFLOAT3& position) noexcept override { root->SetPos(position); }

		inline const std::string& GetName() const noexcept override { return name; }
		inline void SetName(const std::string& newName) noexcept override { name = newName; }

		inline void Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle) noexcept override { root->Update(delta, deltaAngle); }
		inline void UpdatePos(const DirectX::XMFLOAT3& delta) noexcept override { root->UpdatePos(delta); }
		inline void UpdateAngle(const DirectX::XMFLOAT3& deltaAngle) noexcept override { root->UpdateAngle(deltaAngle); }

		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return root->Accept(gfx, probe); }
		inline bool Accept(Graphics& gfx, Probe::ModelProbe& probe) noexcept override { return probe.Visit(gfx, *this, *root); }

		void SetOutline() noexcept override;
		void DisableOutline() noexcept override;

		class ModelException : public Exception::BasicException
		{
			std::string error;

		public:
			inline ModelException(unsigned int line, const char* file, const std::string& error) noexcept
				: BasicException(line, file), error(error) {}
			ModelException(const ModelException&) = default;
			ModelException& operator=(const ModelException&) = default;
			virtual ~ModelException() = default;

			inline const char* GetType() const noexcept override { return "Model Exception"; }
			constexpr const std::string& GetErrorString() const noexcept { return error; }

			const char* what() const noexcept override;
		};
	};
}