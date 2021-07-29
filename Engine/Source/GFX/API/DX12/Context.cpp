#include "GFX/Context.h"
#include "GFX/API/DX12/CommandList.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12
{
	Context::Context(GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx12);
	}

	void Context::DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx12);
		ZE_GFX_THROW_FAILED_INFO(list->DrawIndexedInstanced(count, 1, 0, 0, 0));
	}

	void Context::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx12);
		ZE_GFX_THROW_FAILED_INFO(list->Dispatch(groupX, groupY, groupZ));
	}

	void Context::Execute(GFX::Device& dev, GFX::CommandList& cl) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx12);
	}

	void Context::Execute(GFX::Device& dev, GFX::CommandList* cl, U32 count) const noexcept(ZE_NO_DEBUG)
	{
		assert(count && cl);
	}

	void Context::Execute(GFX::Device& dev, GFX::Context& ctx) const
	{
		ZE_GFX_ENABLE(dev.Get().dx12);
	}

	void Context::Execute(GFX::Device& dev, GFX::Context* ctx, U32 count) const
	{
		assert(count && ctx);
	}

	void Context::CreateList(GFX::Device& dev, GFX::CommandList& cl) const
	{
		ZE_GFX_ENABLE(dev.Get().dx12);
	}

	void Context::CreateDeffered(GFX::Device& dev, GFX::Context& ctx) const
	{
		ZE_GFX_ENABLE(dev.Get().dx12);
	}
}