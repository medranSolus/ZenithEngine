#pragma once
#include "GFX/Pipeline/PassDesc.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	// Type of operation to be performed while clearing buffer
	enum class ClearBufferType : U8 { RTV, DSV, FloatUAV, UIntUAV };

	// Information about clearing single buffer
	struct ClearBufferEntry
	{
		ClearBufferType BufferType = ClearBufferType::RTV;
		union ClearVal
		{
			// For RTV and FloatUAV
			ColorF4 Color;
			// For UIntUAV
			Pixel Colors[4];
			struct
			{
				float Depth;
				U8 Stencil;
			} DSV;

			constexpr ClearVal() noexcept {}
			~ClearVal() = default;
		} ClearValue;
	};

	// Generic component for clearing resources
	template<ResIndex N, const char* MARKER_STRING = nullptr>
	struct ClearBuffer
	{
		struct Resources
		{
			RID Buffers[N];
		};

		struct ExecuteData final : public PassExecuteData, public PassInitData
		{
			ClearBufferEntry Info[N];

			ExecuteData() = default;
			ZE_CLASS_DEFAULT(ExecuteData);
			virtual ~ExecuteData() = default;

			std::unique_ptr<PassInitData> Clone() const noexcept override { return std::make_unique<ExecuteData>(*this); }
		};

		static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, PassInitData* initData) noexcept { return std::make_shared<ExecuteData>(*static_cast<ExecuteData*>(initData)); }

		static PassDesc GetDesc(PassType type, const ExecuteData& clearInfo, PassEvaluateExecutionCallback evaluate = nullptr) noexcept;
		static Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	};

#pragma region Functions
	template<ResIndex N, const char* MARKER_STRING>
	PassDesc ClearBuffer<N, MARKER_STRING>::GetDesc(PassType type, const ExecuteData& clearInfo, PassEvaluateExecutionCallback evaluate) noexcept
	{
		PassDesc desc{ type };
		desc.InitData = std::make_unique<ExecuteData>(clearInfo);
		desc.Init = Initialize;
		desc.Evaluate = evaluate;
		desc.Execute = Execute;
		return desc;
	}

	template<ResIndex N, const char* MARKER_STRING>
	Expected<bool> ClearBuffer<N, MARKER_STRING>::Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

		if constexpr (MARKER_STRING == nullptr)
		{
			ZE_DRAW_TAG_BEGIN(dev, cl, "Batched Clear [" + std::to_string(N) + "]", PixelVal::White);
		}
		else
		{
			ZE_DRAW_TAG_BEGIN(dev, cl, MARKER_STRING, PixelVal::White);
		}
		for (ResIndex i = 0; i < N; ++i)
		{
			RID rid = ids.Buffers[i];
			if (rid != INVALID_RID)
			{
				const auto& info = data.Info[i];
				switch (data.Info[i].BufferType)
				{
				default:
					ZE_ENUM_UNHANDLED();
				case ClearBufferType::RTV:
					renderData.Buffers.ClearRTV(cl, rid, info.ClearValue.Color);
					break;
				case ClearBufferType::DSV:
					renderData.Buffers.ClearDSV(cl, rid, info.ClearValue.DSV.Depth, info.ClearValue.DSV.Stencil);
					break;
				case ClearBufferType::FloatUAV:
					renderData.Buffers.ClearUAV(cl, rid, info.ClearValue.Color);
					break;
				case ClearBufferType::UIntUAV:
					renderData.Buffers.ClearUAV(cl, rid, info.ClearValue.Colors);
					break;
				}
			}
		}
		ZE_DRAW_TAG_END(dev, cl);
		return N > 0;
	}
#pragma endregion
}

#if _ZE_GFX_MARKERS
// Used for setting the name of the debug marker for clear pass to be used later on
#	define ZE_CLEAR_BUFFER_DEBUG_MARKER(str) static const char _ZE_CLEAR_DEBUG_MARKER[] = str
// Specify this after specialization to the ClearBuffer class to allow for debug marker usage if available
#	define ZE_CLEAR_BUFFER_MARKER_SET , _ZE_CLEAR_DEBUG_MARKER
#else
// Used for setting the name of the debug marker for clear pass to be used later on
#	define ZE_CLEAR_BUFFER_DEBUG_MARKER(str)
// Specify this after specialization to the ClearBuffer class to allow for debug marker usage if available
#	define ZE_CLEAR_BUFFER_MARKER_SET
#endif