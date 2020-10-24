#pragma once
#include "Material.h"

namespace GFX::Visual
{
	class ShadowMap : public IVisual
	{
		std::shared_ptr<Resource::ConstBufferExPixelCache> sourcePixelBuffer = nullptr;
		std::shared_ptr<Resource::ConstBufferExPixelCache> parallaxBuffer = nullptr;
		std::shared_ptr<Resource::Texture> diffuseTexture = nullptr;
		std::shared_ptr<Resource::Texture> normalMap = nullptr;
		std::shared_ptr<Resource::Texture> parallaxMap = nullptr;

		static inline GFX::Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		ShadowMap(Graphics& gfx, std::shared_ptr<Visual::Material> material);
		virtual ~ShadowMap() = default;

		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return false; }

		void Bind(Graphics& gfx) override;
	};
}