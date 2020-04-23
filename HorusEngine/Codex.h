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
		std::shared_ptr<T> Find(Graphics& gfx, Params&& ...p);
		template<typename T, typename ...Params>
		inline bool CheckIfNotStored(Params&& ...p) const noexcept;

		Codex() = default;

	public:
		~Codex() = default;

		template<typename T, typename ...Params>
		static inline std::shared_ptr<T> Resolve(Graphics& gfx, Params&& ...p) { return Get().Find<T>(gfx, std::forward<Params>(p)...); }
		template<typename T, typename ...Params>
		static inline bool NotStored(Params&& ...p) noexcept { return Get().CheckIfNotStored<T>(std::forward<Params>(p)...); }
	};

	template<typename T, typename ...Params>
	inline std::shared_ptr<T> Codex::Find(Graphics& gfx, Params&& ...p)
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

	template<typename T, typename ...Params>
	inline bool Codex::CheckIfNotStored(Params&& ...p) const noexcept
	{
		return binds.end() == binds.find(IBindable::GenerateRID<T>(std::forward<Params>(p)...));
	}
}