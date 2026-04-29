#include "RHI/VK/ImGuiBackendData.h"
ZE_WARNING_PUSH
#include "backends/imgui_impl_vulkan.h"
ZE_WARNING_POP

namespace ZE::RHI::VK
{
	ImGuiBackendData::~ImGuiBackendData()
	{
		if (data)
		{
			ImGui_ImplVulkan_Shutdown();
			if (data->DescPool)
			{
				ZE_ASSERT(data->SrcDev, "No source Device for cleanup!");
				vkDestroyDescriptorPool(data->SrcDev->GetDevice(), data->DescPool, nullptr);
			}
			if (data->RenderPass)
			{
				ZE_ASSERT(data->SrcDev, "No source Device for cleanup!");
				vkDestroyRenderPass(data->SrcDev->GetDevice(), data->RenderPass, nullptr);
			}
		}
	}

	Expected<ImGuiBackendData> ImGuiBackendData::Create(GFX::Device& dev, PixelFormat outputFormat) noexcept
	{
		ImGuiBackendData backend;

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
		const VkDescriptorPoolSize descPoolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };
		VkDescriptorPoolCreateInfo descPoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr };
		descPoolInfo.flags = 0;
		descPoolInfo.maxSets = 1;
		descPoolInfo.poolSizeCount = 1;
		descPoolInfo.pPoolSizes = &descPoolSize;

		VkDescriptorPool descPool = VK_NULL_HANDLE;
		ZE_VK_THROW_NOSUCC(vkCreateDescriptorPool(device.GetDevice(), &descPoolInfo, nullptr, &descPool));

		ImGuiRenderData backendData = new U8[sizeof(CustomDataVK)];
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
		return backend;
	}

	void RecreateFonts() noexcept
	{
		ImGui_ImplVulkan_CreateFontsTexture();
	}

	void ImGuiBackendData::NewFrame() noexcept
	{
		ImGui_ImplVulkan_NewFrame();
	}

	void ImGuiBackendData::RunRender(GFX::CommandList& cl) const noexcept
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cl.Get().vk.GetBuffer());
	}
}