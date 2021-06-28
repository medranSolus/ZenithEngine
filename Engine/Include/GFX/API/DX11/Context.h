#pragma once
#include "GFX/Context.h"
#include "Device.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class Context : public GFX::Context
	{
		DX::ComPtr<ID3D11DeviceContext4> ctx = nullptr;

	public:
		Context(Device& dev, bool deffered);
		virtual ~Context() = default;

		ID3D11DeviceContext4* GetContext() const noexcept { return ctx.Get(); }
	};
}