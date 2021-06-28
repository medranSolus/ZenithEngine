#pragma once
#include "Source.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SourceDirectBuffer : public Source
	{
		static_assert(std::is_base_of_v<Resource::IBufferResource, T>, "SourceDirectBuffer target type must be a IBufferResource type!");

		GfxResPtr<T>& buffer;
		bool linked = false;

	public:
		constexpr SourceDirectBuffer(std::string&& name, GfxResPtr<T>& buffer)
			: Source(std::forward<std::string>(name)), buffer(buffer) {}
		SourceDirectBuffer(SourceDirectBuffer&&) = default;
		SourceDirectBuffer(const SourceDirectBuffer&) = default;
		SourceDirectBuffer& operator=(SourceDirectBuffer&&) = default;
		SourceDirectBuffer& operator=(const SourceDirectBuffer&) = default;
		virtual ~SourceDirectBuffer() = default;

		static std::unique_ptr<Source> Make(std::string&& name, GfxResPtr<T>& buffer);

		constexpr GfxResPtr<Resource::IBufferResource> LinkBuffer() override;
	};

#pragma region Functions
	template<typename T>
	std::unique_ptr<Source> SourceDirectBuffer<T>::Make(std::string&& name, GfxResPtr<T>& buffer)
	{
		return std::make_unique<SourceDirectBuffer>(std::forward<std::string>(name), buffer);
	}

	template<typename T>
	constexpr GfxResPtr<Resource::IBufferResource> SourceDirectBuffer<T>::LinkBuffer()
	{
		if (linked)
			throw ZE_RGC_EXCEPT("Trying to link Source \"" + GetName() + "\" second time!");
		linked = true;
		return buffer.CastStatic<Resource::IBufferResource>();
	}
#pragma endregion
}