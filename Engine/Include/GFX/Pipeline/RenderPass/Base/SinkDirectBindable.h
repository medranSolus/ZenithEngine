#pragma once
#include "Sink.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SinkDirectBindable : public Sink
	{
		static_assert(std::is_base_of_v<GFX::Resource::IBindable, T>, "SinkDirectBindable target type must be a IBindable type!");

		GfxResPtr<T>& target;

	public:
		constexpr SinkDirectBindable(std::string&& registeredName, GfxResPtr<T>& bind)
			: Sink(std::forward<std::string>(registeredName)), target(bind) {}
		SinkDirectBindable(SinkDirectBindable&&) = default;
		SinkDirectBindable(const SinkDirectBindable&) = default;
		SinkDirectBindable& operator=(SinkDirectBindable&&) = default;
		SinkDirectBindable& operator=(const SinkDirectBindable&) = default;
		virtual ~SinkDirectBindable() = default;

		static std::unique_ptr<Sink> Make(std::string&& registeredName, GfxResPtr<T>& bind);

		constexpr void Bind(Source& source) override;
	};

#pragma region Functions
	template<typename T>
	std::unique_ptr<Sink> SinkDirectBindable<T>::Make(std::string&& registeredName, GfxResPtr<T>& bind)
	{
		return std::make_unique<SinkDirectBindable>(std::forward<std::string>(registeredName), bind);
	}

	template<typename T>
	constexpr void SinkDirectBindable<T>::Bind(Source& source)
	{
		auto ptr = source.LinkBindable().CastDynamic<T>();
		if (ptr == nullptr)
			throw ZE_RGC_EXCEPT("Binding Sink \"" + GetRegisteredName() + "\" of type {" + typeid(T).name() +
				"} to Source \"" + GetPassPathString() + "." + GetSourceName() + "\" of incompatible type {" + typeid(source.LinkBindable()).name());
		target = std::move(ptr);
		linked = true;
	}
#pragma endregion
}