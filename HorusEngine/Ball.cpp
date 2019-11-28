#include "Ball.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Object
{
	Ball::Ball(Graphics & gfx, float x0, float y0, float z0, unsigned int density, float radius)
		: x(x0), y(y0), z(z0)
	{
		if (!IsStaticInit())
		{
			auto list = Primitive::Sphere::MakeIco<Primitive::Vertex>(density);
			AddStaticBind(std::make_unique<Resource::VertexBuffer>(gfx, list.vertices));
			AddStaticIndexBuffer(std::make_unique<Resource::IndexBuffer>(gfx, list.indices));

			auto vertexShader = std::make_unique<Resource::VertexShader>(gfx, L"VSBasic.cso");
			auto bytecodeVS = vertexShader->GetBytecode();
			AddStaticBind(std::move(vertexShader));
			AddStaticBind(std::make_unique<Resource::PixelShader>(gfx, L"PSTriangle.cso"));

			struct ColorBuffer
			{
				Primitive::Color faceColors[20];
			} colorBuffer;
			for (unsigned int i = 0; i < 20; ++i)
				colorBuffer.faceColors[i] = randColor();
			AddStaticBind(std::make_unique<Resource::ConstantPixelBuffer<ColorBuffer>>(gfx, colorBuffer));

			const std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc =
			{
				{ "Position", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			AddStaticBind(std::make_unique<Resource::InputLayout>(gfx, inputDesc, bytecodeVS));

			AddStaticBind(std::make_unique<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)); // Mesh: D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ
		}
		AddBind(std::make_unique<Resource::ConstantTransformBuffer>(gfx, *this));
	}

	void Ball::Update(float dX, float dY, float dZ, float angleDZ, float angleDX, float angleDY) noexcept
	{
		x += dX;
		y += dY;
		z += dZ;
		angleZ = wrap2Pi(angleZ + angleDZ);
		angleX = wrap2Pi(angleX + angleDX);
		angleY = wrap2Pi(angleY + angleDY);
	}

	DirectX::XMMATRIX Ball::GetTransformMatrix() const noexcept
	{
		return DirectX::XMMatrixRotationRollPitchYaw(angleX, angleY, angleZ) * DirectX::XMMatrixTranslation(x, y, z);
	}
}
