#include "GFX/API/VK/Binding/Schema.h"
#include "GFX/API/VK/VulkanException.h"

namespace ZE::GFX::API::VK::Binding
{
	Schema::Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc)
	{
		ZE_VK_ENABLE_ID();
		VkDevice device = dev.Get().vk.GetDevice();

		// Check input data
		count = 0;
		for (const auto& entry : desc.Ranges)
		{
			entry.Validate();

			if (entry.Flags & GFX::Binding::RangeFlag::Constant
				|| entry.Flags & GFX::Binding::RangeFlag::BufferPack)
				++count;
			else if (!(entry.Flags & GFX::Binding::RangeFlag::BufferPackAppend))
				count += entry.Count;
		}
		bindings = new Binding[count];

		VkDescriptorSetLayoutBindingFlagsCreateInfo descLayoutFlags = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO, nullptr };
		VkDescriptorSetLayoutCreateInfo descLayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, &descLayoutFlags };
		descLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

		std::vector<VkDescriptorBindingFlags> currentFlags;
		std::vector<VkDescriptorSetLayoutBinding> currentBindings;
		std::vector<VkDescriptorSetLayout> sets;
		std::vector<VkPushConstantRange> constants;

		auto createBindingTable = [&]()
		{
			if (currentBindings.size())
			{
				descLayoutFlags.bindingCount = static_cast<U32>(currentFlags.size());
				descLayoutFlags.pBindingFlags = currentFlags.data();
				descLayoutInfo.bindingCount = static_cast<U32>(currentBindings.size());
				descLayoutInfo.pBindings = currentBindings.data();

				VkDescriptorSetLayout descLayout = VK_NULL_HANDLE;
				ZE_VK_THROW_NOSUCC(vkCreateDescriptorSetLayout(device, &descLayoutInfo, nullptr, &descLayout));

				sets.emplace_back(descLayout);
				currentFlags.clear();
				currentBindings.clear();
			}
		};

		U32 constantsOffset = 0;
		ShaderPresenceMask shaderPresenceConstants;
		for (U32 i = 0; const auto & entry : desc.Ranges)
		{
			const VkShaderStageFlags shaderStage = GetShaderStage(entry.Shaders);
			if (entry.Flags & GFX::Binding::RangeFlag::Constant)
			{
				ZE_ASSERT(!shaderPresenceConstants.SetPresence(entry.Shaders), "Only single Constant per shader type is allowed!");

				const U32 size = Math::AlignUp(entry.Count, 4U);
				constants.emplace_back(shaderStage, constantsOffset, size);
				constantsOffset += size;
				bindings[i++] = { BindType::Constant, entry.RangeSlot };
			}
			else
			{
				VkDescriptorSetLayoutBinding binding = {};
				binding.binding = entry.StartSlot;

				if (entry.Flags & GFX::Binding::RangeFlag::SRV)
					binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				else if (entry.Flags & GFX::Binding::RangeFlag::UAV)
					binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				else
					binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // TODO: What about storage buffer?

				binding.descriptorCount = entry.Count;
				binding.stageFlags = shaderStage;
				binding.pImmutableSamplers = nullptr;

				// When descriptors are set in descriptor buffer they won't change until draw finishes
				// and when data is entering pipeline it is already static
				const VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

				if (entry.Flags & GFX::Binding::RangeFlag::BufferPackAppend)
				{
					ZE_ASSERT(currentBindings.size() && bindings[i - 1].Type == BindType::Table,
						"New ranges for table must be specified directly after that table! Preceding range must also have BufferPackAppend or BufferPack flag!");
					currentBindings.emplace_back(binding);
					currentFlags.emplace_back(bindingFlags);
				}
				else
				{
					createBindingTable();
					if (entry.Flags & GFX::Binding::RangeFlag::BufferPack)
					{
						currentBindings.emplace_back(binding);
						currentFlags.emplace_back(bindingFlags);
						bindings[i++] = { BindType::Table, entry.RangeSlot };
					}
					else
					{
						BindType type;
						if (entry.Flags & GFX::Binding::RangeFlag::SRV)
							type = BindType::SRV;
						else if (entry.Flags & GFX::Binding::RangeFlag::UAV)
							type = BindType::UAV;
						else
							type = BindType::CBV;

						descLayoutFlags.bindingCount = 1;
						descLayoutFlags.pBindingFlags = &bindingFlags;
						descLayoutInfo.bindingCount = 1;
						descLayoutInfo.pBindings = &binding;

						for (U32 j = 0; j < entry.Count; ++j)
						{
							bindings[i++] = { type, static_cast<U8>(entry.RangeSlot + j) };
							VkDescriptorSetLayout descLayout = VK_NULL_HANDLE;
							ZE_VK_THROW_NOSUCC(vkCreateDescriptorSetLayout(device, &descLayoutInfo, nullptr, &descLayout));

							sets.emplace_back(descLayout);
							++binding.binding;
						}
					}
				}
			}
		}
		// For any remaining bindings
		createBindingTable();

		// Rearange descriptor sets so each one would be on correct index
		ZE_ASSERT(sets.size() < 255, "Too many descriptor sets!");
		for (U8 i = 0; i < sets.size(); ++i)
		{
			if (bindings[i].Type != BindType::Constant)
			{
				ZE_ASSERT(bindings[i].DescSetIndex < sets.size(), "Incorrect descriptor set index!");
				std::swap(sets.at(i), sets.at(bindings[i].DescSetIndex));
			}
		}

		samplersCount = static_cast<U32>(desc.Samplers.size());
		if (samplersCount)
		{
			VkDescriptorSetLayoutBinding samplerBindings = {};
			samplerBindings.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			samplerBindings.descriptorCount = 1;
			samplerBindings.stageFlags = VK_SHADER_STAGE_ALL;

			descLayoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
			descLayoutInfo.bindingCount = 1;
			descLayoutInfo.pBindings = &samplerBindings;

			VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr };
			samplerInfo.flags = 0;
			samplerInfo.compareEnable = VK_TRUE;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;

			samplers = new VkSampler[samplersCount];
			for (U32 i = 0; const auto & samplerDesc : desc.Samplers)
			{
				// Load data for samplers
				samplerInfo.magFilter = GetFilterTypeMag(samplerDesc.Type);
				samplerInfo.minFilter = GetFilterTypeMin(samplerDesc.Type);
				samplerInfo.mipmapMode = GetFilterTypeMip(samplerDesc.Type);
				samplerInfo.addressModeU = GetTextureAddressMode(samplerDesc.Address.U);
				samplerInfo.addressModeV = GetTextureAddressMode(samplerDesc.Address.V);
				samplerInfo.addressModeW = GetTextureAddressMode(samplerDesc.Address.W);
				samplerInfo.mipLodBias = samplerDesc.MipLevelBias;
				samplerInfo.anisotropyEnable = GetFilterTypeIsAnisotropic(samplerDesc.Type);
				samplerInfo.maxAnisotropy = static_cast<float>(samplerDesc.MaxAnisotropy);
				samplerInfo.compareOp = GetComparisonFunc(samplerDesc.Comparison);
				samplerInfo.minLod = samplerDesc.MinLOD;
				samplerInfo.maxLod = samplerDesc.MaxLOD;
				samplerInfo.borderColor = GetBorderColor(samplerDesc.EdgeColor);

				// Create immutable samplers
				VkSampler sampler = VK_NULL_HANDLE;
				ZE_VK_THROW_NOSUCC(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));
				samplers[i++] = sampler;

				// Create set layout for samplers
				samplerBindings.binding = samplerDesc.Slot;
				samplerBindings.pImmutableSamplers = &sampler;

				VkDescriptorSetLayout descLayout = VK_NULL_HANDLE;
				ZE_VK_THROW_NOSUCC(vkCreateDescriptorSetLayout(device, &descLayoutInfo, nullptr, &descLayout));
				sets.emplace_back(descLayout);
			}
		}

		// Create final layout
		VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr };
		layoutInfo.flags = 0;
		layoutInfo.setLayoutCount = static_cast<U32>(sets.size());
		layoutInfo.pSetLayouts = sets.data();
		layoutInfo.pushConstantRangeCount = static_cast<U32>(constants.size());
		layoutInfo.pPushConstantRanges = constants.data();
		ZE_VK_THROW_NOSUCC(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout));
	}

	void Schema::Free(GFX::Device& dev) noexcept
	{
		if (bindings)
			bindings.DeleteArray();
		if (samplers)
		{
			for (U32 i = 0; i < samplersCount; ++i)
				vkDestroySampler(dev.Get().vk.GetDevice(), samplers[i], nullptr);
			samplers.DeleteArray();
		}
	}

	void Schema::SetCompute(GFX::CommandList& cl) const noexcept
	{
		ZE_ASSERT(isCompute, "Schema is not created for compute pass!");
		for (U32 i = 0; i < samplersCount; ++i)
			vkCmdBindDescriptorBufferEmbeddedSamplersEXT(cl.Get().vk.GetBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, layout, count + i);
	}

	void Schema::SetGraphics(GFX::CommandList& cl) const noexcept
	{
		ZE_ASSERT(!isCompute, "Schema is not created for graphics pass!");
		for (U32 i = 0; i < samplersCount; ++i)
			vkCmdBindDescriptorBufferEmbeddedSamplersEXT(cl.Get().vk.GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, count + i);
	}
}