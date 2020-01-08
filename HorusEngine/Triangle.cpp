#include "Triangle.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Triangle::Triangle(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, float down, float left, float right)
		: Object(position, name)
	{
		const float leftPow2 = left * left;
		const float vertex3X = (right * right - leftPow2 - down * down) / (-2.0f * down);
		const float centerX = (vertex3X + down) / 3;
		const float centerY = sqrtf(leftPow2 - vertex3X * vertex3X) / 3;
		std::mt19937_64 engine(std::random_device{}());

		BasicType::VertexDataBuffer vertices(std::move(BasicType::VertexLayout{}
			.Append(VertexAttribute::Position3D)
			.Append(VertexAttribute::ColorFloat)));

		vertices.EmplaceBack(DirectX::XMFLOAT3(-centerX, -centerY, 0.0f), randColor(engine));
		vertices.EmplaceBack(DirectX::XMFLOAT3(vertex3X - centerX, 2.0f * centerY, 0.0f), randColor(engine));
		vertices.EmplaceBack(DirectX::XMFLOAT3(down - centerX, -centerY, 0.0f), randColor(engine));

		AddBind(std::make_unique<Resource::VertexBuffer>(gfx, vertices));

		if (!IsStaticInit())
		{
			auto vertexShader = std::make_unique<Resource::VertexShader>(gfx, L"ColorBlendVS.cso");
			auto bytecodeVS = vertexShader->GetBytecode();
			AddStaticBind(std::move(vertexShader));

			AddStaticBind(std::make_unique<Resource::PixelShader>(gfx, L"ColorBlendPS.cso"));

			AddStaticIndexBuffer(std::make_unique<Resource::IndexBuffer>(gfx, std::move(std::vector<unsigned int>({ 0, 1, 2 }))));

			AddStaticBind(std::make_unique<Resource::InputLayout>(gfx, vertices.GetLayout().GetDXLayout(), bytecodeVS));

			AddStaticBind(std::make_unique<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		}
		AddBind(std::make_unique<Resource::ConstantTransformBuffer>(gfx, *this));
	}
}
