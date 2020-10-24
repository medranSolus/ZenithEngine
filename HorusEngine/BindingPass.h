#pragma once
#include "BasePass.h"
#include "IRenderTarget.h"

namespace GFX::Pipeline::RenderPass::Base
{
	class BindingPass : public BasePass
	{
		using BasePass::BasePass;

		std::vector<std::shared_ptr<GFX::Resource::IBindable>> binds;

		void BindResources(Graphics& gfx);

	protected:
		std::shared_ptr<Resource::IRenderTarget> renderTarget;
		std::shared_ptr<Resource::DepthStencil> depthStencil;

		inline BindingPass(const std::string& name, std::vector<std::shared_ptr<GFX::Resource::IBindable>>&& binds)
			: BasePass(name), binds(std::forward<std::vector<std::shared_ptr<GFX::Resource::IBindable>>>(binds)) {}

		inline void AddBind(std::shared_ptr<GFX::Resource::IBindable> bind) noexcept { binds.emplace_back(std::move(bind)); }
		inline void PopBind() noexcept { binds.pop_back(); }

		template<typename T>
		void AddBindableSink(const std::string& name);

		void BindAll(Graphics& gfx);
		void Finalize() override;

	public:
		virtual ~BindingPass() = default;
	};

	template<typename T>
	void BindingPass::AddBindableSink(const std::string& name)
	{
		const size_t index = binds.size();
		binds.emplace_back();
		RegisterSink(std::make_unique<SinkContainerBindable<T>>(name, binds, index));
	}
}