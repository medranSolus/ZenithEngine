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
		constexpr Mesh(Device& dev, const MeshData& data) { ZE_RHI_BACKEND_VAR.Init(dev, data); }
		ZE_CLASS_MOVE(Mesh);
		~Mesh() = default;

		constexpr void Init(Device& dev, const MeshData& data);
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, CommandList& cl);
		ZE_RHI_BACKEND_GET(Resource::Mesh);

		// Main Gfx API

		constexpr U32 GetVertexCount() const noexcept { U32 count = 0; ZE_RHI_BACKEND_CALL_RET(count, GetVertexCount); return count; }
		constexpr U32 GetIndexCount() const noexcept { U32 count = 0; ZE_RHI_BACKEND_CALL_RET(count, GetIndexCount); return count; }
		constexpr void Draw(Device& dev, CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(Draw, dev, cl); }
		// Before destroying buffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};

#pragma region Functions
	constexpr void Mesh::Init(Device& dev, const MeshData& data)
	{
		ZE_ASSERT(data.Vertices && data.VertexCount && data.VertexSize, "Empty vertex data!");
		ZE_ASSERT(data.Indices == nullptr || (data.Indices && data.IndexCount && data.IndexSize && data.IndexCount % 3 == 0),
			"Indices have to be multiple of 3!");
		ZE_RHI_BACKEND_VAR.Init(dev, data);
	}

	constexpr void Mesh::SwitchApi(GfxApiType nextApi, Device& dev, CommandList& cl)
	{
		MeshData data = {};
		ZE_RHI_BACKEND_CALL_RET(data, GetData, dev, cl);
		ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, data);
	}
#pragma endregion
}