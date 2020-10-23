#pragma once
#include "IVisual.h"
#include "Texture.h"
#include "ConstBufferExCache.h"
#include "assimp\scene.h"

namespace GFX::Visual
{
	class Material : public IVisual
	{
		bool translucent = false;
		std::shared_ptr<Resource::Texture> diffuseTexture = nullptr;
		std::shared_ptr<Resource::Texture> normalMap = nullptr;
		std::shared_ptr<Resource::Texture> parallaxMap = nullptr;
		std::shared_ptr<Resource::Texture> specularMap = nullptr;
		std::shared_ptr<Resource::ConstBufferExPixelCache> pixelBuffer = nullptr;
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

		inline std::shared_ptr<Resource::Texture> GetTexture() noexcept { return diffuseTexture; }
		inline std::shared_ptr<Resource::Texture> GetNormalMap() noexcept { return normalMap; }
		inline std::shared_ptr<Resource::Texture> GetParallaxMap() noexcept { return parallaxMap; }
		inline Resource::ConstBufferExPixelCache& GetPixelBuffer() noexcept { return *pixelBuffer; }
		inline std::shared_ptr<Data::VertexLayout> GerVertexLayout() noexcept { return vertexLayout; }
		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return pixelBuffer->Accept(gfx, probe); }

		void Bind(Graphics& gfx) override;
	};
}