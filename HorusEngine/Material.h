#pragma once
#include "IVisual.h"
#include "Texture.h"
#include "InputLayout.h"
#include "ConstBufferExCache.h"
#include "assimp\scene.h"

namespace GFX::Visual
{
	class Material : public IVisual
	{
		bool translucent = false;
		GfxResPtr<Resource::InputLayout> depthOnlyInputLayout;
		GfxResPtr<Resource::Texture> diffuseTexture;
		GfxResPtr<Resource::Texture> normalMap;
		GfxResPtr<Resource::Texture> parallaxMap;
		GfxResPtr<Resource::Texture> specularMap;
		GfxResPtr<Resource::ConstBufferExPixelCache> pixelBuffer;
		std::shared_ptr<Data::VertexLayout> vertexLayout = nullptr;

	public:
		Material(Graphics& gfx, Data::ColorFloat3 color, const std::string& name);
		Material(Graphics& gfx, Data::ColorFloat4 color, const std::string& name);
		Material(Graphics& gfx, aiMaterial& material, const std::string& path);
		Material(const Material&) = default;
		Material& operator=(const Material&) = default;
		virtual ~Material() = default;

		constexpr bool IsTranslucent() const noexcept { return translucent; }
		inline bool IsTexture() const noexcept { return diffuseTexture != nullptr; }
		inline bool IsParallax() const noexcept { return parallaxMap != nullptr; }

		inline GfxResPtr<Resource::Texture> GetTexture() noexcept { return diffuseTexture; }
		inline GfxResPtr<Resource::Texture> GetNormalMap() noexcept { return normalMap; }
		inline GfxResPtr<Resource::Texture> GetParallaxMap() noexcept { return parallaxMap; }
		inline Resource::ConstBufferExPixelCache& GetPixelBuffer() noexcept { return *pixelBuffer; }
		inline GfxResPtr<Resource::ConstBufferExPixelCache> GetBuffer() noexcept { return pixelBuffer; }
		inline std::shared_ptr<Data::VertexLayout> GerVertexLayout() noexcept { return vertexLayout; }
		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return pixelBuffer->Accept(gfx, probe); }
		inline void Bind(Graphics& gfx) override { Bind(gfx, RenderChannel::All); }

		void SetDepthOnly(Graphics& gfx);
		void Bind(Graphics& gfx, RenderChannel mode) override;
	};
}