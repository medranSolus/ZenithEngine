#pragma once
#include "Resource/Cbuffer.h"
#include "Resource/Texture/Pack.h"

namespace ZE::GFX
{
	// Base class for creation of material types
	template<typename T, const char* TextureSchemaName>
	class Material final
	{
		T data;
		Resource::CBuffer buffer;
		Resource::Texture::Pack textures;

	public:
		Material() = default;
		Material(Device& dev, T&& data, const Resource::Texture::PackDesc& desc) { Init(dev, std::forward<T>(data), desc); }
		ZE_CLASS_MOVE(Material);
		~Material() = default;

		static constexpr const char* GetTextureSchemaName() noexcept { return TextureSchemaName; }

		constexpr T& GetData() noexcept { return data; }
		constexpr void BindBuffer(CommandList& cl, Binding::Context& bindCtx) const noexcept { buffer.Bind(cl, bindCtx); }
		constexpr void BindTextures(CommandList& cl, Binding::Context& bindCtx) const noexcept { textures.Bind(cl, bindCtx); }

		void UpdateData(CommandList& cl) const { buffer.Update(cl, &data, sizeof(T)); }

		void Init(Device& dev, T&& initData, const Resource::Texture::PackDesc& desc);
	};

#pragma region Functions
	template<typename T, const char* TextureSchemaName>
	void Material<T, TextureSchemaName>::Init(Device& dev, T&& initData, const Resource::Texture::PackDesc& desc)
	{
		data = std::forward<T>(initData);
		buffer.Init(dev, reinterpret_cast<U8*>(&data), sizeof(T), false);
		textures.Init(dev, desc);
	}
#pragma endregion
}