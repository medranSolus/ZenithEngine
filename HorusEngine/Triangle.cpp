#include "Triangle.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Object
{
	Triangle::Triangle(Graphics & gfx, float x0, float y0, float z0, float down, float left, float right) : x(x0), y(y0), z(z0)
	{
		const float leftPow2 = left * left;
		const float vertex3X = (right * right - leftPow2 - down * down) / (-2.0f * down);
		const float centerX = (vertex3X + down) / 3;
		const float centerY = sqrtf(leftPow2 - vertex3X * vertex3X) / 3;
		const std::vector<Primitive::VertexColor> vertices =
		{
			{ { -centerX, -centerY, 1.0f }, randColor() },
			{ { vertex3X - centerX, 2.0f * centerY, 1.0f }, randColor() },
			{ { down - centerX, -centerY, 1.0f }, randColor() },
		};
		AddBind(std::make_unique<Resource::VertexBuffer>(gfx, vertices));

		if (!IsStaticInit())
		{
			auto vertexShader = std::make_unique<Resource::VertexShader>(gfx, L"VSColor.cso");
			auto bytecodeVS = vertexShader->GetBytecode();
			AddStaticBind(std::move(vertexShader));

			AddStaticBind(std::make_unique<Resource::PixelShader>(gfx, L"PSBasic.cso"));

			const std::vector<unsigned int> indices =
			{
				0, 1, 2
			};
			AddStaticIndexBuffer(std::make_unique<Resource::IndexBuffer>(gfx, indices));

			const std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc =
			{
				{ "Position", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "Color", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			AddStaticBind(std::make_unique<Resource::InputLayout>(gfx, inputDesc, bytecodeVS));

			AddStaticBind(std::make_unique<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		}
		AddBind(std::make_unique<Resource::ConstantTransformBuffer>(gfx, *this));
	}

	void Triangle::Update(float dX, float dY, float dZ, float angleDZ, float angleDX, float angleDY) noexcept
	{
		x += dX;
		y += dY;
		z += dZ;
		angleZ = wrap2Pi(this->angleZ + angleDZ);
	}

	DirectX::XMMATRIX Triangle::GetTransformMatrix() const noexcept
	{
		return DirectX::XMMatrixRotationZ(angleZ) * DirectX::XMMatrixTranslation(x, y, z);
	}
}
