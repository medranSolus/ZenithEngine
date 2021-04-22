#pragma once
#include "ResPtr.h"
#include <unordered_map>

namespace GFX::Resource
{
	class Codex final
	{
		bool running = true;
		std::unordered_map<std::string, ResPtr<IBindable>> binds;

		static Codex& Get() noexcept { static Codex codex; return codex; }

		void RemoveResource(const std::string& rid) noexcept { if (running) binds.erase(rid); }

		template<typename T, typename ...Params>
		bool CheckIfNotStored(Params&& ...p) const noexcept;
		template<typename T, typename ...Params>
		ResPtr<T> Find(Graphics& gfx, Params&& ...p) noexcept;

		Codex() = default;

	public:
		~Codex() { running = false; }

		template<typename T, typename ...Params>
		static ResPtr<T> Resolve(Graphics& gfx, Params&& ...p) noexcept { return Get().Find<T>(gfx, std::forward<Params>(p)...); }
		template<typename T, typename ...Params>
		static bool NotStored(Params&& ...p) noexcept { return Get().CheckIfNotStored<T>(std::forward<Params>(p)...); }

		static void Remove(const std::string& rid) noexcept { Get().RemoveResource(rid); }
	};

#pragma region Functions
	template<typename T, typename ...Params>
	bool Codex::CheckIfNotStored(Params&& ...p) const noexcept
	{
		return binds.end() == binds.find(IBindable::GenerateRID<T>(std::forward<Params>(p)...));
	}

	template<typename T, typename ...Params>
	ResPtr<T> Codex::Find(Graphics& gfx, Params&& ...p) noexcept
	{
		const std::string id = IBindable::GenerateRID<T>(std::forward<Params>(p)...);
		const auto& it = binds.find(id);
		if (it == binds.end())
		{
			ResPtr<T> bind(gfx, std::forward<Params>(p)...);
			binds[id] = bind;
			return bind;
		}
		return it->second.CastDynamic<T>();
	}
#pragma endregion
}