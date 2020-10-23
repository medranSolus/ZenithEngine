#include "OutlineMaskScale.h"
#include "GfxResources.h"

namespace GFX::Visual
{
	inline Data::CBuffer::DCBLayout OutlineMaskScale::MakeLayout() noexcept
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

	void OutlineMaskScale::UpdateTransform() noexcept
	{
		float scale = buffer["scale"];
		DirectX::XMFLOAT4X4 scaling;
		DirectX::XMStoreFloat4x4(&scaling, DirectX::XMMatrixScaling(scale, scale, scale));
		std::static_pointer_cast<Resource::ConstBufferTransformEx>(transformBuffer)->UpdateTransform(std::move(scaling));
	}

	OutlineMaskScale::OutlineMaskScale(Graphics& gfx, const std::string& tag, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout)
		: buffer(MakeLayout())
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader));
		AddBind(std::move(vertexShader));
		buffer["scale"] = 1.3f;

		GFX::Data::CBuffer::DCBLayout cbufferLayout;
		cbufferLayout.Add(DCBElementType::Color3, "solidColor");
		Data::CBuffer::DynamicCBuffer cbuffer(std::move(cbufferLayout));
		cbuffer["solidColor"] = std::move(color);
		pixelBuffer = Resource::ConstBufferExPixelCache::Get(gfx, tag, std::move(cbuffer), 8U);
	}

	void OutlineMaskScale::SetTransformBuffer(Graphics& gfx, const GfxObject& parent)
	{
		transformBuffer = std::make_shared<Resource::ConstBufferTransformEx>(gfx, parent);
		UpdateTransform();
	}

	bool OutlineMaskScale::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		dirty = probe.VisitObject(buffer);
		return dirty || pixelBuffer->Accept(gfx, probe);
	}

	void OutlineMaskScale::Bind(Graphics& gfx)
	{
		if (dirty)
			UpdateTransform();
		pixelBuffer->Bind(gfx);
		Effect::Bind(gfx);
	}
}