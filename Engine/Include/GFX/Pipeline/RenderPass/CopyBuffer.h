#pragma once
#include "GFX/Pipeline/PassDesc.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	// Type of operation to be performed while copying buffer
	enum class CopyMethod : U8 { Generic, FullResource, BufferRegion };

	// Information about copying single buffer
	struct CopyBufferEntry
	{
		CopyMethod Method = CopyMethod::Generic;
		union CopyDesc
		{
			struct
			{
				U64 SrcOffset = UINT64_MAX;
				U64 DestOffset = UINT64_MAX;
				U64 Bytes = UINT64_MAX;
			} BufferRegion;

			constexpr CopyDesc() noexcept {}
			~CopyDesc() = default;
		} CopyDesc;
	};

	// Generic component for copying resources
	template<ResIndex N, const char* MARKER_STRING = nullptr>
	struct CopyBuffer
	{
		struct Resources
		{
			RID Sources[N];
			RID Destinations[N];
		};

		struct ExecuteData
		{
			CopyBufferEntry Info[N];
		};

		static Expected<std::unique_ptr<PassExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept { return std::make_unique<ExecuteData>(*reinterpret_cast<ExecuteData*>(initData)); }
		static void* CopyInitData(void* data) noexcept { return new ExecuteData(*reinterpret_cast<ExecuteData*>(data)); }
		static void FreeInitData(void* data) noexcept { delete reinterpret_cast<ExecuteData*>(data); }

		static PassDesc GetDesc(PassType type, const ExecuteData& clearInfo, PassEvaluateExecutionCallback evaluate = nullptr) noexcept;
		static Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	};

#pragma region Functions
	template<ResIndex N, const char* MARKER_STRING>
	PassDesc CopyBuffer<N, MARKER_STRING>::GetDesc(PassType type, const ExecuteData& clearInfo, PassEvaluateExecutionCallback evaluate) noexcept
	{
		PassDesc desc{ type };
		desc.InitData = new ExecuteData(clearInfo);
		desc.Init = Initialize;
		desc.Evaluate = evaluate;
		desc.Execute = Execute;
		desc.CopyInitData = CopyInitData;
		desc.FreeInitData = FreeInitData;
		return desc;
	}

	template<ResIndex N, const char* MARKER_STRING>
	Status CopyBuffer<N, MARKER_STRING>::Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

		if constexpr (MARKER_STRING == nullptr)
		{
			ZE_DRAW_TAG_BEGIN(dev, cl, "Batched Copy [" + std::to_string(N) + "]", PixelVal::White);
		}
		else
		{
			ZE_DRAW_TAG_BEGIN(dev, cl, MARKER_STRING, PixelVal::White);
		}
		for (ResIndex i = 0; i < N; ++i)
		{
			RID src = ids.Sources[i];
			RID dest = ids.Destinations[i];
			if (src != INVALID_RID && dest != INVALID_RID)
			{
				const auto& info = data.Info[i];
				switch (data.Info[i].Method)
				{
				default:
					ZE_ENUM_UNHANDLED();
				case CopyMethod::Generic:
					renderData.Buffers.Copy(dev, cl, src, dest);
					break;
				case CopyMethod::FullResource:
					renderData.Buffers.CopyFullResource(cl, src, dest);
					break;
				case CopyMethod::BufferRegion:
					renderData.Buffers.CopyBufferRegion(cl, src, info.CopyDesc.BufferRegion.SrcOffset,
						dest, info.CopyDesc.BufferRegion.DestOffset, info.CopyDesc.BufferRegion.Bytes);
					break;
				}
			}
		}
		ZE_DRAW_TAG_END(dev, cl);
		return {};
	}
#pragma endregion
}

#if _ZE_GFX_MARKERS
// Used for setting the name of the debug marker for copy pass to be used later on
#	define ZE_COPY_BUFFER_DEBUG_MARKER(str) static const char _ZE_COPY_DEBUG_MARKER[] = str
// Specify this after specialization to the CopyBuffer class to allow for debug marker usage if available
#	define ZE_COPY_BUFFER_MARKER_SET , _ZE_COPY_DEBUG_MARKER
#else
// Used for setting the name of the debug marker for copy pass to be used later on
#	define ZE_COPY_BUFFER_DEBUG_MARKER(str)
// Specify this after specialization to the CopyBuffer class to allow for debug marker usage if available
#	define ZE_COPY_BUFFER_MARKER_SET
#endif