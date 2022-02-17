#include "GFX/API/DX12/DREDRecovery.h"
#include "GFX/API/DX12/Device.h"
#include <format>

namespace ZE::GFX::API::DX12
{
	constexpr const char* DREDRecovery::DecodeLastOperation(D3D12_AUTO_BREADCRUMB_OP operation) noexcept
	{
		switch (operation)
		{
		case D3D12_AUTO_BREADCRUMB_OP_SETMARKER:
			return "[ID3D12GraphicsCommandList/ID3D12VideoDecodeCommandList/ID3D12VideoEncodeCommandList/ID3D12VideoProcessCommandList]::SetMarker()";
		case D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT:
			return "[ID3D12GraphicsCommandList/ID3D12VideoDecodeCommandList/ID3D12VideoEncodeCommandList/ID3D12VideoProcessCommandList]::BeginEvent()";
		case D3D12_AUTO_BREADCRUMB_OP_ENDEVENT:
			return "[ID3D12GraphicsCommandList/ID3D12VideoDecodeCommandList/ID3D12VideoEncodeCommandList/ID3D12VideoProcessCommandList]::EndEvent()";
		case D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED:
			return "ID3D12GraphicsCommandList::DrawInstanced()";
		case D3D12_AUTO_BREADCRUMB_OP_DRAWINDEXEDINSTANCED:
			return "ID3D12GraphicsCommandList::DrawIndexedInstanced()";
		case D3D12_AUTO_BREADCRUMB_OP_EXECUTEINDIRECT:
			return "ID3D12GraphicsCommandList::ExecuteIndirect()";
		case D3D12_AUTO_BREADCRUMB_OP_DISPATCH:
			return "ID3D12GraphicsCommandList::Dispatch()";
		case D3D12_AUTO_BREADCRUMB_OP_COPYBUFFERREGION:
			return "ID3D12GraphicsCommandList::CopyBufferRegion()";
		case D3D12_AUTO_BREADCRUMB_OP_COPYTEXTUREREGION:
			return "ID3D12GraphicsCommandList::CopyTextureRegion()";
		case D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE:
			return "ID3D12GraphicsCommandList::CopyResource()";
		case D3D12_AUTO_BREADCRUMB_OP_COPYTILES:
			return "ID3D12GraphicsCommandList::CopyTiles()";
		case D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCE:
			return "ID3D12GraphicsCommandList::EndEvent()";
		case D3D12_AUTO_BREADCRUMB_OP_CLEARRENDERTARGETVIEW:
			return "ID3D12GraphicsCommandList::ResolveSubresource()";
		case D3D12_AUTO_BREADCRUMB_OP_CLEARUNORDEREDACCESSVIEW:
			return "ID3D12GraphicsCommandList::ClearUnorderedAccessView[Float/Uint]()";
		case D3D12_AUTO_BREADCRUMB_OP_CLEARDEPTHSTENCILVIEW:
			return "ID3D12GraphicsCommandList::ClearDepthStencilView()";
		case D3D12_AUTO_BREADCRUMB_OP_RESOURCEBARRIER:
			return "[ID3D12GraphicsCommandList/ID3D12VideoDecodeCommandList/ID3D12VideoEncodeCommandList/ID3D12VideoProcessCommandList]::ResourceBarrier()";
		case D3D12_AUTO_BREADCRUMB_OP_EXECUTEBUNDLE:
			return "ID3D12GraphicsCommandList::ExecuteBundle()";
		case D3D12_AUTO_BREADCRUMB_OP_PRESENT:
			return "IDXGISwapChain::Present()";
		case D3D12_AUTO_BREADCRUMB_OP_RESOLVEQUERYDATA:
			return "ID3D12GraphicsCommandList::ResolveQueryData()";
		case D3D12_AUTO_BREADCRUMB_OP_BEGINSUBMISSION:
			return "BeginSubmission";
		case D3D12_AUTO_BREADCRUMB_OP_ENDSUBMISSION:
			return "EndSubmission";
		case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME:
			return "ID3D12VideoDecodeCommandList::DecodeFrame()";
		case D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES:
			return "ID3D12VideoProcessCommandList::ProcessFrames()";
		case D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT:
			return "ID3D12GraphicsCommandList1::AtomicCopyBufferUINT()";
		case D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT64:
			return "ID3D12GraphicsCommandList1::AtomicCopyBufferUINT64()";
		case D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCEREGION:
			return "ID3D12GraphicsCommandList1::ResolveSubresourceRegion()";
		case D3D12_AUTO_BREADCRUMB_OP_WRITEBUFFERIMMEDIATE:
			return "[ID3D12GraphicsCommandList2//ID3D12VideoDecodeCommandList/ID3D12VideoEncodeCommandList/ID3D12VideoProcessCommandList]::WriteBufferImmediate()";
		case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME1:
			return "ID3D12VideoDecodeCommandList1::DecodeFrame1";
		case D3D12_AUTO_BREADCRUMB_OP_SETPROTECTEDRESOURCESESSION:
			return "[ID3D12GraphicsCommandList3/ID3D12VideoDecodeCommandList2/ID3D12VideoEncodeCommandList]::SetProtectedResourceSession()";
		case D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME2:
			return "ID3D12VideoDecodeCommandList[?]::DecodeFrame2()";
		case D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES1:
			return "ID3D12VideoProcessCommandList1::ProcessFrames1()";
		case D3D12_AUTO_BREADCRUMB_OP_BUILDRAYTRACINGACCELERATIONSTRUCTURE:
			return "ID3D12GraphicsCommandList4::BuildRaytracingAccelerationStructure()";
		case D3D12_AUTO_BREADCRUMB_OP_EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO:
			return "ID3D12GraphicsCommandList4::EmitRaytracingAccelerationStructurePostbuildInfo()";
		case D3D12_AUTO_BREADCRUMB_OP_COPYRAYTRACINGACCELERATIONSTRUCTURE:
			return "ID3D12GraphicsCommandList4::CopyRaytracingAccelerationStructure()";
		case D3D12_AUTO_BREADCRUMB_OP_DISPATCHRAYS:
			return "ID3D12GraphicsCommandList4::DispatchRays()";
		case D3D12_AUTO_BREADCRUMB_OP_INITIALIZEMETACOMMAND:
			return "ID3D12GraphicsCommandList4::InitializeMetaCommand()";
		case D3D12_AUTO_BREADCRUMB_OP_EXECUTEMETACOMMAND:
			return "ID3D12GraphicsCommandList4::ExecuteMetaCommand()";
		case D3D12_AUTO_BREADCRUMB_OP_ESTIMATEMOTION:
			return "ID3D12VideoEncodeCommandList::EstimateMotion()";
		case D3D12_AUTO_BREADCRUMB_OP_RESOLVEMOTIONVECTORHEAP:
			return "ID3D12VideoEncodeCommandList::ResolveMotionVectorHeap()";
		case D3D12_AUTO_BREADCRUMB_OP_SETPIPELINESTATE1:
			return "ID3D12GraphicsCommandList4::SetPipelineState1()";
		case D3D12_AUTO_BREADCRUMB_OP_INITIALIZEEXTENSIONCOMMAND:
			return "[ID3D12VideoEncodeCommandList1/ID3D12VideoProcessCommandList2]::InitializeExtensionCommand()";
		case D3D12_AUTO_BREADCRUMB_OP_EXECUTEEXTENSIONCOMMAND:
			return "[ID3D12VideoEncodeCommandList1/ID3D12VideoProcessCommandList2]::ExecuteExtensionCommand()";
		case D3D12_AUTO_BREADCRUMB_OP_DISPATCHMESH:
			return "ID3D12GraphicsCommandList6::DispatchMesh()";
		case D3D12_AUTO_BREADCRUMB_OP_ENCODEFRAME:
			return "ID3D12VideoEncodeCommandList2::EncodeFrame()";
		case D3D12_AUTO_BREADCRUMB_OP_RESOLVEENCODEROUTPUTMETADATA:
			return "ID3D12VideoEncodeCommandList2::ResolveEncoderOutputMetadata()";
		default:
			return "UNKNOW_OPERATION";
		}
	}

