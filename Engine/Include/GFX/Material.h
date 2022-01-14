#pragma once
#include "Resource/Cbuffer.h"
#include "Resource/TexturePack.h"

namespace ZE::GFX
{
	// Base class for creation of material types
	template<typename T, const char* TextureSchemaName>
	class Material final
	{
		T data;
		Resource::CBuffer buffer;
		Resource::TexturePack textures;

	public:
		Material() = default;
		Material(Device& dev, T&& data, const Resource::TexturePackDesc& desc) { Init(dev, std::forward<T>(data), desc); }
		ZE_CLASS_MOVE(Material);
		~Material() = default;

		static constexpr const char* GetTextureSchemaName() noexcept { return TextureSchemaName; }

		constexpr T& GetData() noexcept { return data; }
		void UpdateData(CommandList& cl) const { buffer.Update(cl, &data, sizeof(T)); }

		void Init(Device& dev, T&& initData, const Resource::TexturePackDesc& desc);
	};

#pragma region Functions
	template<typename T, const char* TextureSchemaName>
	void Material<T, TextureSchemaName>::Init(Device& dev, T&& initData, const Resource::TexturePackDesc& desc)
	{
		data = std::forward<T>(initData);
		buffer.Init(dev, reinterpret_cast<U8*>(&data), sizeof(T), false);
		textures.Init(dev, desc);
	}
#pragma endregion
}