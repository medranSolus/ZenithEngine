#pragma once
#include "GFX/Surface.h"

namespace ZE::Data
{
	// Information about cubemap source data format
	enum class CubemapSourceType : U8
	{
		// Cubemap texture is located in single file
		// as array of 6 textures in order: px, nx, py, ny, pz, nz
		// Data contents: single string with path to file
		SingleFileCubemap,
		// Specified folder contains 6 files named as above
		// that each one holds single surface of the cubemap (with same extension)
		// Data contents: string with path to the folder (ex. "dir/skybox"), extension of the files (ex. ".png")
		Folder,
		// Paths to 6 different surfaces representing cubemap
		// Data contents: 6 cubemap files in order as above
		CubemapFiles,
		// No cubemap present
		Empty
	};

	// Information about source of the skybox
	struct CubemapSource final
	{
		CubemapSourceType Type = CubemapSourceType::Empty;
		std::unique_ptr<std::string[]> Data = nullptr;

		CubemapSource() = default;
		ZE_CLASS_MOVE_ONLY(CubemapSource);
		CubemapSource(const CubemapSource& source) noexcept { *this = source; }
		CubemapSource& operator=(const CubemapSource& source) noexcept;
		~CubemapSource() = default;

		void InitSingleFileCubemap(std::string_view path) noexcept;
		void InitFolder(std::string_view directory, std::string_view ext) noexcept;
		void InitCubemapFiles(std::string_view px, std::string_view nx,
			std::string_view py, std::string_view ny,
			std::string_view pz, std::string_view nz) noexcept;

		bool LoadTextures(std::vector<GFX::Surface>& textures) const noexcept;

		bool operator==(const CubemapSource& source) const noexcept { return !(*this != source); };
		bool operator!=(const CubemapSource& source) const noexcept;
	};
}