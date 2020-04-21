#include "Ball.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Ball::Ball(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 material, unsigned int density, float radius)
		: BaseShape(gfx, *this), Object(position, name, radius)
	{
		auto list = Primitive::Sphere::MakeIco(density);
		AddBind(Resource::VertexBuffer::Get(gfx, list.typeName, list.vertices));
		AddBind(Resource::IndexBuffer::Get(gfx, list.typeName, list.indices));

		auto vertexShader = Resource::VertexShader::Get(gfx, "PhongVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);
		AddBind(Resource::PixelShader::Get(gfx, "PhongPS.cso"));
		AddBind(Resource::InputLayout::Get(gfx, list.vertices.GetLayout(), bytecodeVS));
		// Mesh: D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ

		Data::CBuffer::Phong buffer;
		buffer.materialColor = material;
		buffer.specularIntensity = 0.6f;
		buffer.specularPower = 60.0f;
		AddBind(Resource::ConstBufferPixel<Data::CBuffer::Phong>::Get(gfx, name, buffer, 1U));
	}
}