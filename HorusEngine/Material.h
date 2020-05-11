#pragma once
#include "IVisual.h"
#include "GfxResources.h"
#include "assimp\scene.h"

namespace GFX::Visual
{
	class Material : public IVisual
	{
		std::shared_ptr<Resource::Texture> diffuseTexture = nullptr;
		std::shared_ptr<Resource::Texture> normalMap = nullptr;
		std::shared_ptr<Resource::Texture> specularMap = nullptr;
		std::shared_ptr<Resource::ConstBufferExPixelCache> pixelBuffer = nullptr;
		std::shared_ptr<Data::VertexLayout> vertexLayout = nullptr;

	public:
		Material(Graphics& gfx, Data::ColorFloat3 color, const std::string& name);
		Material(Graphics& gfx, aiMaterial& material, const std::string& path);
		Material(const Material&) = default;
		Material& operator=(const Material&) = default;
		virtual ~Material() = default;

		inline Resource::ConstBufferExPixelCache& GetPixelBuffer() noexcept { return *pixelBuffer; }
		inline std::shared_ptr<Data::VertexLayout> GerVertexLayout() noexcept { return vertexLayout; }

		void Bind(Graphics& gfx) noexcept override;
		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override;
	};
}