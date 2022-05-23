#pragma once
#include "Resource/CBuffer.h"
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

		constexpr void UpdateData(Device& dev, const T& data) const { buffer.Update(dev, &data, sizeof(T)); }
		constexpr void BindBuffer(CommandList& cl, Binding::Context& bindCtx) const noexcept { buffer.Bind(cl, bindCtx); }
		constexpr void BindTextures(CommandList& cl, Binding::Context& bindCtx) const noexcept { textures.Bind(cl, bindCtx); }
		constexpr void Free(Device& dev) noexcept { buffer.Free(dev); textures.Free(dev); }

		constexpr void Init(Device& dev, const T& initData, const Resource::Texture::PackDesc& desc);
	};

#pragma region Functions
	template<typename T, const char* TextureSchemaName>
	constexpr void Material<T, TextureSchemaName>::Init(Device& dev, const T& initData, const Resource::Texture::PackDesc& desc)
	{
		buffer.Init(dev, reinterpret_cast<const U8*>(&initData), sizeof(T));
		textures.Init(dev, desc);
	}
#pragma endregion
}