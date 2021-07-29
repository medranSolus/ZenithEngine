#pragma once

namespace ZE::GFX::Pipeline
{
	class RendererPBR final
	{
	public:
		RendererPBR() {}
		RendererPBR(RendererPBR&&) = delete;
		RendererPBR(const RendererPBR&) = delete;
		RendererPBR& operator=(RendererPBR&&) = delete;
		RendererPBR& operator=(const RendererPBR&) = delete;
		~RendererPBR() {}

		void Execute() {}
	};
}