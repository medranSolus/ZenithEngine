#include "GUI/ImGuiManager.h"
ZE_WARNING_PUSH
#if _ZE_RHI_DX11
#	include "backends/imgui_impl_dx11.h"
#endif
#if _ZE_RHI_DX12
#	include "backends/imgui_impl_dx12.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/VulkanException.h"
#	include "backends/imgui_impl_vulkan.h"
#endif
ZE_WARNING_POP

namespace ZE::GUI
{
#if _ZE_RHI_VK
	struct CustomDataVK
	{
		VkRenderPass RenderPass;
		VkFramebuffer Framebuffer;
		VkDescriptorPool DescPool;
	};
#endif

#if _ZE_RHI_DX12
	struct CustomDataDX12
	{
		RHI::DX12::DescriptorInfo Desc;
	};
#endif

	ImGuiManager::ImGuiManager()
	{
		if (!std::filesystem::exists("imgui.ini") && std::filesystem::exists("imgui_default.ini"))
			std::filesystem::copy_file("imgui_default.ini", "imgui.ini");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		auto& style = ImGui::GetStyle();
		style.WindowRounding = 1;
		style.WindowBorderSize = 1;
		style.Colors[ImGuiCol_WindowBg].w = 0.785f;
	}

	ImGuiRenderData ImGuiManager::CreateRenderData(GFX::Device& dev, PixelFormat outputFormat)
	{
		ImGuiRenderData backendData = nullptr;
		switch (Settings::GetGfxApi())
		{
#if _ZE_RHI_DX11
		case GfxApiType::DX11:
		{
			backendData = reinterpret_cast<U8*>(1);
			ImGui_ImplDX11_Init(dev.Get().dx11.GetDevice(), dev.Get().dx11.GetMainContext());
			break;
		}
#endif
#if _ZE_RHI_DX12
		case GfxApiType::DX12:
		{
			backendData = new U8[sizeof(CustomDataDX12)];
			*backendData.Cast<CustomDataDX12>() = { dev.Get().dx12.AllocDescs(1) };
			ImGui_ImplDX12_Init(dev.Get().dx12.GetDevice(), Utils::SafeCast<int>(Settings::GetBackbufferCount()),
				RHI::DX::GetDXFormat(outputFormat), dev.Get().dx12.GetDescHeap(),
				backendData.Cast<CustomDataDX12>()->Desc.CPU, backendData.Cast<CustomDataDX12>()->Desc.GPU);
			break;
		}
#endif
#if _ZE_RHI_VK
		case GfxApiType::Vulkan:
		{
			ZE_VK_ENABLE();
			auto& device = dev.Get().vk;

			// Create render pass for ImGui rendering
			VkAttachmentDescription2 attachmentInfo = { VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2, nullptr };
			attachmentInfo.flags = 0;
			attachmentInfo.format = RHI::VK::GetVkFormat(outputFormat);
			attachmentInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentInfo.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference2 attachmentReference = { VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr };
			attachmentReference.attachment = 0;
			attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentReference.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			VkSubpassDescription2 subpassInfo = { VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2, nullptr };
			subpassInfo.flags = 0;
			subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassInfo.viewMask = 1;
			subpassInfo.inputAttachmentCount = 0;
			subpassInfo.pInputAttachments = nullptr;
			subpassInfo.colorAttachmentCount = 1;
			subpassInfo.pColorAttachments = &attachmentReference;
			subpassInfo.pResolveAttachments = nullptr;
			subpassInfo.pDepthStencilAttachment = nullptr;
			subpassInfo.preserveAttachmentCount = 0;
			subpassInfo.pPreserveAttachments = nullptr;

			VkSubpassDependency2 subpassDepInfo = { VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2, nullptr };
			subpassDepInfo.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDepInfo.dstSubpass = 0;
			subpassDepInfo.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT |
				(device.CanPresentFromCompute() ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : 0);
			subpassDepInfo.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			subpassDepInfo.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			subpassDepInfo.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			subpassDepInfo.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			subpassDepInfo.viewOffset = 0;

			VkRenderPassCreateInfo2 passInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2, nullptr };
			passInfo.flags = 0;
			passInfo.attachmentCount = 1;
			passInfo.pAttachments = &attachmentInfo;
			passInfo.subpassCount = 1;
			passInfo.pSubpasses = &subpassInfo;
			passInfo.dependencyCount = 1;
			passInfo.pDependencies = &subpassDepInfo;
			passInfo.correlatedViewMaskCount = 0;
			passInfo.pCorrelatedViewMasks = nullptr;

			VkRenderPass renderPass = VK_NULL_HANDLE;
			ZE_VK_THROW_NOSUCC(vkCreateRenderPass2(device.GetDevice(), &passInfo, nullptr, &renderPass));

			// Create descriptor pool for ImGui
			const VkDescriptorPoolSize descPoolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1 };
			VkDescriptorPoolCreateInfo descPoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr };
			descPoolInfo.flags = 0;
			descPoolInfo.maxSets = 1;
			descPoolInfo.poolSizeCount = 1;
			descPoolInfo.pPoolSizes = &descPoolSize;

