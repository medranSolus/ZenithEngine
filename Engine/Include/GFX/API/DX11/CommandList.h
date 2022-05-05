#pragma once
#include "GFX/CommandType.h"
#include "D3D11.h"

namespace ZE::GFX
{
	class Device;
	namespace Resource
	{
		class PipelineStateCompute;
		class PipelineStateGfx;
	}
}
namespace ZE::GFX::API::DX11
{
	class CommandList final
	{
		bool deferred;
		DX::ComPtr<ID3D11DeviceContext4> context;
		DX::ComPtr<ID3D11CommandList> commands;
#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3DUserDefinedAnnotation> tagManager;
#endif

	public:
		CommandList() = default;
		CommandList(GFX::Device& dev);
		CommandList(GFX::Device& dev, CommandType type);
		ZE_CLASS_MOVE(CommandList);
		~CommandList() = default;

		constexpr void Open(GFX::Device& dev) {}

#ifdef _ZE_MODE_DEBUG
		void TagBegin(GFX::Device& dev, const wchar_t* tag, Pixel color) const noexcept { tagManager->BeginEvent(tag); }
		void TagEnd(GFX::Device& dev) const noexcept { tagManager->EndEvent(); }
#endif
		void Reset(GFX::Device& dev) { commands = nullptr; }

		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso);
		void Close(GFX::Device& dev);

		void Draw(GFX::Device& dev, U32 vertexCount) const noexcept(ZE_NO_DEBUG);
		void DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(ZE_NO_DEBUG);
		void DrawFullscreen(GFX::Device& dev) const noexcept(ZE_NO_DEBUG);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG);

		// Gfx API Internal

		constexpr bool IsDeferred() const noexcept { return deferred; }
		ID3D11DeviceContext4* GetContext() const noexcept { return context.Get(); }
		ID3D11CommandList* GetList() const noexcept { return commands.Get(); }
	};
}