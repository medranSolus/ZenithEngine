#pragma once
#include "ConstBufferTransform.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

namespace GFX::Light::Volume
{
	class IVolume : public GfxObject
	{
		GfxResPtr<Resource::ConstBufferTransform> transformBuffer;

		static float GetVolume(const Data::CBuffer::DynamicCBuffer& lightBuffer);

	protected:
		GfxResPtr<Resource::IndexBuffer> indexBuffer;
		GfxResPtr<Resource::VertexBuffer> vertexBuffer;

		virtual void UpdateTransform(float volume, const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept = 0;

		inline IVolume(Graphics& gfx) : transformBuffer(gfx, *this) {}

	public:
		virtual ~IVolume() = default;

		inline UINT GetIndexCount() const noexcept { return indexBuffer->GetCount(); }

		void Update(const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept;
		void Bind(Graphics& gfx);
	};
}