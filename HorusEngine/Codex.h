#pragma once
#include "IBindable.h"
#include <unordered_map>

namespace GFX::Resource
{
	class Codex
	{
		std::unordered_map<std::string, std::shared_ptr<IBindable>> binds;

		static inline Codex& Get() noexcept
		{
			static Codex codex;
			return codex;
		}

		template<typename T, typename ...Params>
		std::shared_ptr<T> Find(Graphics& gfx, Params&& ...p) noexcept
		{
			const std::string id = IBindable::GenerateRID<T>(std::forward<Params>(p)...);
			const auto it = binds.find(id);
			if (it == binds.end())
			{
				auto bind = std::make_shared<T>(gfx, std::forward<Params>(p)...);
				binds[id] = bind;
				return bind;
			}
			return std::dynamic_pointer_cast<T>(it->second);
		}

	public:
		template<typename T, typename ...Params>
		static inline std::shared_ptr<T> Resolve(Graphics& gfx, Params&& ...p) noexcept
		{
			return Get().Find<T>(gfx, std::forward<Params>(p)...);
		}
	};
}
