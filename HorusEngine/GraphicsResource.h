#pragma once
#include "Graphics.h"

namespace GFX
{
	class GraphicsResource
	{
	protected:
		static inline ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept { return gfx.context.Get(); }
		static inline ID3D11Device* GetDevice(Graphics& gfx) noexcept { return gfx.device.Get(); }

		GraphicsResource() = default;
		GraphicsResource(const GraphicsResource&) = default;
		GraphicsResource& operator=(const GraphicsResource&) = default;

	public:
		virtual ~GraphicsResource() = default;
	};
}