#pragma once
#include "VK.h"

namespace ZE::RHI::VK
{
	// Necessary info for uploading buffer data
	struct UploadInfoBuffer
	{
		VkBindBufferMemoryInfo Dest;
		// Will be filled before uploading data
		VkBindBufferMemoryInfo Staging;
		const void* InitData = nullptr;
		VkBufferCreateInfo CreateInfo;
		VkPipelineStageFlags2 DestStage;
		VkAccessFlags2 DestAccess;
	};

	// Necessary info for uploading texture data
	struct UploadInfoTexture
	{
	};

	// Description of buffer update process
	struct UploadInfoBufferUpdate
	{
		VkBuffer Buffer = VK_NULL_HANDLE;
		const void* Data = nullptr;
		U32 Bytes = 0;
		VkPipelineStageFlags2 DestStage;
		VkAccessFlags2 DestAccess;
		U32 LastUsedQueue;
	};
}