	constexpr const char* DREDRecovery::DecodeAllocation(D3D12_DRED_ALLOCATION_TYPE allocation) noexcept
	{
		switch (allocation)
		{
		case D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE:
			return "Command Queue";
		case D3D12_DRED_ALLOCATION_TYPE_COMMAND_ALLOCATOR:
			return "Command Allocator";
		case D3D12_DRED_ALLOCATION_TYPE_PIPELINE_STATE:
			return "Pipeline State";
		case D3D12_DRED_ALLOCATION_TYPE_COMMAND_LIST:
			return "Command List";
		case D3D12_DRED_ALLOCATION_TYPE_FENCE:
			return "Fence";
		case D3D12_DRED_ALLOCATION_TYPE_DESCRIPTOR_HEAP:
			return "Descriptor Heap";
		case D3D12_DRED_ALLOCATION_TYPE_HEAP:
			return "Heap";
		case D3D12_DRED_ALLOCATION_TYPE_QUERY_HEAP:
			return "Query Heap";
		case D3D12_DRED_ALLOCATION_TYPE_COMMAND_SIGNATURE:
			return "Command Signature";
		case D3D12_DRED_ALLOCATION_TYPE_PIPELINE_LIBRARY:
			return "Pipeline Library";
		case D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER:
			return "Video Decoder";
		case D3D12_DRED_ALLOCATION_TYPE_VIDEO_PROCESSOR:
			return "Video Processor";
		case D3D12_DRED_ALLOCATION_TYPE_RESOURCE:
			return "Resource";
		case D3D12_DRED_ALLOCATION_TYPE_PASS:
			return "Pass";
		case D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSION:
			return "Crypto Session";
		case D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSIONPOLICY:
			return "Crypto Session Policy";
		case D3D12_DRED_ALLOCATION_TYPE_PROTECTEDRESOURCESESSION:
			return "Protected Resource Session";
		case D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER_HEAP:
			return "Video Decoder Heap";
		case D3D12_DRED_ALLOCATION_TYPE_COMMAND_POOL:
			return "Command Pool";
		case D3D12_DRED_ALLOCATION_TYPE_COMMAND_RECORDER:
			return "Command Recorder";
		case D3D12_DRED_ALLOCATION_TYPE_STATE_OBJECT:
			return "State Object";
		case D3D12_DRED_ALLOCATION_TYPE_METACOMMAND:
			return "Meta Command";
		case D3D12_DRED_ALLOCATION_TYPE_SCHEDULINGGROUP:
			return "Scheduling Group";
		case D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_ESTIMATOR:
			return "Video Motion Estimator";
		case D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_VECTOR_HEAP:
			return "Video Motion Estimator Heap";
		case D3D12_DRED_ALLOCATION_TYPE_VIDEO_EXTENSION_COMMAND:
			return "Video Extension Command";
		case D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER:
			return "Video Encoder";
		case D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER_HEAP:
			return "Video Encoder Heap";
		case D3D12_DRED_ALLOCATION_TYPE_INVALID:
			return "Invalid";
		default:
			return "UNKNOWN_ALLOCATION";
		}
	}

