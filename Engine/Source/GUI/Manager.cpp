#include "GUI/Manager.h"
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

	void Manager::RebuildFontsVK(GFX::Device& dev, GFX::CommandList& cl) const
	{
		ImGui_ImplVulkan_CreateFontsTexture();
	}
#endif

#if _ZE_RHI_DX12
	struct CustomDataDX12
	{
		RHI::DX12::DescriptorInfo Desc;
	};
#endif

	Manager::Manager()
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

	void Manager::Init(GFX::Graphics& gfx, bool backbufferSRV)
	{
		GFX::Device& dev = gfx.GetDevice();
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
				RHI::DX::GetDXFormat(Settings::BackbufferFormat), dev.Get().dx12.GetDescHeap(),
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
			VkAttachmentDescription2 attachmentInfo;
			attachmentInfo.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			attachmentInfo.pNext = nullptr;
			attachmentInfo.flags = 0;
			attachmentInfo.format = RHI::VK::GetVkFormat(Settings::BackbufferFormat);
			attachmentInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentInfo.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference2 attachmentReference;
			attachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			attachmentReference.pNext = nullptr;
			attachmentReference.attachment = 0;
			attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentReference.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			VkSubpassDescription2 subpassInfo;
			subpassInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
			subpassInfo.pNext = nullptr;
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

			VkSubpassDependency2 subpassDepInfo;
			subpassDepInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
			subpassDepInfo.pNext = nullptr;
			subpassDepInfo.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDepInfo.dstSubpass = 0;
			subpassDepInfo.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT |
				(device.CanPresentFromCompute() ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : 0);
			subpassDepInfo.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			subpassDepInfo.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			subpassDepInfo.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			subpassDepInfo.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			subpassDepInfo.viewOffset = 0;

			VkRenderPassCreateInfo2 passInfo;
			passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
			passInfo.pNext = nullptr;
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

			// Create framebuffer for swapchain attachment
			const VkFormat backbufferFormat = RHI::VK::GetVkFormat(Settings::BackbufferFormat);
			VkFramebufferAttachmentImageInfo attachmentImageInfo;
			attachmentImageInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
			attachmentImageInfo.pNext = nullptr;
			attachmentImageInfo.flags = 0;
			attachmentImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
				| (backbufferSRV ? VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT : 0);
			attachmentImageInfo.width = Settings::DisplaySize.X;
			attachmentImageInfo.height = Settings::DisplaySize.Y;
			attachmentImageInfo.layerCount = 1;
			attachmentImageInfo.viewFormatCount = 1;
			attachmentImageInfo.pViewFormats = &backbufferFormat;

			VkFramebufferAttachmentsCreateInfo framebufferAttachmentInfo;
			framebufferAttachmentInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO;
			framebufferAttachmentInfo.pNext = nullptr;
			framebufferAttachmentInfo.attachmentImageInfoCount = 1;
			framebufferAttachmentInfo.pAttachmentImageInfos = &attachmentImageInfo;

			VkFramebufferCreateInfo framebufferInfo;
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.pNext = &framebufferAttachmentInfo;
			framebufferInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = nullptr;
			framebufferInfo.width = attachmentImageInfo.width;
			framebufferInfo.height = attachmentImageInfo.height;
			framebufferInfo.layers = 1;

			VkFramebuffer framebuffer = VK_NULL_HANDLE;
			ZE_VK_THROW_NOSUCC(vkCreateFramebuffer(device.GetDevice(), &framebufferInfo, nullptr, &framebuffer));

			// Create descriptor pool for ImGui
			const VkDescriptorPoolSize descPoolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1 };
			VkDescriptorPoolCreateInfo descPoolInfo;
			descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descPoolInfo.pNext = nullptr;
			descPoolInfo.flags = 0;
			descPoolInfo.maxSets = 1;
			descPoolInfo.poolSizeCount = 1;
			descPoolInfo.pPoolSizes = &descPoolSize;

			VkDescriptorPool descPool = VK_NULL_HANDLE;
			ZE_VK_THROW_NOSUCC(vkCreateDescriptorPool(device.GetDevice(), &descPoolInfo, nullptr, &descPool));

			backendData = new U8[sizeof(CustomDataVK)];
			*backendData.Cast<CustomDataVK>() = { renderPass, framebuffer, descPool };

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

			RebuildFontsVK(dev, gfx.GetMainList());
			break;
		}
#endif
		default:
		{
			ZE_FAIL("GUI not supported under current API!");
			break;
		}
		}
	}

	void Manager::Destroy(GFX::Device& dev) noexcept
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
			if (backendData)
			{
				dev.Get().dx12.FreeDescs(backendData.Cast<CustomDataDX12>()->Desc);
				ImGui_ImplDX12_Shutdown();
				backendData.Delete();
			}
			return;
		}
