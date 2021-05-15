#include "GFX/Visual/OutlineMaskBlur.h"
#include "GFX/Resource/GfxResources.h"

namespace ZE::GFX::Visual
{
	OutlineMaskBlur::OutlineMaskBlur(Graphics& gfx, const std::string& tag,
		const ColorF3& color, const std::shared_ptr<Data::VertexLayout>& vertexLayout)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader));
		AddBind(std::move(vertexShader));

		GFX::Data::CBuffer::DCBLayout pixelCBbufferLayout;
		pixelCBbufferLayout.Add(DCBElementType::Color3, "solidColor");
		Data::CBuffer::DynamicCBuffer pixelCBuffer(std::move(pixelCBbufferLayout));
		pixelCBuffer["solidColor"] = color;
		pixelBuffer = Resource::ConstBufferExPixelCache::Get(gfx, tag, std::move(pixelCBuffer));
	}

	void OutlineMaskBlur::Bind(Graphics& gfx) const
	{
		pixelBuffer->Bind(gfx);
		IVisual::Bind(gfx);
	}
}