#include "SolidGlobe.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	SolidGlobe::SolidGlobe(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, BasicType::ColorFloat material,
		unsigned int latitudeDensity, unsigned int longitudeDensity, float width, float height, float length)
		: Object(position, name), sizes(width, height, length)
	{
		auto list = Primitive::Sphere::MakeSolidUV(latitudeDensity, longitudeDensity);
		AddBind(Resource::VertexBuffer::Get(gfx, list.typeName, list.vertices));
		AddBind(Resource::IndexBuffer::Get(gfx, list.typeName, list.indices));

		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);
		AddBind(Resource::PixelShader::Get(gfx, "SolidPS.cso"));

		AddBind(Resource::InputLayout::Get(gfx, list.vertices.GetLayout(), bytecodeVS));
		AddBind(Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)); // Mesh: D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ
		AddBind(std::make_shared<Resource::ConstantTransformBuffer>(gfx, *this));

		Resource::SolidPixelBuffer buffer{ material };
		AddBind(Resource::ConstantPixelBuffer<Resource::SolidPixelBuffer>::Get(gfx, name, buffer));

		UpdateScalingMatrix();
	}

	void SolidGlobe::UpdateScalingMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(scaling.get(),
			DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&sizes)) *
			DirectX::XMMatrixScaling(scale, scale, scale));
	}
}