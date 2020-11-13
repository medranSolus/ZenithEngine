#pragma once
#include "Source.h"

namespace GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SourceDirectBuffer : public Source
	{
		static_assert(std::is_base_of_v<Resource::IBufferResource, T>, "SourceDirectBuffer target type must be a IBufferResource type!");

		GfxResPtr<T>& buffer;
		bool linked = false;

	public:
		inline SourceDirectBuffer(const std::string& name, GfxResPtr<T>& buffer) : Source(name), buffer(buffer) {}
		virtual ~SourceDirectBuffer() = default;

		static inline std::unique_ptr<Source> Make(const std::string& name, GfxResPtr<T>& buffer) { return std::make_unique<SourceDirectBuffer>(name, buffer); }

		GfxResPtr<Resource::IBufferResource> LinkBuffer() override;
	};

	template<typename T>
	GfxResPtr<Resource::IBufferResource> SourceDirectBuffer<T>::LinkBuffer()
	{
		if (linked)
			throw RGC_EXCEPT("Trying to link Source \"" + GetName() + "\" second time!");
		linked = true;
		return buffer.CastStatic<Resource::IBufferResource>();
	}
}