	void DREDRecovery::Enable(DX::DebugInfoManager& debugManager)
	{
		ZE_WIN_ENABLE_EXCEPT();

		DX::ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> dred;
		ZE_GFX_THROW_FAILED_NOINFO(D3D12GetDebugInterface(IID_PPV_ARGS(&dred)));

		dred->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		dred->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		dred->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
	}

	void DREDRecovery::GetDeviceRemovedData(Device& dev, Data& data)
	{
		ZE_GFX_ENABLE(dev);

		DX::ComPtr<ID3D12DeviceRemovedExtendedData1> dred;
		ZE_GFX_THROW_FAILED(dev.GetDevice()->QueryInterface(IID_PPV_ARGS(&dred)));

		D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbs;
		D3D12_DRED_PAGE_FAULT_OUTPUT1 pageFault;
		ZE_GFX_THROW_FAILED(dred->GetAutoBreadcrumbsOutput1(&breadcrumbs));
		ZE_GFX_THROW_FAILED(dred->GetPageFaultAllocationOutput1(&pageFault));

		data.AutoBreadcrumbs = "";
		if (breadcrumbs.pHeadAutoBreadcrumbNode)
		{
			U32 ID = 0;
			for (const D3D12_AUTO_BREADCRUMB_NODE1* node = breadcrumbs.pHeadAutoBreadcrumbNode; node != nullptr && ID != *node->pLastBreadcrumbValue; node = node->pNext)
			{
				data.AutoBreadcrumbs += "\tNode: " + std::to_string(ID++);

				data.AutoBreadcrumbs += "\n\tCommand Queue: ";
				if (node->pCommandQueueDebugNameA)
					data.AutoBreadcrumbs += node->pCommandQueueDebugNameA;
				else
					data.AutoBreadcrumbs += "UNKNOWN";

				data.AutoBreadcrumbs += "\n\tCommand List: ";
				if (node->pCommandListDebugNameA)
					data.AutoBreadcrumbs += node->pCommandListDebugNameA;
				else
					data.AutoBreadcrumbs += "UNKNOWN";

				data.AutoBreadcrumbs += "\n\tLast commands: ";
				if (node->pCommandHistory)
				{
					for (U32 i = 0; i < node->BreadcrumbCount; ++i)
					{
						data.AutoBreadcrumbs += "\n\t\t";
						data.AutoBreadcrumbs += DecodeLastOperation(node->pCommandHistory[i]);

						if (node->pBreadcrumbContexts)
						{
							struct Comparator
							{
								constexpr bool operator() (const D3D12_DRED_BREADCRUMB_CONTEXT& ctx, U32 i) const noexcept { return ctx.BreadcrumbIndex < i; }
								constexpr bool operator() (U32 i, const D3D12_DRED_BREADCRUMB_CONTEXT& ctx) const noexcept { return i < ctx.BreadcrumbIndex; }
							};

							auto range = std::equal_range(node->pBreadcrumbContexts,
								node->pBreadcrumbContexts + node->BreadcrumbContextsCount,
								i, Comparator{});
							if (range.first != range.second)
							{
								for (auto ctx = range.first; ctx != range.second; ++ctx)
								{
									data.AutoBreadcrumbs += "\n\t\t\t";
									data.AutoBreadcrumbs += Utils::ToAscii(ctx->pContextString);
								}
							}
						}
					}
				}
				else
					data.AutoBreadcrumbs += "NONE";

				data.AutoBreadcrumbs += "\n\n";
			}
			if (data.AutoBreadcrumbs.size())
				data.AutoBreadcrumbs.pop_back();
		}

		data.PageFaultAddress = pageFault.PageFaultVA;
		data.ExistingAllocations = "";
		for (const D3D12_DRED_ALLOCATION_NODE1* node = pageFault.pHeadExistingAllocationNode; node != nullptr; node = node->pNext)
		{
			data.ExistingAllocations += "\tLive Object: ";
			if (node->ObjectNameA)
				data.ExistingAllocations += node->ObjectNameA;
			else
				data.ExistingAllocations += "UNKNOWN";

			data.ExistingAllocations += " (0x";
			data.ExistingAllocations += std::format("{:x}", (U64)node->pObject);
			data.ExistingAllocations += ")\n\tAllocation type: ";
			data.ExistingAllocations += DecodeAllocation(node->AllocationType);
			data.ExistingAllocations += "\n\n";
		}
		if (data.ExistingAllocations.size())
			data.ExistingAllocations.pop_back();

		data.FreedAllocations = "";
		for (const D3D12_DRED_ALLOCATION_NODE1* node = pageFault.pHeadRecentFreedAllocationNode; node != nullptr; node = node->pNext)
		{
			data.FreedAllocations += "\tFreed Object: ";
			if (node->ObjectNameA)
				data.FreedAllocations += node->ObjectNameA;
			else
				data.FreedAllocations += "UNKNOWN";

			data.FreedAllocations += " (0x";
			data.FreedAllocations += std::format("{:x}", (U64)node->pObject);
			data.FreedAllocations += ")\n\tAllocation type: ";
			data.FreedAllocations += DecodeAllocation(node->AllocationType);
			data.FreedAllocations += "\n\n";
		}
		if (data.FreedAllocations.size())
			data.FreedAllocations.pop_back();
	}
}