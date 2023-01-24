#include "GFX/API/DX12/DREDRecovery.h"
#include "GFX/API/DX12/Device.h"
#include <format>

namespace ZE::GFX::API::DX12
{
	constexpr const char* DREDRecovery::DecodeLastOperation(D3D12_AUTO_BREADCRUMB_OP operation) noexcept
	{
#define DECODE_OP(op, info) case op: return info
		switch (operation)
		{
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_SETMARKER, "SetMarker()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT, "BeginEvent()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ENDEVENT, "EndEvent()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED, "DrawInstanced()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DRAWINDEXEDINSTANCED, "DrawIndexedInstanced()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_EXECUTEINDIRECT, "ExecuteIndirect()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DISPATCH, "Dispatch()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_COPYBUFFERREGION, "CopyBufferRegion()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_COPYTEXTUREREGION, "CopyTextureRegion()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE, "CopyResource()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_COPYTILES, "CopyTiles()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCE, "ResolveSubresource()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_CLEARRENDERTARGETVIEW, "ClearRenderTargetView()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_CLEARUNORDEREDACCESSVIEW, "ClearUnorderedAccessView[Float/Uint]()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_CLEARDEPTHSTENCILVIEW, "ClearDepthStencilView()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOURCEBARRIER, "ResourceBarrier()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_EXECUTEBUNDLE, "ExecuteBundle()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_PRESENT, "Present()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVEQUERYDATA, "ResolveQueryData()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_BEGINSUBMISSION, "BeginSubmission");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ENDSUBMISSION, "EndSubmission");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME, "DecodeFrame()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES, "ProcessFrames()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT, "AtomicCopyBufferUINT()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT64, "AtomicCopyBufferUINT64()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCEREGION, "ResolveSubresourceRegion()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_WRITEBUFFERIMMEDIATE, "WriteBufferImmediate()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME1, "DecodeFrame1()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_SETPROTECTEDRESOURCESESSION, "SetProtectedResourceSession()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME2, "DecodeFrame2()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES1, "ProcessFrames1()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_BUILDRAYTRACINGACCELERATIONSTRUCTURE, "BuildRaytracingAccelerationStructure()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO, "EmitRaytracingAccelerationStructurePostbuildInfo()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_COPYRAYTRACINGACCELERATIONSTRUCTURE, "CopyRaytracingAccelerationStructure()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DISPATCHRAYS, "DispatchRays()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_INITIALIZEMETACOMMAND, "InitializeMetaCommand()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_EXECUTEMETACOMMAND, "ExecuteMetaCommand()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ESTIMATEMOTION, "EstimateMotion()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVEMOTIONVECTORHEAP, "ResolveMotionVectorHeap()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_SETPIPELINESTATE1, "SetPipelineState1()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_INITIALIZEEXTENSIONCOMMAND, "InitializeExtensionCommand()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_EXECUTEEXTENSIONCOMMAND, "ExecuteExtensionCommand()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DISPATCHMESH, "DispatchMesh()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ENCODEFRAME, "EncodeFrame()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVEENCODEROUTPUTMETADATA, "ResolveEncoderOutputMetadata()");
		default:
			return "UNKNOW_OPERATION";
		}
#undef DECODE_OP
	}

	constexpr const char* DREDRecovery::DecodeAllocation(D3D12_DRED_ALLOCATION_TYPE allocation) noexcept
	{
#define DECODE_ALLOC(alloc, info) case alloc: return info
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
#undef DECODE_ALLOC
	}

	constexpr const char* DREDRecovery::DecodeDxgiError(HRESULT error) noexcept
	{
#define DECODE_ERROR(err) case err: return #err
		switch (error)
		{
			DECODE_ERROR(DXGI_ERROR_ACCESS_DENIED);
			DECODE_ERROR(DXGI_ERROR_ACCESS_LOST);
			DECODE_ERROR(DXGI_ERROR_ALREADY_EXISTS);
			DECODE_ERROR(DXGI_ERROR_CANNOT_PROTECT_CONTENT);
			DECODE_ERROR(DXGI_ERROR_DEVICE_HUNG);
			DECODE_ERROR(DXGI_ERROR_DEVICE_REMOVED);
			DECODE_ERROR(DXGI_ERROR_DEVICE_RESET);
			DECODE_ERROR(DXGI_ERROR_DRIVER_INTERNAL_ERROR);
			DECODE_ERROR(DXGI_ERROR_FRAME_STATISTICS_DISJOINT);
			DECODE_ERROR(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE);
			DECODE_ERROR(DXGI_ERROR_INVALID_CALL);
			DECODE_ERROR(DXGI_ERROR_MORE_DATA);
			DECODE_ERROR(DXGI_ERROR_NAME_ALREADY_EXISTS);
			DECODE_ERROR(DXGI_ERROR_NONEXCLUSIVE);
			DECODE_ERROR(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE);
			DECODE_ERROR(DXGI_ERROR_NOT_FOUND);
			DECODE_ERROR(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED);
			DECODE_ERROR(DXGI_ERROR_REMOTE_OUTOFMEMORY);
			DECODE_ERROR(DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE);
			DECODE_ERROR(DXGI_ERROR_SDK_COMPONENT_MISSING);
			DECODE_ERROR(DXGI_ERROR_SESSION_DISCONNECTED);
			DECODE_ERROR(DXGI_ERROR_UNSUPPORTED);
			DECODE_ERROR(DXGI_ERROR_WAIT_TIMEOUT);
			DECODE_ERROR(DXGI_ERROR_WAS_STILL_DRAWING);
		default:
			return "UNKNOWN_ERROR";
		}
#undef DECODE_ERROR
	}

	void DREDRecovery::Enable(DX::DebugInfoManager& debugManager)
	{
		ZE_WIN_ENABLE_EXCEPT();

		DX::ComPtr<IDeviceRemovedExtendedDataSettings> dred;
		ZE_DX_THROW_FAILED_NOINFO(D3D12GetDebugInterface(IID_PPV_ARGS(&dred)));

		dred->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		dred->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		dred->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
	}

	void DREDRecovery::SaveDeviceRemovedData(Device& dev, const std::string& filename)
	{
		ZE_DX_ENABLE(dev);

		bool fileOutput = true;
		std::string loggerOutput = "";
		std::ofstream fin(filename.c_str());
		if (!fin.good())
		{
			Logger::Error("Device Removed! Cannot create file <" + filename + ">. Falling back to classic logger!");
			fileOutput = false;
		}

		auto writeString = [&](const char* s)
		{
			if (fileOutput)
				fin << s;
			else
				loggerOutput += s;
		};

		ZE_WIN_EXCEPT_RESULT = dev.GetDevice()->GetDeviceRemovedReason();
		{
			writeString(std::format("[HRESULT]  0x{:x}", static_cast<U64>(ZE_WIN_EXCEPT_RESULT)).c_str());
			const char* errorName = DecodeDxgiError(ZE_WIN_EXCEPT_RESULT);
			if (errorName)
			{
				writeString("(");
				writeString(errorName);
				writeString(")");
			}
		}
		{
			writeString("\n[MESSAGE]\n");

			LPSTR msgBuffer = nullptr;
			DWORD msgLen = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, static_cast<DWORD>(ZE_WIN_EXCEPT_RESULT), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&msgBuffer), 0, NULL);
			if (msgLen)
			{
				writeString(msgBuffer);
				LocalFree(msgBuffer);
			}
			else
				writeString("Unknown error code");
		}
		{
			writeString("\n[FRAME]  ");
			writeString(std::to_string(Settings::GetFrameIndex()).c_str());
		}

		DX::ComPtr<IDeviceRemovedExtendedData> dred;
		if (FAILED(dev.GetDev().As(&dred)))
			Logger::Error("Cannot access IDeviceRemovedExtendedData - no DRED output!");
		else
		{
			D3D12_DRED_PAGE_FAULT_OUTPUT1 pageFault;
			if (SUCCEEDED(dred->GetPageFaultAllocationOutput1(&pageFault)))
			{
				if (pageFault.PageFaultVA != 0)
					writeString(std::format("\n[PAGE FAULT ADDRESS]  0x{:x}\n", static_cast<U64>(pageFault.PageFaultVA)).c_str());

				auto getAllocInfo = [&](std::string& info, const D3D12_DRED_ALLOCATION_NODE1* node, const char* tag)
				{
					while (node != nullptr)
					{
						info += tag;
						if (node->ObjectNameA)
							info += node->ObjectNameA;
						else if (node->ObjectNameW)
							info += Utils::ToUTF8(node->ObjectNameW);
						else
							info += "UNKNOWN";

						info += std::format("(0x{:x})\n\tAllocation type:", static_cast<U64>(reinterpret_cast<uintptr_t>(node->pObject)));
						info += DecodeAllocation(node->AllocationType);
						info += "\n\n";

						node = node->pNext;
					}
					if (info.size())
						info.pop_back();
				};

				std::string allocInfo;
				getAllocInfo(allocInfo, pageFault.pHeadExistingAllocationNode, "\tLive Object: ");
				if (allocInfo.size())
				{
					writeString("\n[DRED EXISTING ALLOCATIONS]\n");
					writeString(allocInfo.c_str());
				}

				allocInfo = "";
				getAllocInfo(allocInfo, pageFault.pHeadRecentFreedAllocationNode, "\tFreed Object: ");
				if (allocInfo.size())
				{
					writeString("\n[DRED FREED ALLOCATIONS]\n");
					writeString(allocInfo.c_str());
				}
			}

			D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbs;
			if (SUCCEEDED(dred->GetAutoBreadcrumbsOutput1(&breadcrumbs)) && breadcrumbs.pHeadAutoBreadcrumbNode)
			{
				std::string breadInfo;
				U32 id = 0;
				for (const auto* node = breadcrumbs.pHeadAutoBreadcrumbNode; node; node = node->pNext)
				{
					breadInfo += "\tNode: " + std::to_string(id++);

					breadInfo += "\n\tCommand Queue: ";
					if (node->pCommandQueueDebugNameA)
						breadInfo += node->pCommandQueueDebugNameA;
					else if (node->pCommandQueueDebugNameW)
						breadInfo += Utils::ToUTF8(node->pCommandQueueDebugNameW);
					else
						breadInfo += "UNKNOWN";

					breadInfo += "\n\tCommand List: ";
					if (node->pCommandListDebugNameA)
						breadInfo += node->pCommandListDebugNameA;
					else if (node->pCommandListDebugNameW)
						breadInfo += Utils::ToUTF8(node->pCommandListDebugNameW);
					else
						breadInfo += "UNKNOWN";

					breadInfo += "\n\tLast commands: ";
					if (node->pCommandHistory)
					{
						std::string indent = "\t";
						const char* prevOp = nullptr;
						U32 prevSameOpCount = 0;
						for (U32 i = 0, last = *node->pLastBreadcrumbValue; i <= last; ++i)
						{
							const char* op = DecodeLastOperation(node->pCommandHistory[i]);

							std::pair<D3D12_DRED_BREADCRUMB_CONTEXT*, D3D12_DRED_BREADCRUMB_CONTEXT*> range;
							if (node->pBreadcrumbContexts)
							{
								struct Comparator
								{
									constexpr bool operator() (const D3D12_DRED_BREADCRUMB_CONTEXT& ctx, uint32_t i) const noexcept { return ctx.BreadcrumbIndex < i; }
									constexpr bool operator() (uint32_t i, const D3D12_DRED_BREADCRUMB_CONTEXT& ctx) const noexcept { return i < ctx.BreadcrumbIndex; }
								};

								range = std::equal_range(node->pBreadcrumbContexts,
									node->pBreadcrumbContexts + node->BreadcrumbContextsCount,
									i, Comparator{});
							}
							if (op != prevOp || range.first != range.second
								|| node->pCommandHistory[i] == D3D12_AUTO_BREADCRUMB_OP_ENDEVENT
								|| node->pCommandHistory[i] == D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT)
							{
								if (prevSameOpCount > 0)
								{
									breadInfo += " x";
									breadInfo += std::to_string(prevSameOpCount + 1);
									prevSameOpCount = 0;
								}

								if (node->pCommandHistory[i] == D3D12_AUTO_BREADCRUMB_OP_ENDEVENT && indent.size() > 1)
									indent.pop_back();

								breadInfo += "\n\t";
								breadInfo += indent;
								breadInfo += op;
								prevOp = op;

								if (range.first != range.second)
								{
									breadInfo += ":";
									auto ctx = ++range.first;
									do
									{
										breadInfo += " | \"";
										breadInfo += Utils::ToUTF8(ctx->pContextString);
										breadInfo += "\"";
									} while (++ctx != range.second);
								}
								if (node->pCommandHistory[i] == D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT)
									indent += "\t";
							}
							else
								++prevSameOpCount;
						}
					}
					else
						breadInfo += "NONE";

					breadInfo += "\n\n";
				}
				if (breadInfo.size())
				{
					breadInfo.pop_back();
					writeString("\n[DRED AUTO BREADCRUMBS]\n");
					writeString(breadInfo.c_str());
				}
			}
		}

		if (fileOutput)
		{
			fin.close();
			Logger::Error("Device Removed! Saving crash dump to <" + filename + "> for inspection.");
		}
		else
			Logger::Error("Device Removed Recovery data:\n" + loggerOutput);
	}
}