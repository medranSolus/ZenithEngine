#pragma once
#include "GFX/Pipeline/PassDesc.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	// Type of operation to be performed while clearing buffer
	enum class ClearBufferType : U8 { RTV, DSV, FloatUAV, UIntUAV };

	// Information about clearing single buffer
	struct ClearBufferEntry
	{
		ClearBufferType BufferType;
		union
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
		} ClearValue;
	};

	// Generic component for clearing resources
	template<ResIndex N>
	struct ClearBuffer
	{
		struct Resources
		{
			RID Buffers[N];
		};

		struct ExecuteData
		{
			ClearBufferEntry Info[N];

			constexpr ExecuteData() noexcept {}
			~ExecuteData() = default;
		};

		static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void*& initData) { return initData; }
		static void Clean(Device& dev, void* data) noexcept { delete reinterpret_cast<ExecuteData*>(data); }

		static PassDesc GetDesc(PassType type, const ExecuteData& clearInfo) noexcept;
		static void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	};

#pragma region Functions
	template<ResIndex N>
	PassDesc ClearBuffer<N>::GetDesc(PassType type, const ExecuteData& clearInfo) noexcept
	{
		PassDesc desc{ type };
		desc.InitData = = new ExecuteData(clearInfo);
		desc.Initialize = Initialize;
		desc.Execute = Execute;
		desc.Clean = Clean;
		return desc;
	}

	template<ResIndex N>
	void ClearBuffer<N>::Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "Batched Clear [" + std::to_string(N) + "]", PixelVal::White);
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
					//renderData.Buffers.ClearRTV(cl, rid, info.Color);
					break;
				case ClearBufferType::DSV:
					//renderData.Buffers.ClearDSV(cl, rid, info.DSV.Depth, info.DSV.Stencil);
					break;
				case ClearBufferType::FloatUAV:
					//renderData.Buffers.ClearUAV(cl, rid, info.Color);
					break;
				case ClearBufferType::UIntUAV:
					//renderData.Buffers.ClearUAV(cl, rid, info.Colors);
					break;
				}
			}
		}
		ZE_DRAW_TAG_END(dev, cl);
	}
#pragma endregion
}