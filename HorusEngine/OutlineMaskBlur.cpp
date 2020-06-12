#include "OutlineMaskBlur.h"

namespace GFX::Visual
{
	OutlineMaskBlur::OutlineMaskBlur(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS.cso");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader->GetBytecode()));
		AddBind(std::move(vertexShader));

		GFX::Data::CBuffer::DCBLayout pixelCBbufferLayout;
		pixelCBbufferLayout.Add(DCBElementType::Color3, "solidColor");
		Data::CBuffer::DynamicCBuffer pixelCBuffer(std::move(pixelCBbufferLayout));
		pixelCBuffer["solidColor"] = std::move(color);
		pixelBuffer = Resource::ConstBufferExPixelCache::Get(gfx, tag, std::move(pixelCBuffer), 1U);
	}

	void OutlineMaskBlur::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		pixelBuffer->Accept(gfx, probe);
	}

	void OutlineMaskBlur::Bind(Graphics& gfx) noexcept
	{
		pixelBuffer->Bind(gfx);
		Effect::Bind(gfx);
	}
}