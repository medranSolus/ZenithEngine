#pragma once
#include "Entity.h"

namespace ZE::Data
{
	// Main facility for accessing data storages related to game data and it's resources
	class SystemsBank final
	{
	public:
		// Identifier for loading data to which source entity it belongs depending on storage location
		struct WorldSourceID { EID ID = INVALID_EID; };

		// Tags for signaling that data can be loaded into the main registry
		struct WorldObjectLoaded {};
		struct AssetLoaded {};

	private:
		Storage worldData;
		Storage assetsData;
		Storage loadingData;
		std::shared_mutex loadingMutex;

		void MergeNode(EID nodeId, EID destId) noexcept;

	public:
		SystemsBank() noexcept;
		ZE_CLASS_MOVE(SystemsBank);
		~SystemsBank() = default;

		constexpr Storage& GetWorldData() noexcept { return worldData; }
		constexpr Storage& GetAssetsData() noexcept { return assetsData; }
		constexpr Storage& GetLoadingData() noexcept { return loadingData; }
		constexpr std::shared_mutex& GetLoadingLock() noexcept { return loadingMutex; }

		void MergeLoadedData() noexcept;
		void ClearStorage() noexcept;
	};
}