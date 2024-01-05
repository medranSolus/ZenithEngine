#pragma once
#include "Resource/Texture/Pack.h"
#include "Resource/CBuffer.h"

namespace ZE::GFX
{
	// Base class for creation of material types, EID of material is the same as the one from Texture::PackDesc
	//
	// NOTE: When creating new material with initial data or updating it,
	// source data must be static - it's content must be preserved for whole upload process
	template<typename T, const char* TEXTURE_SCHEMA_NAME>
	class Material final
	{
		Resource::CBuffer buffer;
		Resource::Texture::Pack textures;

	public:
		Material() = default;
		Material(Device& dev, IO::DiskManager& disk, const T& data, const Resource::Texture::PackDesc& desc) { Init(dev, disk, data, desc); }
		ZE_CLASS_MOVE(Material);
		~Material() = default;

		static constexpr const char* GetTextureSchemaName() noexcept { return TEXTURE_SCHEMA_NAME; }

		constexpr void UpdateData(Device& dev, IO::DiskManager& disk, EID materialId, const T& data) const { ZE_VALID_EID(materialId); buffer.Update(dev, disk, { materialId, &data, nullptr, sizeof(T) }); }
		constexpr void BindBuffer(CommandList& cl, Binding::Context& bindCtx) const noexcept { buffer.Bind(cl, bindCtx); }
		constexpr void BindTextures(CommandList& cl, Binding::Context& bindCtx) const noexcept { textures.Bind(cl, bindCtx); }
		constexpr void Free(Device& dev) noexcept { buffer.Free(dev); textures.Free(dev); }

		constexpr void Init(Device& dev, IO::DiskManager& disk, const T& initData, const Resource::Texture::PackDesc& desc);
	};

#pragma region Functions
	template<typename T, const char* TEXTURE_SCHEMA_NAME>
	constexpr void Material<T, TEXTURE_SCHEMA_NAME>::Init(Device& dev, IO::DiskManager& disk,
		const T& initData, const Resource::Texture::PackDesc& desc)
	{
		buffer.Init(dev, disk, { INVALID_EID, &initData, nullptr, sizeof(T) });
		textures.Init(dev, disk, desc);
	}
#pragma endregion
}