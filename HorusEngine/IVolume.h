#pragma once
#include "ConstBufferTransform.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "PixelShader.h"

namespace GFX::Light::Volume
{
	class IVolume : public GfxObject
	{
		std::shared_ptr<Resource::ConstBufferTransform> transformBuffer = nullptr;

		static float GetVolume(const Data::CBuffer::DynamicCBuffer& lightBuffer);

	protected:
		std::shared_ptr<Resource::IndexBuffer> indexBuffer = nullptr;
		std::shared_ptr<Resource::VertexBuffer> vertexBuffer = nullptr;
		std::shared_ptr<Resource::PixelShader> pixelShader = nullptr;

		inline IVolume(Graphics& gfx) { transformBuffer = std::make_shared<Resource::ConstBufferTransform>(gfx, *this); }

	public:
		inline IVolume(IVolume&& volume) noexcept { *this = std::forward<IVolume>(volume); }
		IVolume& operator=(IVolume&& volume) noexcept;
		virtual ~IVolume() = default;

		inline UINT GetIndexCount() const noexcept { return indexBuffer->GetCount(); }

		void Bind(Graphics& gfx, const Data::CBuffer::DynamicCBuffer& lightBuffer);
	};
}