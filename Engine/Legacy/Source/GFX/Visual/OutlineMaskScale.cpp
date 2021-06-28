#include "GFX/Visual/OutlineMaskScale.h"
#include "GFX/Resource/GfxResources.h"

namespace ZE::GFX::Visual
{
	Data::CBuffer::DCBLayout OutlineMaskScale::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Float, "scale");
			initNeeded = false;
		}
		return layout;
	}

	void OutlineMaskScale::UpdateTransform() const noexcept
	{
		float scale = buffer["scale"];
		Float4x4 scaling;
		Math::XMStoreFloat4x4(&scaling, Math::XMMatrixScaling(scale, scale, scale));
		transformBuffer.CastStatic<Resource::ConstBufferTransformEx>()->UpdateTransform(std::move(scaling));
		dirty = false;
	}

	OutlineMaskScale::OutlineMaskScale(Graphics& gfx, const std::string& tag,
		const ColorF3& color, const std::shared_ptr<Data::VertexLayout>& vertexLayout)
		: buffer(MakeLayout())
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader));
		AddBind(std::move(vertexShader));
		buffer["scale"] = 1.3f;

		GFX::Data::CBuffer::DCBLayout cbufferLayout;
		cbufferLayout.Add(DCBElementType::Color3, "solidColor");
		Data::CBuffer::DynamicCBuffer cbuffer(std::move(cbufferLayout));
		cbuffer["solidColor"] = color;
		pixelBuffer = Resource::ConstBufferExPixelCache::Get(gfx, tag, std::move(cbuffer));
	}

	void OutlineMaskScale::SetTransformBuffer(Graphics& gfx, const GfxObject& parent) const
	{
		transformBuffer = GfxResPtr<Resource::ConstBufferTransformEx>(gfx, parent);
		UpdateTransform();
	}

	void OutlineMaskScale::Bind(Graphics& gfx) const
	{
		if (dirty)
			UpdateTransform();
		pixelBuffer->Bind(gfx);
		IVisual::Bind(gfx);
	}
}