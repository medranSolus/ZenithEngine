#pragma once
#include "BasePass.h"
#include "GFX/Pipeline/Resource/IRenderTarget.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	class BindingPass : public BasePass
	{
		std::vector<GfxResPtr<GFX::Resource::IBindable>> binds;

		void BindResources(Graphics& gfx);

	protected:
		GfxResPtr<Resource::IRenderTarget> renderTarget;
		GfxResPtr<Resource::DepthStencil> depthStencil;

		template<typename T>
		void AddBindableSink(std::string&& name);

		void AddBind(GfxResPtr<GFX::Resource::IBindable>&& bind) noexcept;
		void BindAll(Graphics& gfx);

		using BasePass::BasePass;

	public:
		BindingPass(BindingPass&&) = default;
		BindingPass(const BindingPass&) = default;
		BindingPass& operator=(BindingPass&&) = default;
		BindingPass& operator=(const BindingPass&) = default;
		virtual ~BindingPass() = default;

		void Finalize() override;
	};

#pragma region Functions
	template<typename T>
	void BindingPass::AddBindableSink(std::string&& name)
	{
		const U64 index = binds.size();
		binds.emplace_back();
		RegisterSink(std::make_unique<SinkContainerBindable<T>>(std::forward<std::string>(name), binds, index));
	}
#pragma endregion
}