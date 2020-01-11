#include "Globe.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Globe::Globe(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, BasicType::ColorFloat material,
		unsigned int latitudeDensity, unsigned int longitudeDensity, float width, float height, float length)
		: Object(position, name), sizes(width, height, length)
	{
		auto list = Primitive::Sphere::MakeUV(latitudeDensity, longitudeDensity);
		AddBind(std::make_shared<Resource::VertexBuffer>(gfx, list.vertices));
		AddBind(std::make_shared<Resource::IndexBuffer>(gfx, list.indices));

		auto vertexShader = std::make_shared<Resource::VertexShader>(gfx, L"PhongVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);
		AddBind(std::make_shared<Resource::PixelShader>(gfx, L"PhongPS.cso"));

		AddBind(std::make_shared<Resource::InputLayout>(gfx, list.vertices.GetLayout().GetDXLayout(), bytecodeVS));

		AddBind(std::make_shared<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)); // Mesh: D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ


		AddBind(std::make_shared<Resource::ConstantTransformBuffer>(gfx, *this));

		Resource::ObjectConstantBuffer buffer;
		buffer.materialColor = material;
		buffer.specularIntensity = 0.6f;
		buffer.specularPower = 60.0f;
		AddBind(std::make_shared<Resource::ConstantPixelBuffer<Resource::ObjectConstantBuffer>>(gfx, buffer, 1U));

		UpdateScalingMatrix();
	}

	void Globe::UpdateScalingMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(scaling.get(),
			DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&sizes)) *
			DirectX::XMMatrixScaling(scale, scale, scale));
	}
}
