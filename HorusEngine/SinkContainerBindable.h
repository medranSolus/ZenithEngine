#pragma once
#include "Sink.h"

namespace GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SinkContainerBindable : public Sink
	{
		static_assert(std::is_base_of_v<GFX::Resource::IBindable, T>, "SinkContainerBindable target type must be a IBindable type!");

		std::vector<std::shared_ptr<GFX::Resource::IBindable>>& container;
		size_t index;
		bool linked = false;

	public:
		inline SinkContainerBindable(const std::string& registeredName, std::vector<std::shared_ptr<GFX::Resource::IBindable>>& binds, size_t index)
			: Sink(registeredName), container(binds), index(index) {}
		virtual ~SinkContainerBindable() = default;

		inline static std::unique_ptr<Sink> Make(const std::string& registeredName, std::shared_ptr<T>& bind) { return std::make_unique<SinkContainerBindable>(registeredName, bind); }

		inline void ValidateLink() const override { if (!linked) throw RGC_EXCEPT("Unlinked Sink \"" + GetRegisteredName() + "\"!"); }

		void Bind(Source& source) override;
	};

	template<typename T>
	void SinkContainerBindable<T>::Bind(Source& source)
	{
		auto ptr = std::dynamic_pointer_cast<T>(source.LinkBindable());
		if (ptr == nullptr)
			throw RGC_EXCEPT("Binding Sink \"" + GetRegisteredName() + "\" of type {" + typeid(T).name() +
				"} to Source \"" + GetPassPathString() + "." + GetSourceName() + "\" of incompatible type {" + typeid(source.LinkBindable()).name());
		container.at(index) = std::move(ptr);
		linked = true;
	}
}