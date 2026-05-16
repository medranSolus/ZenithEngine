#pragma once
#include "DX12.h"

namespace ZE::RHI::DX12
{
	// Resource information holding reference to created resource
	struct ResourceInfo
	{
		DX::ComPtr<IResource> Resource = nullptr;
		AllocHandle Handle = 0;

		ResourceInfo() = default;
		ZE_CLASS_COPY_ONLY(ResourceInfo);
		constexpr ResourceInfo(ResourceInfo&& res) noexcept;
		constexpr ResourceInfo& operator=(ResourceInfo&& res) noexcept;
		~ResourceInfo() = default;

		bool IsFree() const noexcept { return Resource == nullptr && Handle == 0; }
	};

#pragma region Functions
	constexpr ResourceInfo::ResourceInfo(ResourceInfo&& res) noexcept
		: Resource(std::move(res.Resource)), Handle(res.Handle)
	{
		res.Handle = nullptr;
	}

	constexpr ResourceInfo& ResourceInfo::operator=(ResourceInfo&& res) noexcept
	{
		Resource = std::move(res.Resource);
		// Needed for safe deallocation
		std::swap(Handle, res.Handle);
		return *this;
	}
#pragma endregion
}