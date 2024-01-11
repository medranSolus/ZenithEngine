#include "RHI/VK/Resource/Texture/Pack.h"
#include "RHI/VK/VulkanException.h"

namespace ZE::RHI::VK::Resource::Texture
{
	Pack::Pack(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::Texture::PackDesc& desc)
	{
		ZE_VK_ENABLE_ID();
		Device& device = dev.Get().vk;

		count = Utils::SafeCast<U32>(desc.Textures.size());
		// Something about descriptors
		images = new VkImage[count];
		resources = new Allocation[count];

		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr };
		imageInfo.mipLevels = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 0;
		imageInfo.pQueueFamilyIndices = nullptr;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		for (U32 i = 0; const auto& tex : desc.Textures)
		{
			U16 surfaces = Utils::SafeCast<U16>(tex.Surfaces.size());
			if (surfaces)
			{
				auto& startSurface = tex.Surfaces.front();
				imageInfo.format = GetVkFormat(startSurface.GetFormat());
				imageInfo.extent.width = Utils::SafeCast<U32>(startSurface.GetWidth());
				imageInfo.extent.height = Utils::SafeCast<U32>(startSurface.GetHeight());

				switch (tex.Type)
				{
				default:
					ZE_ENUM_UNHANDLED();
				case GFX::Resource::Texture::Type::Tex2D:
				{
					imageInfo.flags = 0;
					imageInfo.imageType = VK_IMAGE_TYPE_2D;
					imageInfo.extent.depth = 1;
					imageInfo.arrayLayers = 1;
					break;
				}
				case GFX::Resource::Texture::Type::Tex3D:
				{
					imageInfo.flags = 0;
					imageInfo.imageType = VK_IMAGE_TYPE_3D;
					imageInfo.extent.depth = surfaces;
					imageInfo.arrayLayers = 1;
					break;
				}
				case GFX::Resource::Texture::Type::Cube:
				{
					imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
					imageInfo.imageType = VK_IMAGE_TYPE_2D;
					imageInfo.extent.depth = 1;
					imageInfo.arrayLayers = 6;
					break;
				}
				case GFX::Resource::Texture::Type::Array:
				{
					imageInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
					imageInfo.imageType = VK_IMAGE_TYPE_2D;
					imageInfo.extent.depth = 1;
					imageInfo.arrayLayers = surfaces;
					break;
				}
				}

				ZE_VK_THROW_NOSUCC(vkCreateImage(device.GetDevice(), &imageInfo, nullptr, images + i));
				resources[i] = device.GetMemory().AllocImage(device, images[i], Allocation::Usage::GPU);
			}
			else
			{
				imageInfo.format = GetVkFormat(Settings::BackbufferFormat);
			}
			++i;
		}
	}

	Pack::Pack(GFX::Device& dev, IO::DiskManager& disk, const GFX::Resource::Texture::PackFileDesc& desc, IO::File& file)
	{
	}

	Pack::~Pack()
	{
	}

	void Pack::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
	}

	void Pack::Free(GFX::Device& dev) noexcept
	{
	}

	std::vector<std::vector<GFX::Surface>> Pack::GetData(GFX::Device& dev) const
	{
		std::vector<std::vector<GFX::Surface>> vec;
		vec.emplace_back(std::vector<GFX::Surface>());
		vec.front().emplace_back(1U, 1U);
		return vec;
	}
}