#endif
#if _ZE_RHI_VK
		case GfxApiType::Vulkan:
		{
			if (backendData)
			{
				ImGui_ImplVulkan_Shutdown();
				vkDestroyDescriptorPool(dev.Get().vk.GetDevice(), backendData.Cast<CustomDataVK>()->DescPool, nullptr);
				vkDestroyFramebuffer(dev.Get().vk.GetDevice(), backendData.Cast<CustomDataVK>()->Framebuffer, nullptr);
				vkDestroyRenderPass(dev.Get().vk.GetDevice(), backendData.Cast<CustomDataVK>()->RenderPass, nullptr);
				backendData.Delete();
			}
			break;
		}
#endif
		default:
		{
			ZE_FAIL("GUI not supported under current API!");
			break;
		}
		}
		backendData = nullptr;
	}

	void Manager::StartFrame(const Window::MainWindow& window) const noexcept
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
			ZE_FAIL("GUI not supported under current API!");
			break;
		}
		}
		window.NewGuiFrame();
		ImGui::NewFrame();
	}

	void Manager::EndFrame(GFX::Graphics& gfx) const noexcept
	{
		ImGui::Render();
		auto& mainList = gfx.GetMainList();
		switch (Settings::GetGfxApi())
		{
#if _ZE_RHI_DX11
		case GfxApiType::DX11:
		{
			ZE_DRAW_TAG_BEGIN(gfx.GetDevice(), mainList.Get().dx11, "ImGui", PixelVal::Cobalt);
			mainList.Get().dx11.GetContext()->OMSetRenderTargets(1,
				reinterpret_cast<ID3D11RenderTargetView* const*>(gfx.GetSwapChain().Get().dx11.GetRTV().GetAddressOf()), nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			ZE_DRAW_TAG_END(gfx.GetDevice(), mainList.Get().dx11);
			break;
		}
#endif
#if _ZE_RHI_DX12
		case GfxApiType::DX12:
		{
			auto& dev = gfx.GetDevice().Get().dx12;
			auto& swapChain = gfx.GetSwapChain().Get().dx12;
			auto& list = mainList.Get().dx12;
			list.Open(dev);
			ZE_DRAW_TAG_BEGIN(gfx.GetDevice(), list, "ImGui", PixelVal::Cobalt);

			const D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain.GetCurrentRTV();
			list.GetList()->OMSetRenderTargets(1, &rtv, true, nullptr);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), list.GetList());
			list.GetList()->ResourceBarrier(1, &swapChain.GetPresentBarrier());

			ZE_DRAW_TAG_END(gfx.GetDevice(), list);
			list.Close(dev);
			dev.ExecuteMain(mainList);
			break;
		}
#endif
#if _ZE_RHI_VK
		case GfxApiType::Vulkan:
		{
			ZE_ASSERT(backendData != nullptr, "GUI not initialized properly!");
			auto& dev = gfx.GetDevice().Get().vk;
			auto& swapChain = gfx.GetSwapChain().Get().vk;
			auto& list = mainList.Get().vk;

			list.Open();
			ZE_DRAW_TAG_BEGIN(gfx.GetDevice(), list, "ImGui", PixelVal::Cobalt);

			const VkSubpassBeginInfo subpassBeginInfo = { VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO, nullptr, VK_SUBPASS_CONTENTS_INLINE };
			const VkSubpassEndInfo subpassEndInfo = { VK_STRUCTURE_TYPE_SUBPASS_END_INFO, nullptr };

			const VkImageView backbufferView = swapChain.GetCurrentView();
			VkRenderPassAttachmentBeginInfo attachmentInfo;
			attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
			attachmentInfo.pNext = nullptr;
			attachmentInfo.attachmentCount = 1;
			attachmentInfo.pAttachments = &backbufferView;

			VkRenderPassBeginInfo passBeginInfo;
			passBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			passBeginInfo.pNext = &attachmentInfo;
			passBeginInfo.renderPass = backendData.CastConst<CustomDataVK>()->RenderPass;
			passBeginInfo.framebuffer = backendData.CastConst<CustomDataVK>()->Framebuffer;
			passBeginInfo.renderArea = { { 0, 0 }, { Settings::DisplaySize.X, Settings::DisplaySize.Y } };
			passBeginInfo.clearValueCount = 0;
			passBeginInfo.pClearValues = nullptr;

			vkCmdBeginRenderPass2(list.GetBuffer(), &passBeginInfo, &subpassBeginInfo);
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), list.GetBuffer());
			vkCmdEndRenderPass2(list.GetBuffer(), &subpassEndInfo);

			ZE_DRAW_TAG_END(gfx.GetDevice(), list);
			list.Close();
			swapChain.ExecutePresentTransition(dev, list);
			break;
		}
#endif
		default:
		{
			ZE_FAIL("GUI not supported under current API!");
			break;
		}
		}
	}

	void Manager::SetFont(GFX::Graphics& gfx, const std::string& font, float size) const
	{
		ImFontAtlas* atlas = ImGui::GetIO().Fonts;
		if (atlas->Fonts.size())
		{
			atlas->Clear();
#if _ZE_RHI_VK
			if (Settings::GetGfxApi() == GfxApiType::Vulkan)
				RebuildFontsVK(gfx.GetDevice(), gfx.GetMainList());
#endif
		}
		atlas->AddFontFromFileTTF(font.c_str(), size);
	}
}