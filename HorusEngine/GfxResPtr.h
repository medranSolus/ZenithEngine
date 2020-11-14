#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	template<typename R>
	template<typename ...Params>
	ResPtr<R>::ResPtr(Params && ...p)
	{
		uint8_t* memory = static_cast<uint8_t*>(::operator new(sizeof(R) + sizeof(uint64_t), ALIGNMENT));
		ptr = new(memory) R(std::forward<Params>(p)...);
		count = (uint64_t*)(memory + sizeof(R));
		*count = 1U;
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
				uint8_t* memory = (uint8_t*)ptr;
				::operator delete(memory, ALIGNMENT);
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