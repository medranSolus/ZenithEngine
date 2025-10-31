#include "Data/SkyboxSource.h"

namespace ZE::Data
{
	SkyboxSource& SkyboxSource::operator=(const SkyboxSource& source) noexcept
	{
		switch (source.Type)
		{
		case SkyboxType::SingleFileCubemap:
		{
			InitSingleFileCubemap(source.Data[0]);
			break;
		}
		default:
			ZE_ENUM_UNHANDLED();
		case SkyboxType::Folder:
		{
			InitFolder(source.Data[0], source.Data[1]);
			break;
		}
		case SkyboxType::CubemapFiles:
		{
			InitCubemapFiles(source.Data[0], source.Data[1],
				source.Data[2], source.Data[3],
				source.Data[4], source.Data[5]);
			break;
		}
		}
		return *this;
	}

	void SkyboxSource::InitSingleFileCubemap(std::string_view path) noexcept
	{
		Type = SkyboxType::SingleFileCubemap;
		Data = std::make_unique<std::string[]>(1);
		Data[0] = path;
	}

	void SkyboxSource::InitFolder(std::string_view directory, std::string_view ext) noexcept
	{
		Type = SkyboxType::Folder;
		Data = std::make_unique<std::string[]>(2);
		Data[0] = directory;
		Data[1] = ext;
	}

	void SkyboxSource::InitCubemapFiles(std::string_view px, std::string_view nx,
		std::string_view py, std::string_view ny,
		std::string_view pz, std::string_view nz) noexcept
	{
		Type = SkyboxType::CubemapFiles;
		Data = std::make_unique<std::string[]>(6);
		Data[0] = px;
		Data[1] = nx;
		Data[2] = py;
		Data[3] = ny;
		Data[4] = pz;
		Data[5] = nz;
	}

	bool SkyboxSource::operator!=(const SkyboxSource& source) const noexcept
	{
		bool result = true;
		if (Type == source.Type && Data != nullptr && source.Data != nullptr)
		{
			result = false;
			switch (Type)
			{
			default:
				ZE_ENUM_UNHANDLED();
			case Data::SkyboxType::CubemapFiles:
				result = Data[2] != source.Data[2] || Data[3] != source.Data[3]
					|| Data[4] != source.Data[4] || Data[5] != source.Data[5];
				[[fallthrough]];
			case Data::SkyboxType::Folder:
				result |= Data[1] != source.Data[1];
				[[fallthrough]];
			case Data::SkyboxType::SingleFileCubemap:
				result |= Data[0] != source.Data[0];
				break;
			}
		}
		return result;
	}
}