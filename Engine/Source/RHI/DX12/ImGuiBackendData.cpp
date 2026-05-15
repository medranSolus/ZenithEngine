#include "RHI/DX12/ImGuiBackendData.h"
#include "RHI/DX12/GarbageCollector.h"
ZE_WARNING_PUSH
#include "backends/imgui_impl_dx12.h"
ZE_WARNING_POP

namespace ZE::RHI::DX12
{
	ImGuiBackendData::~ImGuiBackendData()
	{
		if (data)
		{
			ImGui_ImplDX12_Shutdown();
			if (data->AllocatedDescs.size())
			{
				for (auto& desc : data->AllocatedDescs)
					GarbageCollector::Get().Register(GarbageCollector::Get().MarkInactive(desc.second.Handle), std::move(desc.second));
			}
		}
	}

	Expected<ImGuiBackendData> ImGuiBackendData::Create(GFX::Device& dev, PixelFormat outputFormat) noexcept
	{
		ImGuiBackendData backend;

		backend.data = std::make_unique<AllocData>();
		backend.data->SrcDev = &dev.Get().dx12;

		ImGui_ImplDX12_InitInfo initInfo = {};
		initInfo.Device = dev.Get().dx12.GetDevice();
		initInfo.CommandQueue = dev.Get().dx12.GetQueueMain();
		initInfo.NumFramesInFlight = Utils::SafeCast<int>(Settings::GetBackbufferCount());
		initInfo.RTVFormat = RHI::DX::GetDXFormat(outputFormat);
		initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
		initInfo.UserData = backend.data.get();
		initInfo.SrvDescriptorHeap = dev.Get().dx12.GetDescHeap();
		initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
			{
				AllocData& dx12Data = *reinterpret_cast<AllocData*>(info->UserData);
				auto exp = dx12Data.SrcDev->AllocDescs(1);
				if (exp)
				{
					dx12Data.AllocatedDescs.emplace(exp->CPU.ptr, *exp);
					*out_cpu_desc_handle = exp->CPU;
					*out_gpu_desc_handle = exp->GPU;
					GarbageCollector::Get().MarkActive(*dx12Data.SrcDev, exp->Handle);
				}
				else
				{
					ZE_CODE_ERROR(exp.error(), "Failed to allocate ImGui descriptors! Crash possible!");
				}
			};
		initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle)
			{
				AllocData& dx12Data = *reinterpret_cast<AllocData*>(info->UserData);
				auto it = dx12Data.AllocatedDescs.find(cpu_desc_handle.ptr);
				if (it != dx12Data.AllocatedDescs.end())
				{
					GarbageCollector::Get().Register(GarbageCollector::Get().MarkInactive(it->second.Handle), std::move(it->second));
					dx12Data.AllocatedDescs.erase(it);
				}
				else
				{
					ZE_FAIL("Unknown descriptor to free for ImGui DX12 backend!");
				}
			};
		[[maybe_unused]] bool status = ImGui_ImplDX12_Init(&initInfo);
		ZE_ASSERT(status, "Error initializing ImGui DX12 backend!");
		return backend;
	}

	void ImGuiBackendData::NewFrame() noexcept
	{
		ImGui_ImplDX12_NewFrame();
	}

	void ImGuiBackendData::RunRender(GFX::CommandList& cl) const noexcept
	{
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cl.Get().dx12.GetList());
	}
}