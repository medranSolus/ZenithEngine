#pragma once
#include "GFX/CommandList.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class CommandList : public GFX::CommandList
	{
		DX::ComPtr<ID3D11CommandList> commands = nullptr;

	public:
		CommandList() = default;
		virtual ~CommandList() = default;

		ID3D11CommandList** GetList() noexcept { return commands.GetAddressOf(); }
	};
}