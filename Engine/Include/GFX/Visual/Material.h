#pragma once
#include "IVisual.h"
#include "GFX/Resource/Texture.h"
#include "GFX/Resource/InputLayout.h"
#include "GFX/Resource/ConstBufferExCache.h"
#include "assimp\scene.h"

namespace ZE::GFX::Visual
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
		Material(Graphics& gfx, const ColorF3& color, const std::string& name);
		Material(Graphics& gfx, const ColorF4& color, const std::string& name);
		Material(Graphics& gfx, aiMaterial& material, const std::string& path);
		Material(Material&&) = default;
		Material(const Material&) = default;
		Material& operator=(Material&&) = default;
		Material& operator=(const Material&) = default;
		virtual ~Material() = default;

		constexpr bool IsTranslucent() const noexcept { return translucent; }
		constexpr bool IsTexture() const noexcept { return diffuseTexture != nullptr; }
		constexpr bool IsParallax() const noexcept { return parallaxMap != nullptr; }

		GfxResPtr<Resource::Texture> GetTexture() noexcept { return diffuseTexture; }
		GfxResPtr<Resource::Texture> GetNormalMap() noexcept { return normalMap; }
		GfxResPtr<Resource::Texture> GetParallaxMap() noexcept { return parallaxMap; }
		constexpr Resource::ConstBufferExPixelCache& GetPixelBuffer() noexcept { return *pixelBuffer; }
		GfxResPtr<Resource::ConstBufferExPixelCache> GetBuffer() noexcept { return pixelBuffer; }

		std::shared_ptr<Data::VertexLayout> GerVertexLayout() noexcept { return vertexLayout; }
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return pixelBuffer->Accept(gfx, probe); }
		void Bind(Graphics& gfx) const override { Bind(gfx, RenderChannel::All); }

		void SetDepthOnly(Graphics& gfx);
		void Bind(Graphics& gfx, RenderChannel mode) const override;
	};
}