#pragma once
#include "IBindable.h"

namespace GFX::Resource
{
	// How to perform lookups into texture
	class Sampler : public IBindable
	{
		Microsoft::WRL::ComPtr<ID3D11SamplerState> state;

	public:
		Sampler(Graphics & gfx);
		Sampler(const Sampler&) = delete;
		Sampler & operator=(const Sampler&) = delete;
		~Sampler() = default;

		inline void Bind(Graphics & gfx) noexcept override { GetContext(gfx)->PSSetSamplers(0U, 1U, state.GetAddressOf()); }
	};
}
