#include "Box.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Object
{
	Box::Box(Graphics& gfx, float x0, float y0, float z0, float rotationR) : r(rotationR), moveX(x0), moveY(y0), moveZ(z0)
	{
		std::mt19937 engine(std::random_device{}());
		dRotX = rand(-M_PI_2, M_PI_2, engine);
		dRotY = rand(-M_PI_2, M_PI_2, engine);
		dRotZ = rand(-M_PI_2, M_PI_2, engine);
		dMoveX = rand(-M_PI_2, M_PI_2, engine);
		dMoveY = rand(-M_PI_2, M_PI_2, engine);
		dMoveZ = rand(-M_PI_2, M_PI_2, engine);
		if (!IsStaticInit())
		{
			auto list = Primitive::Cube::Make<Primitive::Vertex>();
			AddStaticBind(std::make_unique<Resource::VertexBuffer>(gfx, list.vertices));
			AddStaticIndexBuffer(std::make_unique<Resource::IndexBuffer>(gfx, list.indices));
			
			auto vertexShader = std::make_unique<Resource::VertexShader>(gfx, L"VSBasic.cso");
			auto bytecodeVS = vertexShader->GetBytecode();
			AddStaticBind(std::move(vertexShader));
			AddStaticBind(std::make_unique<Resource::PixelShader>(gfx, L"PSRectangle.cso"));

			struct ColorBuffer
			{
				Primitive::Color faceColors[6];
			} colorBuffer;
			for (unsigned int i = 0; i < 6; ++i)
				colorBuffer.faceColors[i] = randColor();
			AddStaticBind(std::make_unique<Resource::ConstantPixelBuffer<ColorBuffer>>(gfx, colorBuffer));

			const std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc =
			{
				{ "Position", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			AddStaticBind(std::make_unique<Resource::InputLayout>(gfx, inputDesc, bytecodeVS));

			AddStaticBind(std::make_unique<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)); // Mesh: D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ
		}
		AddBind(std::make_unique<Resource::ConstantTransformBuffer>(gfx, *this));
	}

	void Box::Update(float dX, float dY, float dZ, float angleDZ, float angleDX, float angleDY) noexcept
	{
		rotZ = wrap2Pi(rotZ + dRotZ * angleDZ);
		rotX = wrap2Pi(rotX + dRotX * angleDX);
		rotY = wrap2Pi(rotY + dRotY * angleDY);
		moveX = wrap2Pi(moveX + dMoveX * dX);
		moveY = wrap2Pi(moveY + dMoveY * dY);
		moveZ = wrap2Pi(moveZ + dMoveZ * dZ);
	}

	DirectX::XMMATRIX Box::GetTransformMatrix() const noexcept
	{
		return DirectX::XMMatrixRotationRollPitchYaw(rotX, rotY, rotZ) * // Rotation around center
			DirectX::XMMatrixTranslation(r, 0.0f, 0.0f) *				 // Move to side of rotation sphere
			DirectX::XMMatrixRotationRollPitchYaw(moveX, moveY, moveZ) * // Rotate around sphere
			DirectX::XMMatrixTranslation(0.0f, 0.0f, 10.0f);			 // Move a bit to the back
	}
}