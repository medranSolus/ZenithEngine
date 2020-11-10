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

		inline void AddBind(std::shared_ptr<GFX::Resource::IBindable> bind) noexcept { binds.emplace_back(std::move(bind)); }

		template<typename T>
		void AddBindableSink(const std::string& name);

		void BindAll(Graphics& gfx);

	public:
		virtual ~BindingPass() = default;

		void Finalize() override;
	};

	template<typename T>
	void BindingPass::AddBindableSink(const std::string& name)
	{
		const size_t index = binds.size();
		binds.emplace_back();
		RegisterSink(std::make_unique<SinkContainerBindable<T>>(name, binds, index));
	}
}