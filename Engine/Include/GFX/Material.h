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
		ZE_CLASS_MOVE(Material);
		~Material() = default;

		static constexpr const char* GetTextureSchemaName() noexcept { return TEXTURE_SCHEMA_NAME; }

		static Expected<Material> Create(Device& dev, DiskManager& disk, const T& data, const Resource::Texture::PackDesc& desc) noexcept;
		static Expected<Material> Create(Device& dev, DiskManager& disk, const Resource::CBufferFileData& data, const Resource::Texture::PackFileDesc& pack, GFile& file) noexcept;

		constexpr void UpdateData(Device& dev, DiskManager& disk, EID materialId, const T& data) const { ZE_VALID_EID(materialId); return buffer.Update(dev, disk, { materialId, &data, nullptr, sizeof(T) }); }
		constexpr void BindBuffer(CommandList& cl, Binding::Context& bindCtx) const noexcept { buffer.Bind(cl, bindCtx); }
		constexpr void BindTextures(CommandList& cl, Binding::Context& bindCtx) const noexcept { textures.Bind(cl, bindCtx); }
	};

#pragma region Functions
	template<typename T, const char* TEXTURE_SCHEMA_NAME>
	Expected<Material<T, TEXTURE_SCHEMA_NAME>> Material<T, TEXTURE_SCHEMA_NAME>::Create(Device& dev, DiskManager& disk,
		const T& initData, const Resource::Texture::PackDesc& desc) noexcept
	{
		Material mat;
		ZE_EXPECT_RET_FAILED(mat.buffer, Resource::CBuffer::Create(dev, disk, { INVALID_EID, &initData, nullptr, sizeof(T) }));
		ZE_EXPECT_RET_FAILED(mat.textures, Resource::Texture::Pack::Create(dev, disk, desc));
		return mat;
	}

	template<typename T, const char* TEXTURE_SCHEMA_NAME>
	Expected<Material<T, TEXTURE_SCHEMA_NAME>> Material<T, TEXTURE_SCHEMA_NAME>::Create(Device& dev, DiskManager& disk,
		const Resource::CBufferFileData& data, const Resource::Texture::PackFileDesc& pack, GFile& file) noexcept
	{
		Material mat;
		ZE_EXPECT_RET_FAILED(mat.buffer, Resource::CBuffer::Create(dev, disk, data, file));
		ZE_EXPECT_RET_FAILED(mat.textures, Resource::Texture::Pack::Create(dev, disk, pack, file));
		return mat;
	}
#pragma endregion
}