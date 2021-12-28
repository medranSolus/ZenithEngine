#pragma once
// Headers needed for DirectX Graphics Infrastructure
#include "GFX/Resource/Topology.h"
#include "GFX/DX.h"
#include "DebugInfoManager.h"
#include <dxgi1_6.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

namespace ZE::GFX::API::DX
{
	// Creates DXGI Factory
	ComPtr<IDXGIFactory7> CreateFactory(
#ifdef _ZE_MODE_DEBUG
		DebugInfoManager& debugManager
#endif
	);

	// Creates DXGI Factory and enumerates available GPU adapters in order of highest performant
	ComPtr<IDXGIAdapter4> CreateAdapter(
#ifdef _ZE_MODE_DEBUG
		DebugInfoManager& debugManager
#endif
	);

	// Creates swap chain for window and returns present flags
	UINT CreateSwapChain(ComPtr<IDXGIFactory7> factory, IUnknown* device, HWND window, ComPtr<IDXGISwapChain4>& swapChain
#ifdef _ZE_MODE_DEBUG
		, DebugInfoManager& debugManager
#endif
	);

	// Get qualified primitive topology
	constexpr D3D_PRIMITIVE_TOPOLOGY GetTopology(GFX::Resource::TopologyType type, GFX::Resource::TopologyOrder order) noexcept;

#pragma region Functions
	constexpr D3D_PRIMITIVE_TOPOLOGY GetTopology(GFX::Resource::TopologyType type, GFX::Resource::TopologyOrder order) noexcept
	{
		switch (type)
		{
		case GFX::Resource::TopologyType::Point:
		{
			switch (order)
			{
			case GFX::Resource::TopologyOrder::List:
				return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			}
			ZE_ASSERT(false, "Wrong combination of TopologyType and TopologyOrder!");
			break;
		}
		case GFX::Resource::TopologyType::Line:
		{
			switch (order)
			{
			case GFX::Resource::TopologyOrder::List:
				return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case GFX::Resource::TopologyOrder::Strip:
				return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case GFX::Resource::TopologyOrder::ListAdjacency:
				return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
			case GFX::Resource::TopologyOrder::StripAdjacency:
				return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
			}
			ZE_ASSERT(false, "Wrong combination of TopologyType and TopologyOrder!");
			break;
		}
		case GFX::Resource::TopologyType::Triangle:
		{
			switch (order)
			{
			case GFX::Resource::TopologyOrder::List:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case GFX::Resource::TopologyOrder::Strip:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			case GFX::Resource::TopologyOrder::ListAdjacency:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
			case GFX::Resource::TopologyOrder::StripAdjacency:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
			}
			ZE_ASSERT(false, "Wrong combination of TopologyType and TopologyOrder!");
			break;
		}
		case GFX::Resource::TopologyType::ControlPoint:
		{
			switch (order)
			{
			case GFX::Resource::TopologyOrder::PatchList1:
				return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList2:
				return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList3:
				return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList4:
				return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList5:
				return D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList6:
				return D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList7:
				return D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList8:
				return D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList9:
				return D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList10:
				return D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList11:
				return D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList12:
				return D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList13:
				return D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList14:
				return D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList15:
				return D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList16:
				return D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList17:
				return D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList18:
				return D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList19:
				return D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList20:
				return D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList21:
				return D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList22:
				return D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList23:
				return D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList24:
				return D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList25:
				return D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList26:
				return D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList27:
				return D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList28:
				return D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList29:
				return D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList30:
				return D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList31:
				return D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
			case GFX::Resource::TopologyOrder::PatchList32:
				return D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
			}
			ZE_ASSERT(false, "Wrong combination of TopologyType and TopologyOrder!");
			break;
		}
		}
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
#pragma endregion
}