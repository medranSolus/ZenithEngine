#include "OutlineMaskOffset.h"

namespace GFX::Visual
{
	OutlineMaskOffset::OutlineMaskOffset(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "OffsetVS.cso");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader->GetBytecode()));
		AddBind(std::move(vertexShader));

		GFX::Data::CBuffer::DCBLayout vertexCBbufferLayout;
		vertexCBbufferLayout.Add(DCBElementType::Float, "offset");
		Data::CBuffer::DynamicCBuffer vertexCBuffer(std::move(vertexCBbufferLayout));
		vertexCBuffer["offset"] = 0.11f;
		vertexBuffer = Resource::ConstBufferExVertexCache::Get(gfx, tag, std::move(vertexCBuffer), 1U);

		GFX::Data::CBuffer::DCBLayout pixelCBbufferLayout;
		pixelCBbufferLayout.Add(DCBElementType::Color3, "solidColor");
		Data::CBuffer::DynamicCBuffer pixelCBuffer(std::move(pixelCBbufferLayout));
		pixelCBuffer["solidColor"] = std::move(color);
		pixelBuffer = Resource::ConstBufferExPixelCache::Get(gfx, tag, std::move(pixelCBuffer), 1U);
	}

	void OutlineMaskOffset::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		vertexBuffer->Accept(gfx, probe);
		pixelBuffer->Accept(gfx, probe);
	}

	void OutlineMaskOffset::Bind(Graphics& gfx) noexcept
	{
		vertexBuffer->Bind(gfx);
		pixelBuffer->Bind(gfx);
		Effect::Bind(gfx);
	}
}