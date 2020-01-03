#pragma once
#include "Object.h"
#include "Mesh.h"
#include "assimp/scene.h"

namespace GFX::Shape
{
	class Model : public Object
	{
		class Node
		{
			std::string name;
			std::vector<std::unique_ptr<Node>> children;
			std::vector<std::shared_ptr<Mesh>> meshes;
			DirectX::XMFLOAT4X4 transform;

		public:
			Node(const std::string & name, std::vector<std::shared_ptr<Mesh>> && meshes, const DirectX::FXMMATRIX & nodeTransform) noexcept;

			inline void ReserveChildren(size_t capacity) noexcept { children.reserve(capacity); }
			inline void AddChild(std::unique_ptr<Node> child) noexcept(!IS_DEBUG)
			{
				assert(child);
				children.emplace_back(std::move(child));
			}

			void Draw(Graphics & gfx, const DirectX::FXMMATRIX & higherTransform) const noexcept;
		};

		static unsigned long long modelCount;

		std::unique_ptr<Node> root = nullptr;
		std::vector<std::shared_ptr<Mesh>> meshes;
		float scale;

		std::unique_ptr<Node> ParseNode(const aiNode & node) noexcept;

	public:
		Model(Graphics & gfx, const std::string & file, const DirectX::XMFLOAT3 & position = { 0.0f,0.0f,0.0f }, const std::string & modelName = "Model_", float scale = 1.0f);

		static std::shared_ptr<Mesh> ParseMesh(Graphics & gfx, const aiMesh & mesh);

		void Draw(Graphics & gfx) const noexcept override;
	};
}
