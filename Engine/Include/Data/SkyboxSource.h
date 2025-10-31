#pragma once

namespace ZE::Data
{
	// Information about skybox source data format
	enum class SkyboxType : U8
	{
		// Skybox texture is located in single file
		// as array of 6 textures in order: px, nx, py, ny, pz, nz
		// Data contents: single string with path to file
		SingleFileCubemap,
		// Specified folder contains 6 files named as above
		// that each one holds single surface of the skybox (with same extension)
		// Data contents: string with path to the folder (ex. "dir/skybox"), extension of the files (ex. ".png")
		Folder,
		// Paths to 6 different surfaces representing skybox
		// Data contents: 6 cubemap files in order as above
		CubemapFiles
	};

	// Information about source of the skybox
	struct SkyboxSource final
	{
		SkyboxType Type = SkyboxType::Folder;
		std::unique_ptr<std::string[]> Data = nullptr;

		SkyboxSource() = default;
		ZE_CLASS_MOVE_ONLY(SkyboxSource);
		SkyboxSource(const SkyboxSource& source) noexcept { *this = source; }
		SkyboxSource& operator=(const SkyboxSource& source) noexcept;
		~SkyboxSource() = default;

		void InitSingleFileCubemap(std::string_view path) noexcept;
		void InitFolder(std::string_view directory, std::string_view ext) noexcept;
		void InitCubemapFiles(std::string_view px, std::string_view nx,
			std::string_view py, std::string_view ny,
			std::string_view pz, std::string_view nz) noexcept;

		bool operator==(const SkyboxSource& source) const noexcept { return !(*this != source); };
		bool operator!=(const SkyboxSource& source) const noexcept;
	};
}