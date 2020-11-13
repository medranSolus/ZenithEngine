#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	template<typename R>
	template<typename ...Params>
	ResPtr<R>::ResPtr(Params && ...p) noexcept
	{
		uint8_t* memory = new uint8_t[sizeof(size_t) + sizeof(R)];
		count = (size_t*)memory;
		*count = 1U;
		ptr = new(memory + sizeof(size_t)) R(std::forward<Params>(p)...);
	}

	template<typename R>
	ResPtr<R>::~ResPtr() noexcept
	{
		if (count)
		{
			switch (--(*count))
			{
			case 0U:
			{
				ptr->~R();
				uint8_t* memory = (uint8_t*)count;
				delete[] memory;
				break;
			}
			case 1U:
			{
				std::string rid = ptr->GetRID();
				if (rid != IBindable::GetNoCodexRID())
					Codex::Remove(rid);
				break;
			}
			}
		}
	}

	template<typename R>
	constexpr ResPtr<R>& ResPtr<R>::operator=(const ResPtr& rp) noexcept
	{
		count = rp.count;
		ptr = rp.ptr;
		++(*count);
		return *this;
	}

	template<typename R>
	constexpr ResPtr<R>& ResPtr<R>::operator=(ResPtr&& rp) noexcept
	{
		count = rp.count;
		ptr = rp.ptr;
		rp.count = nullptr;
		rp.ptr = nullptr;
		return *this;
	}
}

template<typename R>
using GfxResPtr = GFX::Resource::ResPtr<R>;