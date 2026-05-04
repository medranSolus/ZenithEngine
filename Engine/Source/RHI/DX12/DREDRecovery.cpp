#include "RHI/DX12/DREDRecovery.h"
#include "RHI/DX12/Device.h"
#include <format>

namespace ZE::RHI::DX12
{
	template<typename T, U8 EXT>
	constexpr void DREDRecovery::HandleBreadcrumbNode(std::string& breadInfo, U32& id, const T* node) noexcept
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

				std::pair<D3D12_DRED_BREADCRUMB_CONTEXT*, D3D12_DRED_BREADCRUMB_CONTEXT*> range = { nullptr, nullptr };
				if constexpr (EXT > 0)
				{
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

					if constexpr (EXT > 0)
					{
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

	template<typename T, U8 EXT>
	constexpr void DREDRecovery::HandlePagefault(std::function<void(std::string_view)> writeString, const T& pageFault) noexcept
	{
		if (pageFault.PageFaultVA != 0)
			writeString(std::format("\n[PAGE FAULT ADDRESS] 0x{:x}\n", Utils::SafeCast<U64>(pageFault.PageFaultVA)));
		if constexpr (EXT > 1)
		{
			if (pageFault.PageFaultFlags != 0)
			{
				writeString("[PAGE FAULT FLAGS] ");
				writeString(std::to_string(pageFault.PageFaultFlags));
				writeString("\n");
			}
		}

		auto getAllocInfo = [&](const auto* node, std::string_view tag) noexcept -> std::string
			{
				std::string info;
				while (node != nullptr)
				{
					info += tag;
					if (node->ObjectNameA)
						info += node->ObjectNameA;
					else if (node->ObjectNameW)
						info += Utils::ToUTF8(node->ObjectNameW);
					else
						info += "UNKNOWN";

					if constexpr (EXT > 0)
						info += std::format("(0x{:x})", Utils::SafeCast<U64>(reinterpret_cast<uintptr_t>(node->pObject)));

					info += "\n\tAllocation type:";
					info += DecodeAllocation(node->AllocationType);
					info += "\n\n";

					node = node->pNext;
				}
				if (info.size())
					info.pop_back();
				return info;
			};

		std::string allocInfo = getAllocInfo(pageFault.pHeadExistingAllocationNode, "\tLive Object: ");
		if (allocInfo.size())
		{
			writeString("\n[DRED EXISTING ALLOCATIONS]\n");
			writeString(allocInfo.c_str());
		}

		allocInfo = getAllocInfo(pageFault.pHeadRecentFreedAllocationNode, "\tFreed Object: ");
		if (allocInfo.size())
		{
			writeString("\n[DRED FREED ALLOCATIONS]\n");
			writeString(allocInfo.c_str());
		}
	}

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
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_BARRIER, "Barrier()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_BEGIN_COMMAND_LIST, "Reset()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_DISPATCHGRAPH, "DispatchGraph()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_SETPROGRAM, "SetProgram()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_ENCODEFRAME1, "EncodeFrame1()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVEENCODEROUTPUTMETADATA1, "ResolveEncoderOutputMetadata1()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_RESOLVEINPUTPARAMLAYOUT, "ResolveEncoderInputLayout()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES2, "ProcessFrames2()");
			DECODE_OP(D3D12_AUTO_BREADCRUMB_OP_SET_WORK_GRAPH_MAXIMUM_GPU_INPUT_RECORDS, "SetWorkGraphMaximumInputRecords()");
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

	void DREDRecovery::Enable() noexcept
	{
		DX::ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dred = nullptr;
		DX::ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> dred1 = nullptr;
		DX::ComPtr<ID3D12DeviceRemovedExtendedDataSettings2> dred2 = nullptr;

		// Try to access highest possible DRED interface
		HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&dred2));
		if (FAILED(hr))
		{
			hr = D3D12GetDebugInterface(IID_PPV_ARGS(&dred1));
			if (FAILED(hr))
				hr = D3D12GetDebugInterface(IID_PPV_ARGS(&dred));
			else
				dred = dred1;
		}
		else
			dred = dred1 = dred2;

		if (dred)
		{
			dred->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			dred->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			if (dred1)
			{
				dred1->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
				if (dred2)
					dred2->UseMarkersOnlyAutoBreadcrumbs(false);
			}
		}
		else
		{
			ZE_CODE_WARNING(DX::Error::Make(hr), "Cannot access DRED interface - no DRED output on device removed!");
		}
	}

