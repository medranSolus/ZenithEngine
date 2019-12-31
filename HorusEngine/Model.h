#pragma once
#include "Mesh.h"
#include "assimp/scene.h"

namespace GFX::Object
{
	class Model
	{
		class Node
		{
			std::vector<std::unique_ptr<Node>> children;
			std::vector<std::shared_ptr<Mesh>> meshes;
			DirectX::XMFLOAT4X4 transform;

		public:
			Node(std::vector<std::shared_ptr<Mesh>> && meshes, const DirectX::FXMMATRIX & nodeTransform) noexcept;

			inline void ReserveChildren(size_t capacity) noexcept { children.reserve(capacity); }
			inline void AddChild(std::unique_ptr<Node> child) noexcept(!IS_DEBUG)
			{
				assert(child);
				children.emplace_back(std::move(child));
			}

			void Draw(Graphics & gfx, const DirectX::FXMMATRIX & higherTransform) const noexcept;
		};

		std::unique_ptr<Node> root = nullptr;
		std::vector<std::shared_ptr<Mesh>> meshes;
		DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT3 pos;
		float scale;

		std::unique_ptr<Node> ParseNode(const aiNode & node);

	public:
		Model(Graphics & gfx, const std::string & file, float x0 = 0.0f, float y0 = 0.0f, float z0 = 0.0f, float scale = 1.0f);

		constexpr DirectX::XMFLOAT3 GetPos() const noexcept { return pos; }
		constexpr void SetPos(DirectX::XMFLOAT3 position) noexcept { pos = position; }
		constexpr DirectX::XMFLOAT3 GetAngle() const noexcept { return angle; }
		constexpr void SetAngle(DirectX::XMFLOAT3 meshAngle) noexcept { angle = meshAngle; }

		static std::shared_ptr<Mesh> ParseMesh(Graphics & gfx, const aiMesh & mesh);

		void Update(float dX, float dY, float dZ, float angleDZ = 0.0f, float angleDX = 0.0f, float angleDY = 0.0f) noexcept;
		void Draw(Graphics & gfx) const noexcept;
	};
}
