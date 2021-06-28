#pragma once
#include "GFX/Resource/ConstBufferTransform.h"
#include "GFX/Resource/IndexBuffer.h"
#include "GFX/Resource/VertexBuffer.h"

namespace ZE::GFX::Light::Volume
{
	class IVolume : public GfxObject
	{
		GfxResPtr<Resource::ConstBufferTransform> transformBuffer;

		static float GetVolume(const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept;

	protected:
		GfxResPtr<Resource::IndexBuffer> indexBuffer;
		GfxResPtr<Resource::VertexBuffer> vertexBuffer;

		virtual void UpdateTransform(float volume, const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept = 0;

		IVolume(Graphics& gfx) : transformBuffer(gfx, *this) {}

	public:
		IVolume(const IVolume&) = delete;
		IVolume& operator=(const IVolume&) = delete;
		virtual ~IVolume() = default;

		constexpr U32 GetIndexCount() const noexcept { return indexBuffer->GetCount(); }

		void Update(const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept;
		void Bind(Graphics& gfx) const noexcept;
	};
}