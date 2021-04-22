#include "GFX/Visual/OutlineMaskOffset.h"
#include "GFX/Resource/GfxResources.h"

namespace GFX::Visual
{
	OutlineMaskOffset::OutlineMaskOffset(Graphics& gfx, const std::string& tag,
		const ColorF3& color, const std::shared_ptr<Data::VertexLayout>& vertexLayout)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "OffsetVS");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader));
		AddBind(std::move(vertexShader));

		GFX::Data::CBuffer::DCBLayout vertexCBbufferLayout;
		vertexCBbufferLayout.Add(DCBElementType::Float, "offset");
		Data::CBuffer::DynamicCBuffer vertexCBuffer(std::move(vertexCBbufferLayout));
		vertexCBuffer["offset"] = 0.11f;
		vertexBuffer = Resource::ConstBufferExVertexCache::Get(gfx, tag, std::move(vertexCBuffer), 2);

		GFX::Data::CBuffer::DCBLayout pixelCBbufferLayout;
		pixelCBbufferLayout.Add(DCBElementType::Color3, "solidColor");
		Data::CBuffer::DynamicCBuffer pixelCBuffer(std::move(pixelCBbufferLayout));
		pixelCBuffer["solidColor"] = color;
		pixelBuffer = Resource::ConstBufferExPixelCache::Get(gfx, tag, std::move(pixelCBuffer));
	}

	void OutlineMaskOffset::Bind(Graphics& gfx) const
	{
		vertexBuffer->Bind(gfx);
		pixelBuffer->Bind(gfx);
		IVisual::Bind(gfx);
	}
}