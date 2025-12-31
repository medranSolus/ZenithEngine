#include "Data/CubemapSource.h"

namespace ZE::Data
{
	CubemapSource& CubemapSource::operator=(const CubemapSource& source) noexcept
	{
		switch (source.Type)
		{
		case CubemapSourceType::SingleFileCubemap:
		{
			InitSingleFileCubemap(source.Data[0]);
			break;
		}
		case CubemapSourceType::Folder:
		{
			InitFolder(source.Data[0], source.Data[1]);
			break;
		}
		case CubemapSourceType::CubemapFiles:
		{
			InitCubemapFiles(source.Data[0], source.Data[1],
				source.Data[2], source.Data[3],
				source.Data[4], source.Data[5]);
			break;
		}
		default:
			ZE_ENUM_UNHANDLED();
		case CubemapSourceType::Empty:
		{
			Type = CubemapSourceType::Empty;
			Data = nullptr;
			break;
		}
		}
		return *this;
	}

	void CubemapSource::InitSingleFileCubemap(std::string_view path) noexcept
	{
		Type = CubemapSourceType::SingleFileCubemap;
		Data = std::make_unique<std::string[]>(1);
		Data[0] = path;
	}

	void CubemapSource::InitFolder(std::string_view directory, std::string_view ext) noexcept
	{
		Type = CubemapSourceType::Folder;
		Data = std::make_unique<std::string[]>(2);
		Data[0] = directory;
		Data[1] = ext;
	}

	void CubemapSource::InitCubemapFiles(std::string_view px, std::string_view nx,
		std::string_view py, std::string_view ny,
		std::string_view pz, std::string_view nz) noexcept
	{
		Type = CubemapSourceType::CubemapFiles;
		Data = std::make_unique<std::string[]>(6);
		Data[0] = px;
		Data[1] = nx;
		Data[2] = py;
		Data[3] = ny;
		Data[4] = pz;
		Data[5] = nz;
	}

	bool CubemapSource::LoadTextures(std::vector<GFX::Surface>& textures) const noexcept
	{
		bool result = false;
		if (Data != nullptr)
		{
			switch (Type)
			{
			case Data::CubemapSourceType::SingleFileCubemap:
			{
				result = textures.emplace_back().Load(Data[0]);
				break;
			}
			case Data::CubemapSourceType::Folder:
			{
				textures.reserve(6);
				result = textures.emplace_back().Load(Data[0] + "/px" + Data[1]); // Right
				if (result)
					result &= textures.emplace_back().Load(Data[0] + "/nx" + Data[1]); // Left
				if (result)
					result &= textures.emplace_back().Load(Data[0] + "/py" + Data[1]); // Up
				if (result)
					result &= textures.emplace_back().Load(Data[0] + "/ny" + Data[1]); // Down
				if (result)
					result &= textures.emplace_back().Load(Data[0] + "/pz" + Data[1]); // Front
				if (result)
					result &= textures.emplace_back().Load(Data[0] + "/nz" + Data[1]); // Back
				break;
			}
			case Data::CubemapSourceType::CubemapFiles:
			{
				textures.reserve(6);
				result = textures.emplace_back().Load(Data[0]); // Right
				if (result)
					result &= textures.emplace_back().Load(Data[1]); // Left
				if (result)
					result &= textures.emplace_back().Load(Data[2]); // Up
				if (result)
					result &= textures.emplace_back().Load(Data[3]); // Down
				if (result)
					result &= textures.emplace_back().Load(Data[4]); // Front
				if (result)
					result &= textures.emplace_back().Load(Data[5]); // Back
				break;
			}
			default:
				ZE_ENUM_UNHANDLED();
			case Data::CubemapSourceType::Empty:
				break;
			}
		}
		return result;
	}

	bool CubemapSource::operator!=(const CubemapSource& source) const noexcept
	{
		bool result = true;
		if (Type == source.Type && Data != nullptr && source.Data != nullptr)
		{
			result = false;
			switch (Type)
			{
			case Data::CubemapSourceType::CubemapFiles:
				result = Data[2] != source.Data[2] || Data[3] != source.Data[3]
					|| Data[4] != source.Data[4] || Data[5] != source.Data[5];
				[[fallthrough]];
			case Data::CubemapSourceType::Folder:
				result |= Data[1] != source.Data[1];
				[[fallthrough]];
			case Data::CubemapSourceType::SingleFileCubemap:
				result |= Data[0] != source.Data[0];
				break;
			default:
				ZE_ENUM_UNHANDLED();
			case Data::CubemapSourceType::Empty:
				break;
			}
		}
		return result;
	}
}