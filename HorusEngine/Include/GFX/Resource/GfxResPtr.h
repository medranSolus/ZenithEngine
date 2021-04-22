#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	template<typename R>
	template<typename ...Params>
	ResPtr<R>::ResPtr(Graphics& gfx, Params&& ...p)
	{
		U8* memory = static_cast<U8*>(::operator new(sizeof(R) + sizeof(U64), ALIGNMENT));
		ptr = new(memory) R(gfx, std::forward<Params>(p)...);
		count = (U64*)(memory + sizeof(R));
		*count = 1;
	}

	template<typename R>
	ResPtr<R>::~ResPtr()
	{
		if (count)
		{
			switch (--(*count))
			{
			case 0:
			{
				ptr->~R();
				U8* memory = (U8*)ptr;
				::operator delete(memory, ALIGNMENT);
				break;
			}
			case 1:
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
	constexpr ResPtr<R>& ResPtr<R>::operator=(ResPtr&& rp) noexcept
	{
		count = rp.count;
		ptr = rp.ptr;
		rp.count = nullptr;
		rp.ptr = nullptr;
		return *this;
	}

	template<typename R>
	constexpr ResPtr<R>& ResPtr<R>::operator=(const ResPtr& rp) noexcept
	{
		count = rp.count;
		ptr = rp.ptr;
		++(*count);
		return *this;
	}
}

template<typename R>
using GfxResPtr = GFX::Resource::ResPtr<R>;