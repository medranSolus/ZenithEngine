#include "RHI/VK/Resource/PipelineStateGfx.h"
#include "RHI/VK/VulkanException.h"

namespace ZE::RHI::VK::Resource
{
	PipelineStateGfx::PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding)
	{
		ZE_VK_ENABLE_ID();
		Device& device = dev.Get().vk;
		topology = GetPrimitiveTopology(desc.Topology, desc.Ordering);
		stencilFace = GetStencilFace(desc.Culling);

		ZE_ASSERT(desc.VS, "Vertex Shader is always required!");
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(5);
		shaderStages.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr,
			0, VK_SHADER_STAGE_VERTEX_BIT, desc.VS->Get().vk.GetModule(), "main", nullptr);

		// Optional shaders
		if (desc.DS)
		{
			shaderStages.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr,
				0, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, desc.DS->Get().vk.GetModule(), "main", nullptr);
		}
		if (desc.HS)
		{
			shaderStages.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr,
				0, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, desc.HS->Get().vk.GetModule(), "main", nullptr);
		}
		if (desc.GS)
		{
			shaderStages.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr,
				0, VK_SHADER_STAGE_GEOMETRY_BIT, desc.GS->Get().vk.GetModule(), "main", nullptr);
		}
		if (desc.PS)
		{
			shaderStages.emplace_back(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr,
				0, VK_SHADER_STAGE_FRAGMENT_BIT, desc.PS->Get().vk.GetModule(), "main", nullptr);
		}

		// Setup vertex input
		VkVertexInputBindingDescription vertexBinding = {};
		vertexBinding.binding = 0;
		vertexBinding.stride = 0;
		vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr };
		vertexInputInfo.flags = 0;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &vertexBinding;
		vertexInputInfo.vertexAttributeDescriptionCount = Utils::SafeCast<U32>(desc.InputLayout.size());

		std::vector<VkVertexInputAttributeDescription> vertexAttributes(vertexInputInfo.vertexAttributeDescriptionCount);
		if (vertexInputInfo.vertexAttributeDescriptionCount)
		{
			vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();
			for (U32 i = 0; i < vertexInputInfo.vertexAttributeDescriptionCount; ++i)
			{
				PixelFormat format = GFX::Resource::GetInputFormat(desc.InputLayout.at(i));
				auto& element = vertexAttributes.at(i);
				element.location = i;
				element.binding = 0;
				element.format = GetVkFormat(format);
				element.offset = Utils::GetFormatBitCount(format) / 8;
				vertexBinding.stride += element.offset;
			}
		}
		else
			vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		// Setup input assembly
		VkPipelineInputAssemblyStateCreateInfo inputInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr };
		inputInfo.flags = 0;
		inputInfo.topology = GetTopologyType(desc.Topology);
		inputInfo.primitiveRestartEnable = VK_TRUE;

		// Setup tesselation
		VkPipelineTessellationDomainOriginStateCreateInfo tessaltionOriginInfo = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO, nullptr };
		tessaltionOriginInfo.domainOrigin = VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT;

		VkPipelineTessellationStateCreateInfo tessalationInfo = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, &tessaltionOriginInfo };
		tessalationInfo.flags = 0;
		tessalationInfo.patchControlPoints = GetPatchCount(desc.Ordering);

		// Setup viewport information
		VkPipelineViewportStateCreateInfo viewportStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr };
		viewportStateInfo.flags = 0;
		viewportStateInfo.viewportCount = desc.RenderTargetsCount;
		viewportStateInfo.pViewports = nullptr;
		viewportStateInfo.scissorCount = desc.RenderTargetsCount;
		viewportStateInfo.pScissors = nullptr;

		// Setup rasterizer
		VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClipInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT, nullptr };
		depthClipInfo.flags = 0;
		depthClipInfo.depthClipEnable = desc.IsDepthClip();

		VkPipelineRasterizationStateCreateInfo rasterInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, &depthClipInfo };
		rasterInfo.depthClampEnable = VK_TRUE;
		rasterInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterInfo.polygonMode = desc.IsWireframe() ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rasterInfo.cullMode = GetCulling(desc.Culling);
		rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterInfo.depthBiasEnable = VK_FALSE;
		rasterInfo.depthBiasConstantFactor = 0.0f;
		rasterInfo.depthBiasClamp = 0.0f;
		rasterInfo.depthBiasSlopeFactor = 0.0f;
		rasterInfo.lineWidth = 1.0f;

		VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterInfo;
		if (device.IsExtensionSupported(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME))
		{
			conservativeRasterInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT, nullptr };
			conservativeRasterInfo.flags = 0;
			conservativeRasterInfo.conservativeRasterizationMode = desc.IsConservativeRaster() ? VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT : VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT;
			conservativeRasterInfo.extraPrimitiveOverestimationSize = device.GetConservativeRasterOverestimateSize();
			depthClipInfo.pNext = &conservativeRasterInfo;
		}

		VkPipelineRasterizationStateRasterizationOrderAMD rasterOrder;
		if (device.IsExtensionSupported(VK_AMD_RASTERIZATION_ORDER_EXTENSION_NAME))
		{
			rasterOrder = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD, depthClipInfo.pNext };
			rasterOrder.rasterizationOrder = desc.IsRelaxedRasterOrder() ? VK_RASTERIZATION_ORDER_RELAXED_AMD : VK_RASTERIZATION_ORDER_STRICT_AMD;
			depthClipInfo.pNext = &rasterOrder;
		}

		// Setup sampling
		const VkSampleMask sampleMask = UINT32_MAX;
		VkPipelineMultisampleStateCreateInfo multisampleInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr };
		multisampleInfo.flags = 0;
		multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleInfo.sampleShadingEnable = VK_FALSE;
		multisampleInfo.minSampleShading = 1.0f;
		multisampleInfo.pSampleMask = &sampleMask;
		multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineCoverageModulationStateCreateInfoNV coverageModulationInfo;
		if (device.IsExtensionSupported(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME))
		{
			coverageModulationInfo = { VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV, nullptr };
			coverageModulationInfo.flags = 0;
			coverageModulationInfo.coverageModulationMode = VK_COVERAGE_MODULATION_MODE_NONE_NV;
			coverageModulationInfo.coverageModulationTableEnable = VK_FALSE;
			coverageModulationInfo.coverageModulationTableCount = 0;
			coverageModulationInfo.pCoverageModulationTable = nullptr;
			multisampleInfo.pNext = &coverageModulationInfo;
		}

		// Setup depth stencil state
		VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, nullptr };
		depthStencilStateInfo.flags = 0;
		depthStencilStateInfo.depthTestEnable = VK_TRUE;
		depthStencilStateInfo.depthWriteEnable = VK_FALSE;
		depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_GREATER;
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateInfo.stencilTestEnable = VK_FALSE;
		depthStencilStateInfo.front.failOp = depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilStateInfo.front.passOp = depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
		depthStencilStateInfo.front.depthFailOp = depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
		depthStencilStateInfo.front.compareOp = depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilStateInfo.front.compareMask = depthStencilStateInfo.back.compareMask = UINT32_MAX;
		depthStencilStateInfo.front.writeMask = depthStencilStateInfo.back.writeMask = UINT32_MAX;
		depthStencilStateInfo.front.reference = depthStencilStateInfo.back.reference = UINT32_MAX; // Dynamic
		depthStencilStateInfo.minDepthBounds = 0.0f;
		depthStencilStateInfo.maxDepthBounds = 1.0f;

		switch (desc.DepthStencil)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::DepthStencilMode::StencilOff:
		{
			depthStencilStateInfo.depthWriteEnable = VK_TRUE;
			break;
		}
		case GFX::Resource::DepthStencilMode::StencilWrite:
		{
			depthStencilStateInfo.depthTestEnable = VK_FALSE;
			depthStencilStateInfo.stencilTestEnable = VK_TRUE;
			depthStencilStateInfo.front.writeMask = depthStencilStateInfo.back.writeMask = 0xFF;
			depthStencilStateInfo.front.passOp = depthStencilStateInfo.back.passOp = VK_STENCIL_OP_REPLACE;
			break;
		}
		case GFX::Resource::DepthStencilMode::StencilMask:
		{
			depthStencilStateInfo.depthTestEnable = VK_FALSE;
			depthStencilStateInfo.stencilTestEnable = VK_TRUE;
			depthStencilStateInfo.front.compareMask = depthStencilStateInfo.back.compareMask = 0xFF;
			depthStencilStateInfo.front.writeMask = depthStencilStateInfo.back.writeMask = 0;
			depthStencilStateInfo.front.compareOp = depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_NOT_EQUAL;
			break;
		}
		case GFX::Resource::DepthStencilMode::DepthOff:
		{
			depthStencilStateInfo.depthTestEnable = VK_FALSE;
			break;
		}
		case GFX::Resource::DepthStencilMode::DepthReverse:
		{
			depthStencilStateInfo.depthWriteEnable = VK_TRUE;
			depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			break;
		}
		case GFX::Resource::DepthStencilMode::DepthBefore:
		{
			depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
			break;
		}
		}

		// Setup blend state
		VkPipelineColorBlendAttachmentState attachmentBlendState = {};
		attachmentBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		attachmentBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		attachmentBlendState.colorBlendOp = VK_BLEND_OP_ADD;
		attachmentBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		attachmentBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		attachmentBlendState.alphaBlendOp = VK_BLEND_OP_ADD;
		attachmentBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		switch (desc.Blender)
		{
		case GFX::Resource::BlendType::None:
		{
			attachmentBlendState.blendEnable = VK_FALSE;
			break;
		}
		case GFX::Resource::BlendType::Light:
		{
			attachmentBlendState.blendEnable = VK_TRUE;
			attachmentBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			attachmentBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
			break;
		}
		case GFX::Resource::BlendType::Normal:
		{
			attachmentBlendState.blendEnable = VK_FALSE;
			attachmentBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Maybe ONE
			attachmentBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		}
		}
		const std::vector<VkPipelineColorBlendAttachmentState> attachmentBlendStates(desc.RenderTargetsCount, attachmentBlendState);

		VkPipelineColorBlendStateCreateInfo blendStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr };
		blendStateInfo.flags = 0;
		blendStateInfo.logicOpEnable = VK_FALSE;
		blendStateInfo.logicOp = VK_LOGIC_OP_NO_OP;
		blendStateInfo.attachmentCount = desc.RenderTargetsCount;
		blendStateInfo.pAttachments = attachmentBlendStates.data();
		blendStateInfo.blendConstants[0] = blendStateInfo.blendConstants[1] =
			blendStateInfo.blendConstants[2] = blendStateInfo.blendConstants[3] = 1.0f;

		// Setup dynamic states
		constexpr VkDynamicState DYNAMIC_STATES[]
		{
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, VK_DYNAMIC_STATE_STENCIL_REFERENCE
		};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr };
		dynamicStateInfo.flags = 0;
		dynamicStateInfo.dynamicStateCount = sizeof(DYNAMIC_STATES) / sizeof(VkDynamicState);
		dynamicStateInfo.pDynamicStates = DYNAMIC_STATES;

		// Set output info
		VkFormat rtFormats[8] = {};
		for (U8 i = 0; i < desc.RenderTargetsCount; ++i)
			rtFormats[i] = GetVkFormat(desc.FormatsRT[i]);

		VkPipelineRenderingCreateInfo renderInfo = { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO, nullptr };
		renderInfo.viewMask = (1U << desc.RenderTargetsCount) - 1;
		renderInfo.colorAttachmentCount = desc.RenderTargetsCount;
		renderInfo.pColorAttachmentFormats = rtFormats;
		renderInfo.stencilAttachmentFormat = renderInfo.depthAttachmentFormat = GetVkFormat(desc.FormatDS);

		// Specify optional extensions
		constexpr VkSampleCountFlagBits SAMPLE_COUNTS[]
		{
			VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_1_BIT,
			VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_1_BIT
		};
		VkAttachmentSampleCountInfoAMD sampleCountInfo;
		if (device.IsExtensionSupported(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME)
			|| device.IsExtensionSupported(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME))
		{
			sampleCountInfo = { VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD, nullptr };
			sampleCountInfo.colorAttachmentCount = desc.RenderTargetsCount;
			sampleCountInfo.pColorAttachmentSamples = SAMPLE_COUNTS;
			sampleCountInfo.depthStencilAttachmentSamples = VK_SAMPLE_COUNT_1_BIT;
			renderInfo.pNext = &sampleCountInfo;
		}

		const VkPipelineCompilerControlCreateInfoAMD compilerInfo = { VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD, renderInfo.pNext, 0 };
		if (device.IsExtensionSupported(VK_AMD_PIPELINE_COMPILER_CONTROL_EXTENSION_NAME))
			renderInfo.pNext = &compilerInfo;

		const VkPipelineRepresentativeFragmentTestStateCreateInfoNV fragmentTestInfo = { VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV, renderInfo.pNext, VK_TRUE };
		if (device.IsExtensionSupported(VK_NV_REPRESENTATIVE_FRAGMENT_TEST_EXTENSION_NAME))
			renderInfo.pNext = &fragmentTestInfo;

		// Create final pipeline (check if any of the states is needed for sure when creating, otherwise leave to nullptr)
		VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, &renderInfo };
		pipelineInfo.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
		pipelineInfo.stageCount = Utils::SafeCast<U32>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputInfo;
		pipelineInfo.pTessellationState = &tessalationInfo;
		pipelineInfo.pViewportState = &viewportStateInfo;
		pipelineInfo.pRasterizationState = &rasterInfo;
		pipelineInfo.pMultisampleState = &multisampleInfo;
		pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
		pipelineInfo.pColorBlendState = &blendStateInfo;
		pipelineInfo.pDynamicState = &dynamicStateInfo;
		pipelineInfo.layout = binding.Get().vk.GetLayout();
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = UINT32_MAX;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;
		ZE_VK_THROW_NOSUCC(vkCreateGraphicsPipelines(device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &state));
	}

	void PipelineStateGfx::Bind(GFX::CommandList& cl) const noexcept
	{
		vkCmdBindPipeline(cl.Get().vk.GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, state);
		vkCmdSetPrimitiveTopologyEXT(cl.Get().vk.GetBuffer(), topology);
		SetStencilRef(cl, 0);
	}

	void PipelineStateGfx::Free(GFX::Device& dev) noexcept
	{
		if (state)
		{
			vkDestroyPipeline(dev.Get().vk.GetDevice(), state, nullptr);
			state = VK_NULL_HANDLE;
		}
	}
}