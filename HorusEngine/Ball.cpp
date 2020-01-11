#include "Ball.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Ball::Ball(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, BasicType::ColorFloat material, unsigned int density, float radius)
		: Object(position, name, radius)
	{

		auto list = Primitive::Sphere::MakeIco(density);
		AddBind(std::make_shared<Resource::VertexBuffer>(gfx, list.vertices));
		AddBind(std::make_shared<Resource::IndexBuffer>(gfx, list.indices));

		auto vertexShader = std::make_shared<Resource::VertexShader>(gfx, L"PhongVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);
		AddBind(std::make_shared<Resource::PixelShader>(gfx, L"PhongPS.cso"));

		AddBind(std::make_shared<Resource::InputLayout>(gfx, list.vertices.GetLayout().GetDXLayout(), bytecodeVS));

		AddBind(std::make_shared<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)); // Mesh: D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ


		AddBind(std::make_shared<Resource::ConstantTransformBuffer>(gfx, *this));

		Resource::ObjectConstantBuffer buffer;
		buffer.materialColor = material;
		buffer.specularIntensity = 0.6f;
		buffer.specularPower = 60.0f;
		AddBind(std::make_shared<Resource::ConstantPixelBuffer<Resource::ObjectConstantBuffer>>(gfx, buffer, 1U));
	}
}
