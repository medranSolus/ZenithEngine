#include "GFX/API/DX12/DREDRecovery.h"
#include "GFX/API/DX12/Device.h"
#include <format>

namespace ZE::GFX::API::DX12
{
	constexpr const char* DREDRecovery::DecodeLastOperation(D3D12_AUTO_BREADCRUMB_OP operation) noexcept
	{
#	define DECODE_OP(op, info) case op: return info
		switch (operation)
		{
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_SETMARKER, "[ID3D12GraphicsCommandList/ID3D12VideoDecodeCommandList/ID3D12VideoEncodeCommandList/ID3D12VideoProcessCommandList]::SetMarker()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT, "[ID3D12GraphicsCommandList/ID3D12VideoDecodeCommandList/ID3D12VideoEncodeCommandList/ID3D12VideoProcessCommandList]::BeginEvent()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ENDEVENT, "[ID3D12GraphicsCommandList/ID3D12VideoDecodeCommandList/ID3D12VideoEncodeCommandList/ID3D12VideoProcessCommandList]::EndEvent()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED, "ID3D12GraphicsCommandList::DrawInstanced()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DRAWINDEXEDINSTANCED, "ID3D12GraphicsCommandList::DrawIndexedInstanced()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_EXECUTEINDIRECT, "ID3D12GraphicsCommandList::ExecuteIndirect()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DISPATCH, "ID3D12GraphicsCommandList::Dispatch()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_COPYBUFFERREGION, "ID3D12GraphicsCommandList::CopyBufferRegion()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_COPYTEXTUREREGION, "ID3D12GraphicsCommandList::CopyTextureRegion()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE, "ID3D12GraphicsCommandList::CopyResource()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_COPYTILES, "ID3D12GraphicsCommandList::CopyTiles()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCE, "ID3D12GraphicsCommandList::ResolveSubresource()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_CLEARRENDERTARGETVIEW, "ID3D12GraphicsCommandList::ClearRenderTargetView()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_CLEARUNORDEREDACCESSVIEW, "ID3D12GraphicsCommandList::ClearUnorderedAccessView[Float/Uint]()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_CLEARDEPTHSTENCILVIEW, "ID3D12GraphicsCommandList::ClearDepthStencilView()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOURCEBARRIER, "[ID3D12GraphicsCommandList/ID3D12VideoDecodeCommandList/ID3D12VideoEncodeCommandList/ID3D12VideoProcessCommandList]::ResourceBarrier()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_EXECUTEBUNDLE, "ID3D12GraphicsCommandList::ExecuteBundle()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_PRESENT, "IDXGISwapChain::Present()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVEQUERYDATA, "ID3D12GraphicsCommandList::ResolveQueryData()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_BEGINSUBMISSION, "BeginSubmission");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ENDSUBMISSION, "EndSubmission");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME, "ID3D12VideoDecodeCommandList::DecodeFrame()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES, "ID3D12VideoProcessCommandList::ProcessFrames()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT, "ID3D12GraphicsCommandList1::AtomicCopyBufferUINT()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT64, "ID3D12GraphicsCommandList1::AtomicCopyBufferUINT64()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCEREGION, "ID3D12GraphicsCommandList1::ResolveSubresourceRegion()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_WRITEBUFFERIMMEDIATE, "[ID3D12GraphicsCommandList2/ID3D12VideoDecodeCommandList/ID3D12VideoEncodeCommandList/ID3D12VideoProcessCommandList]::WriteBufferImmediate()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME1, "ID3D12VideoDecodeCommandList1::DecodeFrame1()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_SETPROTECTEDRESOURCESESSION, "[ID3D12GraphicsCommandList3/ID3D12VideoDecodeCommandList2/ID3D12VideoEncodeCommandList]::SetProtectedResourceSession()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME2, "ID3D12VideoDecodeCommandList[?]::DecodeFrame2()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES1, "ID3D12VideoProcessCommandList1::ProcessFrames1()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_BUILDRAYTRACINGACCELERATIONSTRUCTURE, "ID3D12GraphicsCommandList4::BuildRaytracingAccelerationStructure()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO, "ID3D12GraphicsCommandList4::EmitRaytracingAccelerationStructurePostbuildInfo()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_COPYRAYTRACINGACCELERATIONSTRUCTURE, "ID3D12GraphicsCommandList4::CopyRaytracingAccelerationStructure()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DISPATCHRAYS, "ID3D12GraphicsCommandList4::DispatchRays()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_INITIALIZEMETACOMMAND, "ID3D12GraphicsCommandList4::InitializeMetaCommand()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_EXECUTEMETACOMMAND, "ID3D12GraphicsCommandList4::ExecuteMetaCommand()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ESTIMATEMOTION, "ID3D12VideoEncodeCommandList::EstimateMotion()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVEMOTIONVECTORHEAP, "ID3D12VideoEncodeCommandList::ResolveMotionVectorHeap()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_SETPIPELINESTATE1, "ID3D12GraphicsCommandList4::SetPipelineState1()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_INITIALIZEEXTENSIONCOMMAND, "[ID3D12VideoEncodeCommandList1/ID3D12VideoProcessCommandList2]::InitializeExtensionCommand()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_EXECUTEEXTENSIONCOMMAND, "[ID3D12VideoEncodeCommandList1/ID3D12VideoProcessCommandList2]::ExecuteExtensionCommand()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DISPATCHMESH, "ID3D12GraphicsCommandList6::DispatchMesh()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ENCODEFRAME, "ID3D12VideoEncodeCommandList2::EncodeFrame()");
		DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVEENCODEROUTPUTMETADATA, "ID3D12VideoEncodeCommandList2::ResolveEncoderOutputMetadata()");
		default:
			return "UNKNOW_OPERATION";
		}
#	undef DECODE_OP
	}

	constexpr const char* DREDRecovery::DecodeAllocation(D3D12_DRED_ALLOCATION_TYPE allocation) noexcept
	{
#	define DECODE_ALLOC(alloc, info) case alloc: return info
		switch (allocation)
		{
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE, "Command Queue");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_COMMAND_ALLOCATOR, "Command Allocator");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_PIPELINE_STATE, "Pipeline State");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_COMMAND_LIST, "Command List");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_FENCE, "Fence");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_DESCRIPTOR_HEAP, "Descriptor Heap");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_HEAP, "Heap");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_QUERY_HEAP, "Query Heap");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_COMMAND_SIGNATURE, "Command Signature");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_PIPELINE_LIBRARY, "Pipeline Library");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER, "Video Decoder");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_VIDEO_PROCESSOR, "Video Processor");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_RESOURCE, "Resource");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_PASS, "Pass");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSION, "Crypto Session");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSIONPOLICY, "Crypto Session Policy");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_PROTECTEDRESOURCESESSION, "Protected Resource Session");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER_HEAP, "Video Decoder Heap");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_COMMAND_POOL, "Command Pool");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_COMMAND_RECORDER, "Command Recorder");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_STATE_OBJECT, "State Object");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_METACOMMAND, "Meta Command");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_SCHEDULINGGROUP, "Scheduling Group");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_ESTIMATOR, "Video Motion Estimator");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_VECTOR_HEAP, "Video Motion Estimator Heap");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_VIDEO_EXTENSION_COMMAND, "Video Extension Command");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER, "Video Encoder");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER_HEAP, "Video Encoder Heap");
		DECODE_ALLOC(D3D12_DRED_ALLOCATION_TYPE_INVALID, "Invalid");
		default:
			return "UNKNOWN_ALLOCATION";
		}
#	undef DECODE_ALLOC
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
		ZE_GFX_THROW_FAILED(dev.GetDev().As(&dred));

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