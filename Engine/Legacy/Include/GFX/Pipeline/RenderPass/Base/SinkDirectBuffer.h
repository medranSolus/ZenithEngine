#pragma once
#include "Sink.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SinkDirectBuffer : public Sink
	{
		static_assert(std::is_base_of_v<Resource::IBufferResource, T>, "SinkDirectBuffer target type must be a IBufferResource type!");

		GfxResPtr<T>& target;

	public:
		constexpr SinkDirectBuffer(std::string&& registeredName, GfxResPtr<T>& buffer)
			: Sink(std::forward<std::string>(registeredName)), target(buffer) {}
		SinkDirectBuffer(SinkDirectBuffer&&) = default;
		SinkDirectBuffer(const SinkDirectBuffer&) = default;
		SinkDirectBuffer& operator=(SinkDirectBuffer&&) = default;
		SinkDirectBuffer& operator=(const SinkDirectBuffer&) = default;
		virtual ~SinkDirectBuffer() = default;

		static std::unique_ptr<Sink> Make(std::string&& registeredName, GfxResPtr<T>& resource);

		constexpr void Bind(Source& source) override;
	};

#pragma region Functions
	template<typename T>
	std::unique_ptr<Sink> SinkDirectBuffer<T>::Make(std::string&& registeredName, GfxResPtr<T>& resource)
	{
		return std::make_unique<SinkDirectBuffer>(std::forward<std::string>(registeredName), resource);
	}

	template<typename T>
	constexpr void SinkDirectBuffer<T>::Bind(Source& source)
	{
		auto basePtr = source.LinkBuffer();
		auto ptr = basePtr.CastDynamic<T>();
		if (ptr == nullptr)
			throw ZE_RGC_EXCEPT("Binding Sink \"" + GetRegisteredName() + "\" of type {" + typeid(T).name() +
				"} to Source \"" + GetPassPathString() + "." + GetSourceName() + "\" of incompatible type {" + typeid(basePtr).name());
		target = std::move(ptr);
		linked = true;
	}
#pragma endregion
}