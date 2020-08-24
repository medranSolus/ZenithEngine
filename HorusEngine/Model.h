#pragma once
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

		static std::shared_ptr<Mesh> ParseMesh(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& path, aiMesh& mesh, std::vector<std::shared_ptr<Visual::Material>>& materials);

		std::unique_ptr<ModelNode> ParseNode(const aiNode& node, unsigned long long& id) noexcept(!IS_DEBUG);

	public:
		Model(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& file, const DirectX::XMFLOAT3& position = { 0.0f,0.0f,0.0f }, const std::string& modelName = "Model", float scale = 1.0f);
		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		virtual ~Model() = default;

		inline void Submit() noexcept override { root->Submit(); }

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

		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { root->Accept(gfx, probe); }
		inline void Accept(Graphics& gfx, Probe::ModelProbe& probe) noexcept override { probe.Visit(gfx, *this, *root); }

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