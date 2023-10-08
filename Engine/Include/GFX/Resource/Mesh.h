#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Resource/Mesh.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Resource/Mesh.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/Mesh.h"
#endif

namespace ZE::GFX::Resource
{
	// Buffer holding indices into VertexBuffer
	class Mesh final
	{
		ZE_RHI_BACKEND(Resource::Mesh);

	public:
		Mesh() = default;
		constexpr Mesh(Device& dev, IO::DiskManager& disk, const MeshData& data) { Init(dev, disk, data); }
		Mesh(Device& dev, IO::DiskManager& disk, const MeshFileData& data, IO::File& file) { Init(dev, disk, data, file); }
		ZE_CLASS_MOVE(Mesh);
		~Mesh() = default;

		constexpr void Init(Device& dev, IO::DiskManager& disk, const MeshData& data);
		constexpr void Init(Device& dev, IO::DiskManager& disk, const MeshFileData& data, IO::File& file);
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, IO::DiskManager& disk, CommandList& cl);
		ZE_RHI_BACKEND_GET(Resource::Mesh);

		// Main Gfx API

		constexpr U32 GetVertexCount() const noexcept { U32 count = 0; ZE_RHI_BACKEND_CALL_RET(count, GetVertexCount); return count; }
		constexpr U32 GetIndexCount() const noexcept { U32 count = 0; ZE_RHI_BACKEND_CALL_RET(count, GetIndexCount); return count; }
		constexpr void Draw(Device& dev, CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(Draw, dev, cl); }
		// Before destroying buffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};

#pragma region Functions
	constexpr void Mesh::Init(Device& dev, IO::DiskManager& disk, const MeshData& data)
	{
		ZE_ASSERT(data.Vertices && data.VertexCount && data.VertexSize, "Empty vertex data!");
		ZE_ASSERT(data.Indices == nullptr || (data.Indices && data.IndexCount && data.IndexSize && data.IndexCount % 3 == 0),
			"Indices have to be multiple of 3!");
		ZE_RHI_BACKEND_VAR.Init(dev, disk, data);
	}

	constexpr void Mesh::Init(Device& dev, IO::DiskManager& disk, const MeshFileData& data, IO::File& file)
	{
		ZE_ASSERT(data.SourceBytes && data.VertexCount && data.VertexSize, "Empty vertex data!");
		ZE_ASSERT(data.IndexCount % 3 == 0 && (data.IndexFormat == PixelFormat::Unknown
			|| data.IndexFormat == PixelFormat::R8_UInt || data.IndexFormat == PixelFormat::R16_UInt || data.IndexFormat == PixelFormat::R32_UInt),
			"Indices have to be multiple of 3 and one of the following formats: R8_UInt, R16_UInt or R32_UInt!");
		ZE_RHI_BACKEND_VAR.Init(dev, disk, data, file);
	}

	constexpr void Mesh::SwitchApi(GfxApiType nextApi, Device& dev, IO::DiskManager& disk, CommandList& cl)
	{
		MeshData data = {};
		ZE_RHI_BACKEND_CALL_RET(data, GetData, dev, cl);
		ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, disk, data);
	}
#pragma endregion
}