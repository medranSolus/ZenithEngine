#pragma once
#include "Sink.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SinkContainerBindable : public Sink
	{
		static_assert(std::is_base_of_v<GFX::Resource::IBindable, T>, "SinkContainerBindable target type must be a IBindable type!");

		std::vector<GfxResPtr<GFX::Resource::IBindable>>& container;
		U64 index;

	public:
		constexpr SinkContainerBindable(std::string&& registeredName, std::vector<GfxResPtr<GFX::Resource::IBindable>>& binds, U64 index)
			: Sink(std::forward<std::string>(registeredName)), container(binds), index(index) {}
		SinkContainerBindable(SinkContainerBindable&&) = default;
		SinkContainerBindable(const SinkContainerBindable&) = default;
		SinkContainerBindable& operator=(SinkContainerBindable&&) = default;
		SinkContainerBindable& operator=(const SinkContainerBindable&) = default;
		virtual ~SinkContainerBindable() = default;

		constexpr void Bind(Source& source) override;
	};

#pragma region Functions
	template<typename T>
	constexpr void SinkContainerBindable<T>::Bind(Source& source)
	{
		auto ptr = source.LinkBindable().CastStatic<T>();
		if (ptr == nullptr)
			throw ZE_RGC_EXCEPT("Binding Sink \"" + GetRegisteredName() + "\" of type {" + typeid(T).name() +
				"} to Source \"" + GetPassPathString() + "." + GetSourceName() + "\" of incompatible type {" + typeid(source.LinkBindable()).name());
		container.at(index) = std::move(ptr);
		linked = true;
	}
#pragma endregion
}