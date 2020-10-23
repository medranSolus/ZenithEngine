#pragma once
#include "ConstBufferTransform.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

namespace GFX::Light::Volume
{
	class IVolume : public GfxObject
	{
		std::shared_ptr<Resource::ConstBufferTransform> transformBuffer = nullptr;

		static float GetVolume(const Data::CBuffer::DynamicCBuffer& lightBuffer);

	protected:
		std::shared_ptr<Resource::IndexBuffer> indexBuffer = nullptr;
		std::shared_ptr<Resource::VertexBuffer> vertexBuffer = nullptr;

		virtual void UpdateTransform(float volume, const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept = 0;

		inline IVolume(Graphics& gfx) { transformBuffer = std::make_shared<Resource::ConstBufferTransform>(gfx, *this); }

	public:
		virtual ~IVolume() = default;

		inline UINT GetIndexCount() const noexcept { return indexBuffer->GetCount(); }

		void Update(const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept;
		void Bind(Graphics& gfx);
	};
}