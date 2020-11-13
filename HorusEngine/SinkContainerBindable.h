#pragma once
#include "Sink.h"

namespace GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SinkContainerBindable : public Sink
	{
		static_assert(std::is_base_of_v<GFX::Resource::IBindable, T>, "SinkContainerBindable target type must be a IBindable type!");

		std::vector<GfxResPtr<GFX::Resource::IBindable>>& container;
		size_t index;

	public:
		inline SinkContainerBindable(const std::string& registeredName, std::vector<GfxResPtr<GFX::Resource::IBindable>>& binds, size_t index)
			: Sink(registeredName), container(binds), index(index) {}
		virtual ~SinkContainerBindable() = default;

		void Bind(Source& source) override;
	};

	template<typename T>
	void SinkContainerBindable<T>::Bind(Source& source)
	{
		auto ptr = source.LinkBindable().CastDynamic<T>();
		if (ptr == nullptr)
			throw RGC_EXCEPT("Binding Sink \"" + GetRegisteredName() + "\" of type {" + typeid(T).name() +
				"} to Source \"" + GetPassPathString() + "." + GetSourceName() + "\" of incompatible type {" + typeid(source.LinkBindable()).name());
		container.at(index) = std::move(ptr);
		linked = true;
	}
}