			VkDescriptorPool descPool = VK_NULL_HANDLE;
			ZE_VK_THROW_NOSUCC(vkCreateDescriptorPool(device.GetDevice(), &descPoolInfo, nullptr, &descPool));

			backendData = new U8[sizeof(CustomDataVK)];
			*backendData.Cast<CustomDataVK>() = { renderPass, descPool };

			// Init ImGui
			ImGui_ImplVulkan_InitInfo initInfo;
			initInfo.Instance = device.GetInstance();
			initInfo.PhysicalDevice = device.GetPhysicalDevice();
			initInfo.Device = device.GetDevice();
			initInfo.QueueFamily = device.GetGfxQueueIndex();
			initInfo.Queue = device.GetGfxQueue();
			initInfo.PipelineCache = VK_NULL_HANDLE;
			initInfo.DescriptorPool = descPool;
			initInfo.Subpass = 0;
			initInfo.ImageCount = initInfo.MinImageCount = Settings::GetBackbufferCount();
			initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			initInfo.Allocator = nullptr;
			initInfo.CheckVkResultFn = [](VkResult res) { if (res != VK_SUCCESS) throw ZE_VK_EXCEPT(res); };
			ImGui_ImplVulkan_LoadFunctions([](const char* name, void* instance) { return vkGetInstanceProcAddr(reinterpret_cast<VkInstance>(instance), name); }, initInfo.Instance);
			ImGui_ImplVulkan_Init(&initInfo, renderPass);

			ImGui_ImplVulkan_CreateFontsTexture();
			break;
		}
#endif
		default:
		{
			ZE_FAIL("ImGui not supported under current API!");
			break;
		}
		}
		ZE_ASSERT(backendData, "Error creating ImGui data!");
		return backendData;
	}

	void ImGuiManager::DestroyRenderData(GFX::Device& dev, ImGuiRenderData& data) noexcept
	{
		switch (Settings::GetGfxApi())
		{
#if _ZE_RHI_DX11
		case GfxApiType::DX11:
		{
			if (backendData)
				ImGui_ImplDX11_Shutdown();
			break;
		}
#endif
#if _ZE_RHI_DX12
		case GfxApiType::DX12:
		{
			if (data)
			{
				ImGui_ImplDX12_Shutdown();
				dev.Get().dx12.FreeDescs(data.Cast<CustomDataDX12>()->Desc);
				data.Delete();
			}
			return;
		}
#endif
#if _ZE_RHI_VK
		case GfxApiType::Vulkan:
		{
			if (data)
			{
				ImGui_ImplVulkan_Shutdown();
				vkDestroyDescriptorPool(dev.Get().vk.GetDevice(), data.Cast<CustomDataVK>()->DescPool, nullptr);
				vkDestroyRenderPass(dev.Get().vk.GetDevice(), data.Cast<CustomDataVK>()->RenderPass, nullptr);
				data.Delete();
			}
			break;
		}
#endif
		default:
		{
			ZE_FAIL("ImGui not supported under current API!");
			break;
		}
		}
		data = nullptr;
	}

	void ImGuiManager::RunRender(GFX::CommandList& cl) noexcept
	{
		switch (Settings::GetGfxApi())
		{
#if _ZE_RHI_DX11
		case GfxApiType::DX11:
		{
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			break;
		}
#endif
#if _ZE_RHI_DX12
		case GfxApiType::DX12:
		{
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cl.Get().dx12.GetList());
			break;
		}
#endif
#if _ZE_RHI_VK
		case GfxApiType::Vulkan:
		{
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cl.Get().vk.GetBuffer());
			break;
		}
#endif
		default:
		{
			ZE_FAIL("ImGui not supported under current API!");
			break;
		}
		}
	}

	void ImGuiManager::StartFrame(const Window::MainWindow& window) const noexcept
	{
		switch (Settings::GetGfxApi())
		{
#if _ZE_RHI_DX11
		case GfxApiType::DX11:
		{
			ImGui_ImplDX11_NewFrame();
			break;
		}
#endif
#if _ZE_RHI_DX12
		case GfxApiType::DX12:
		{
			ImGui_ImplDX12_NewFrame();
			break;
		}
#endif
#if _ZE_RHI_VK
		case GfxApiType::Vulkan:
		{
			ImGui_ImplVulkan_NewFrame();
			break;
		}
#endif
		default:
		{
			ZE_FAIL("ImGui not supported under current API!");
			break;
		}
		}
		window.NewImGuiFrame();
		ImGui::NewFrame();
	}

	void ImGuiManager::EndFrame() const noexcept
	{
		ImGui::Render();
	}

	void ImGuiManager::SetFont(std::string_view font, float size) const
	{
		ImFontAtlas* atlas = ImGui::GetIO().Fonts;
		if (atlas->Fonts.size())
		{
			atlas->Clear();
#if _ZE_RHI_VK
			if (Settings::GetGfxApi() == GfxApiType::Vulkan)
				ImGui_ImplVulkan_CreateFontsTexture();
#endif
		}
		atlas->AddFontFromFileTTF(font.data(), size);
	}
}