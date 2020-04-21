#include "Globe.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Globe::Globe(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 material,
		unsigned int latitudeDensity, unsigned int longitudeDensity, float width, float height, float length)
		: BaseShape(gfx, *this), Object(position, name), sizes(width, height, length)
	{
		auto list = Primitive::Sphere::MakeUV(latitudeDensity, longitudeDensity);
		AddBind(Resource::VertexBuffer::Get(gfx, list.typeName, list.vertices));
		AddBind(Resource::IndexBuffer::Get(gfx, list.typeName, list.indices));

		auto vertexShader = Resource::VertexShader::Get(gfx, "PhongVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);
		AddBind(Resource::PixelShader::Get(gfx, "PhongPS.cso"));
		AddBind(Resource::InputLayout::Get(gfx, list.vertices.GetLayout(), bytecodeVS));
		// Mesh: D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ

		Data::CBuffer::Phong buffer;
		buffer.materialColor = material;
		buffer.specularIntensity = 0.6f;
		buffer.specularPower = 60.0f;
		AddBind(Resource::ConstBufferPixel<Data::CBuffer::Phong>::Get(gfx, name, buffer, 1U));

		UpdateTransformMatrix();
	}

	void Globe::UpdateTransformMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&sizes)) *
			CreateTransformMatrix());
	}
}