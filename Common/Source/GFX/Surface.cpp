#include "GFX/Surface.h"
#include "Platform/WinAPI/DirectXTexException.h"

namespace ZE::GFX
{
	Surface::Surface(const std::string& name)
	{
		constexpr DXGI_FORMAT API_PIXEL_FORMAT = API::DX::GetDXFormat(PIXEL_FORMAT);
		ZE_DXT_ENABLE_EXCEPT();
		const std::filesystem::path path(name);
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return std::tolower(c); });
		if (ext == ".dds")
		{
			ZE_DXT_THROW_FAILED(Tex::LoadFromDDSFile(Utils::ToUtf8(name).c_str(), Tex::DDS_FLAGS::DDS_FLAGS_FORCE_RGB, nullptr, scratch),
				"Loading image \"" + name + "\": failed.");
			image = scratch.GetImage(0, 0, 0);

			if (image->format != API_PIXEL_FORMAT)
			{
				Tex::ScratchImage decompressed;
				ZE_DXT_THROW_FAILED(Tex::Decompress(*image, API_PIXEL_FORMAT, decompressed),
					"Decompressing image \"" + name + "\": failed.");
				scratch = std::move(decompressed);
				image = scratch.GetImage(0, 0, 0);
			}
		}
		else if (ext == ".hdr")
		{
			ZE_DXT_THROW_FAILED(Tex::LoadFromHDRFile(Utils::ToUtf8(name).c_str(), nullptr, scratch),
				"Loading image \"" + name + "\": failed.");

			throw ZE_IMG_EXCEPT("HDR textures not implemented!");
		}
		else if (ext == ".tga")
		{
			ZE_DXT_THROW_FAILED(Tex::LoadFromTGAFile(Utils::ToUtf8(name).c_str(), nullptr, scratch),
				"Loading image \"" + name + "\": failed.");

			throw ZE_IMG_EXCEPT("TGA textures not implemented!");
		}
		else
		{
			ZE_DXT_THROW_FAILED(Tex::LoadFromWICFile(Utils::ToUtf8(name).c_str(),
				Tex::WIC_FLAGS::WIC_FLAGS_IGNORE_SRGB, nullptr, scratch),
				"Loading image \"" + name + "\": failed.");
			image = scratch.GetImage(0, 0, 0);

			if (image->format != API_PIXEL_FORMAT)
			{
				Tex::ScratchImage converted;
				ZE_DXT_THROW_FAILED(Tex::Convert(*image, API_PIXEL_FORMAT,
					Tex::TEX_FILTER_FLAGS::TEX_FILTER_DEFAULT, Tex::TEX_THRESHOLD_DEFAULT, converted),
					"Converting image \"" + name + "\": failed.");
				scratch = std::move(converted);
				image = scratch.GetImage(0, 0, 0);
			}
		}
	}

	Surface::Surface(U64 width, U64 height, PixelFormat format)
	{
		ZE_DXT_ENABLE_EXCEPT();
		ZE_DXT_THROW_FAILED(scratch.Initialize2D(API::DX::GetDXFormat(format), width, height, 1U, 1U),
			"Creating image " + std::to_string(width) + "x" + std::to_string(height) + ": failed.");
		image = scratch.GetImage(0, 0, 0);
	}

	bool Surface::IsImage(const std::string& ext) noexcept
	{
		return ext == ".png" ||
			ext == ".jpg" ||
			ext == ".jpeg" ||
			ext == ".bmp" ||
			ext == ".tif" ||
			ext == ".tiff" ||
			ext == ".ico" ||
			ext == ".gif" ||
			ext == ".hdp" ||
			ext == ".jxr" ||
			ext == ".wdp" ||
			ext == ".dds" ||
			ext == ".hdr" ||
			ext == ".tga";
	}

	void Surface::Save(const std::string& filename) const
	{
		ZE_DXT_ENABLE_EXCEPT();
		const std::filesystem::path path = filename;
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return std::tolower(c); });

		if (ext == ".dds")
		{
			ZE_DXT_THROW_FAILED(Tex::SaveToDDSFile(image, scratch.GetImageCount(), scratch.GetMetadata(),
				Tex::DDS_FLAGS::DDS_FLAGS_FORCE_DX10_EXT, Utils::ToUtf8(filename).c_str()),
				"Saving surface to \"" + filename + "\": failed to save.");
		}
		else if (ext == ".hdr")
		{
			ZE_DXT_THROW_FAILED(Tex::SaveToHDRFile(*image, Utils::ToUtf8(filename).c_str()),
				"Saving surface to \"" + filename + "\": failed to save.");
		}
		else if (ext == ".tga")
		{
			ZE_DXT_THROW_FAILED(Tex::SaveToTGAFile(*image, Utils::ToUtf8(filename).c_str()),
				"Saving surface to \"" + filename + "\": failed to save.");
		}
		else
		{
			Tex::WICCodecs codedID;
			if (ext == ".png")
				codedID = Tex::WICCodecs::WIC_CODEC_PNG;
			else if (ext == ".jpg" || ext == ".jpeg")
				codedID = Tex::WICCodecs::WIC_CODEC_JPEG;
			else if (ext == ".bmp")
				codedID = Tex::WICCodecs::WIC_CODEC_BMP;
			else if (ext == ".tif" || ext == ".tiff")
				codedID = Tex::WICCodecs::WIC_CODEC_TIFF;
			else if (ext == ".ico")
				codedID = Tex::WICCodecs::WIC_CODEC_ICO;
			else if (ext == ".gif")
				codedID = Tex::WICCodecs::WIC_CODEC_GIF;
			else if (ext == ".hdp" || ext == ".jxr" || ext == ".wdp")
				codedID = Tex::WICCodecs::WIC_CODEC_WMP;
			throw ZE_IMG_EXCEPT("Saving surface to \"" + filename + "\": not supported file extension.");

			ZE_DXT_THROW_FAILED(Tex::SaveToWICFile(*image, Tex::WIC_FLAGS::WIC_FLAGS_NONE, Tex::GetWICCodec(codedID), Utils::ToUtf8(filename).c_str()),
				"Saving surface to \"" + filename + "\": failed to save.");
		}
	}
}