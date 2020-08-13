#pragma once
#include "Source.h"

namespace GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SourceDirectBuffer : public Source
	{
		static_assert(std::is_base_of_v<Resource::IBufferResource, T>, "SourceDirectBuffer target type must be a IBufferResource type!");

		std::shared_ptr<T>& buffer;
		bool linked = false;

	public:
		inline SourceDirectBuffer(const std::string& name, std::shared_ptr<T>& buffer) : Source(name), buffer(buffer) {}
		virtual ~SourceDirectBuffer() = default;

		static inline std::unique_ptr<Source> Make(const std::string& name, std::shared_ptr<T>& buffer) { return std::make_unique<SourceDirectBuffer>(name, buffer); }

		std::shared_ptr<Resource::IBufferResource> LinkBuffer() override;
	};

	template<typename T>
	std::shared_ptr<Resource::IBufferResource> SourceDirectBuffer<T>::LinkBuffer()
	{
		if (linked)
			throw RGC_EXCEPT("Trying to link Source \"" + GetName() + "\" second time!");
		linked = true;
		return buffer;
	}
}