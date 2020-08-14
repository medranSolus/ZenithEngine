#pragma once
#include "Sink.h"

namespace GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SinkDirectBindable : public Sink
	{
		static_assert(std::is_base_of_v<GFX::Resource::IBindable, T>, "SinkDirectBindable target type must be a IBindable type!");

		std::shared_ptr<T>& target;
		bool linked = false;

	public:
		inline SinkDirectBindable(const std::string& registeredName, std::shared_ptr<T>& bind) : Sink(registeredName), target(bind) {}
		virtual ~SinkDirectBindable() = default;

		inline static std::unique_ptr<Sink> Make(const std::string& registeredName, std::shared_ptr<T>& bind) { return std::make_unique<SinkDirectBindable>(registeredName, bind); }

		inline void ValidateLink() const override { if (!linked) throw RGC_EXCEPT("Unlinked Sink \"" + GetRegisteredName() + "\"!"); }

		void Bind(Source& source) override;
	};

	template<typename T>
	void SinkDirectBindable<T>::Bind(Source& source)
	{
		auto ptr = std::dynamic_pointer_cast<T>(source.LinkBindable());
		if (ptr == nullptr)
			throw RGC_EXCEPT("Binding Sink \"" + GetRegisteredName() + "\" of type {" + typeid(T).name() +
				"} to Source \"" + GetPassName() + "." + GetSourceName() + "\" of incompatible type {" + typeid(source.LinkBuffer()).name());
		target = std::move(ptr);
		linked = true;
	}
}