	void DREDRecovery::SaveDeviceRemovedData(Device& dev, const std::string& filename) noexcept
	{
		bool fileOutput = true;
		std::string loggerOutput = "";
		IO::File file;
		
		if (Status code = file.Open(filename, Base(IO::FileFlag::DefaultWrite)))
		{
			ZE_CODE_CRITICAL(code, "Device Removed! Cannot create file <" + filename + ">. Falling back to classic logger!");
			fileOutput = false;
		}

		auto writeString = [&](std::string_view s) noexcept
			{
				if (fileOutput)
				{
					if (Status code = file.Write(s.data(), Utils::SafeCast<U32>(s.size())))
					{
						ZE_CODE_ERROR(code, "Device Removed! Failed to write to file <" + filename + "> while saving DRED recovery data. Remaining data will be written via logger.");
						fileOutput = false;
						loggerOutput += s;
					}
				}
				else
					loggerOutput += s;
			};

		HRESULT hr = dev.GetDevice()->GetDeviceRemovedReason();
		{
			writeString(std::format("[HRESULT] 0x{:x}", Utils::SafeCast<U64>(hr)));
			writeString("(");
			writeString(DecodeDxgiError(hr));
			writeString(")");
		}
		{
			writeString("\n[MESSAGE]\n");

			LPSTR msgBuffer = nullptr;
			DWORD msgLen = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, Utils::SafeCast<DWORD>(hr), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&msgBuffer), 0, nullptr);
			if (msgLen)
			{
				writeString(msgBuffer);
				LocalFree(msgBuffer);
			}
			else
				writeString("Unknown error code");
		}
		{
			writeString("\n[FRAME] ");
			writeString(std::to_string(Settings::GetFrameIndex()));
		}

		DX::ComPtr<ID3D12DeviceRemovedExtendedData> dred = nullptr;
		DX::ComPtr<ID3D12DeviceRemovedExtendedData1> dred1 = nullptr;
		DX::ComPtr<ID3D12DeviceRemovedExtendedData2> dred2 = nullptr;

		// Try to access highest possible DRED interface
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&dred2));
		if (FAILED(hr))
		{
			hr = D3D12GetDebugInterface(IID_PPV_ARGS(&dred1));
			if (FAILED(hr))
				hr = D3D12GetDebugInterface(IID_PPV_ARGS(&dred));
			else
				dred = dred1;
		}
		else
			dred = dred1 = dred2;


		if (dred == nullptr)
			Logger::Critical("Cannot access DRED extended data - no breadcrumbs output!");
		else
		{
			if (dred2)
			{
				writeString("\n[DEVICE STATE] ");
				switch (dred2->GetDeviceState())
				{
				default:
					ZE_ENUM_UNHANDLED();
				case D3D12_DRED_DEVICE_STATE_UNKNOWN:
					writeString("Unknown\n");
					break;
				case D3D12_DRED_DEVICE_STATE_HUNG:
					writeString("Hung\n");
					break;
				case D3D12_DRED_DEVICE_STATE_FAULT:
					writeString("Fault\n");
					break;
				case D3D12_DRED_DEVICE_STATE_PAGEFAULT:
					writeString("Pagefault\n");
					break;
				}
			}

			D3D12_DRED_PAGE_FAULT_OUTPUT pageFault = {};
			D3D12_DRED_PAGE_FAULT_OUTPUT1 pageFault1 = {};
			D3D12_DRED_PAGE_FAULT_OUTPUT2 pageFault2 = {};
			if (dred2 && SUCCEEDED(dred2->GetPageFaultAllocationOutput2(&pageFault2)))
				HandlePagefault<D3D12_DRED_PAGE_FAULT_OUTPUT2, 2>(writeString, pageFault2);
			else if (dred1 && SUCCEEDED(dred1->GetPageFaultAllocationOutput1(&pageFault1)))
				HandlePagefault<D3D12_DRED_PAGE_FAULT_OUTPUT1, 1>(writeString, pageFault1);
			else if (SUCCEEDED(dred->GetPageFaultAllocationOutput(&pageFault)))
				HandlePagefault<D3D12_DRED_PAGE_FAULT_OUTPUT, 0>(writeString, pageFault);

			std::string breadInfo;
			U32 id = 0;
			D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbs = {};
			D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbs1 = {};
			if (dred1 && SUCCEEDED(dred1->GetAutoBreadcrumbsOutput1(&breadcrumbs1)) && breadcrumbs1.pHeadAutoBreadcrumbNode)
			{
				for (const auto* node = breadcrumbs1.pHeadAutoBreadcrumbNode; node; node = node->pNext)
				{
					HandleBreadcrumbNode<D3D12_AUTO_BREADCRUMB_NODE1, 1>(breadInfo, id, node);
				}
			}
			else if (SUCCEEDED(dred->GetAutoBreadcrumbsOutput(&breadcrumbs)) && breadcrumbs.pHeadAutoBreadcrumbNode)
			{
				for (const auto* node = breadcrumbs.pHeadAutoBreadcrumbNode; node; node = node->pNext)
				{
					HandleBreadcrumbNode<D3D12_AUTO_BREADCRUMB_NODE, 0>(breadInfo, id, node);
				}
			}
			if (breadInfo.size())
			{
				breadInfo.pop_back();
				writeString("\n[DRED AUTO BREADCRUMBS]\n");
				writeString(breadInfo);
			}
		}

		Logger::Critical(fileOutput ?
			"Device Removed! Saving crash dump to <" + filename + "> for inspection."
			: "Device Removed Recovery data:\n" + loggerOutput);
	}
}