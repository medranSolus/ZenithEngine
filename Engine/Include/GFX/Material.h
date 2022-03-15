#pragma once
#include "Resource/Cbuffer.h"
#include "Resource/Texture/Pack.h"

namespace ZE::GFX
{
	// Base class for creation of material types
	template<typename T, const char* TextureSchemaName>
	class Material final
	{
		Resource::CBuffer buffer;
		Resource::Texture::Pack textures;

	public:
		Material() = default;
		Material(Device& dev, const T& data, const Resource::Texture::PackDesc& desc) { Init(dev, data, desc); }
		ZE_CLASS_MOVE(Material);
		~Material() = default;

		static constexpr const char* GetTextureSchemaName() noexcept { return TextureSchemaName; }

		constexpr void BindBuffer(CommandList& cl, Binding::Context& bindCtx) const noexcept { buffer.Bind(cl, bindCtx); }
		constexpr void BindTextures(CommandList& cl, Binding::Context& bindCtx) const noexcept { textures.Bind(cl, bindCtx); }

		void UpdateData(CommandList& cl, const T& data) const { buffer.Update(cl, &data, sizeof(T)); }

		void Init(Device& dev, const T& initData, const Resource::Texture::PackDesc& desc);
	};

#pragma region Functions
	template<typename T, const char* TextureSchemaName>
	void Material<T, TextureSchemaName>::Init(Device& dev, const T& initData, const Resource::Texture::PackDesc& desc)
	{
		buffer.Init(dev, reinterpret_cast<const U8*>(&initData), sizeof(T), false);
		textures.Init(dev, desc);
	}
#pragma endregion
}