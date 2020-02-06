#include "Box.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Box::Box(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, BasicType::ColorFloat4 material,
		float width, float height, float length)
		: Object(position, name), sizes(width, height, length)
	{
		auto list = Primitive::Cube::Make();
		AddBind(Resource::VertexBuffer::Get(gfx, list.typeName, list.vertices));
		AddBind(Resource::IndexBuffer::Get(gfx, list.typeName, list.indices));

		auto vertexShader = Resource::VertexShader::Get(gfx, "PhongVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);
		AddBind(Resource::PixelShader::Get(gfx, "PhongPS.cso"));

		AddBind(Resource::InputLayout::Get(gfx, list.vertices.GetLayout(), bytecodeVS));
		AddBind(Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)); // Mesh: D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ
		AddBind(std::make_shared<Resource::ConstantTransformBufferVS>(gfx, *this));

		Resource::PhongPixelBuffer buffer;
		buffer.materialColor = material;
		buffer.specularIntensity = 0.6f;
		buffer.specularPower = 60.0f;
		AddBind(Resource::ConstantPixelBuffer<Resource::PhongPixelBuffer>::Get(gfx, name, buffer, 1U));

		UpdateTransformMatrix();
	}

	void Box::UpdateTransformMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&sizes)) *
			CreateTransformMatrix());
	}
}