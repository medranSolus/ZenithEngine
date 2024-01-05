#include "Data/AssetsStreamer.h"
#include "Data/MaterialPBR.h"
#include "Data/Tags.h"
#include "GFX/Vertex.h"
#include "GUI/DialogWindow.h"
#include "IO/Format/ResourcePackFile.h"
#include "IO/File.h"

namespace ZE::Data
{
#if _ZE_EXTERNAL_MODEL_LOADING
	template<typename Index>
	void AssetsStreamer::ParseIndices(Index* indices, const aiMesh& mesh) noexcept
	{
		for (U32 i = 0, index = 0; i < mesh.mNumFaces; ++i)
		{
			const aiFace& face = mesh.mFaces[i];
			ZE_ASSERT(face.mNumIndices == 3, "Mesh face is not a triangle!");

			indices[index++] = Utils::SafeCast<Index>(face.mIndices[0]);
			indices[index++] = Utils::SafeCast<Index>(face.mIndices[1]);
			indices[index++] = Utils::SafeCast<Index>(face.mIndices[2]);
		}
	}
#endif

	void AssetsStreamer::Init(GFX::Device& dev)
	{
		diskManager.Init(dev);

		// Initialize schema for known materials
		GFX::Resource::Texture::Schema pbrTextureSchema;
		pbrTextureSchema.AddTexture(MaterialPBR::TEX_COLOR_NAME, GFX::Resource::Texture::Type::Tex2D);
		pbrTextureSchema.AddTexture(MaterialPBR::TEX_NORMAL_NAME, GFX::Resource::Texture::Type::Tex2D);
		pbrTextureSchema.AddTexture(MaterialPBR::TEX_SPECULAR_NAME, GFX::Resource::Texture::Type::Tex2D);
		pbrTextureSchema.AddTexture(MaterialPBR::TEX_HEIGHT_NAME, GFX::Resource::Texture::Type::Tex2D);
		texSchemaLib.Add(MaterialPBR::TEX_SCHEMA_NAME, std::move(pbrTextureSchema));
	}

	void AssetsStreamer::Free(GFX::Device& dev)
	{
	}

	Task<IO::FileStatus> AssetsStreamer::LoadResourcePack(GFX::Device& dev, std::string_view packFile)
	{
		return Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
			[&]() -> IO::FileStatus
			{
				IO::File file;
				if (!file.Open(diskManager, packFile, IO::FileFlag::GpuReading))
					return IO::FileStatus::ErrorOpeningFile;

				IO::Format::ResourcePackFileHeader header = {};
				if (!file.Read(&header, sizeof(header), 0))
					return IO::FileStatus::ErrorReading;
				// Check if signature is correct first and resources are present
				if (std::memcmp(header.Signature, IO::Format::ResourcePackFileHeader::SIGNATURE_STR, 4) != 0)
					return IO::FileStatus::ErrorBadSignature;
				if (header.ResourceCount == 0)
					return IO::FileStatus::ErrorNoResources;

				// Prepare IDs for all created entities
				std::vector<EID> resourceIds(header.ResourceCount);
				Settings::CreateEntities(resourceIds);
				Storage& assets = Settings::Data;

				// Handle files according to version
				IO::FileStatus result = IO::FileStatus::Ok;
				switch (header.Version)
				{
				case Utils::MakeVersion(1, 0, 0):
				{
					const U32 infoSectionSize = header.ResourceCount * sizeof(IO::Format::ResourcePackEntry)
						+ header.TexturesCount * sizeof(IO::Format::ResourcePackTextureEntry) + header.NameSectionSize;
					std::unique_ptr<U8[]> infoSection = std::make_unique<U8[]>(infoSectionSize);
					auto tableWait = file.ReadAsync(infoSection.get(), infoSectionSize, sizeof(header));

					const IO::Format::ResourcePackEntry* resourceTable = reinterpret_cast<const IO::Format::ResourcePackEntry*>(infoSection.get() + sizeof(header));
					const IO::Format::ResourcePackTextureEntry* textureTable = reinterpret_cast<const IO::Format::ResourcePackTextureEntry*>(resourceTable + header.ResourceCount);
					const char* nameTable = reinterpret_cast<const char*>(textureTable + header.TexturesCount);

					// Wait for last read and check if correct numer of bytes are read
					if (tableWait.get() != infoSectionSize)
					{
						result = IO::FileStatus::ErrorReading;
						break;
					}

					// First check for integrity of resources and loading of CPU only data
					U32 resIdIndex = 0;
					for (U32 i = 0; i < header.ResourceCount; ++i)
					{
						auto& entry = resourceTable[i];

						EID resId = resourceIds.at(resIdIndex++);
						assets.emplace<PackID>(resId, header.ID);

						std::string& name = assets.emplace<std::string>(resId);
						name.resize(entry.NameSize);
						std::memcpy(name.data(), nameTable + entry.NameIndex, entry.NameSize);

						// Check for correct type of resource pack entry
						switch (entry.Type)
						{
						case IO::Format::ResourcePackEntryType::Geometry:
						{
							assets.emplace<Math::BoundingBox>(resId, entry.Geometry.BoxCenter, entry.Geometry.BoxExtents);
							break;
						}
						case IO::Format::ResourcePackEntryType::Material:
						case IO::Format::ResourcePackEntryType::Buffer:
						case IO::Format::ResourcePackEntryType::Textures:
						default:
						{
							result = IO::FileStatus::ErrorUnknownResourceEntry;
							i = header.ResourceCount;
							break;
						}
						}
					}
					// If integrity check failed then abort resource pack
					if (result != IO::FileStatus::Ok)
						break;

					// Final processing of GPU resources
					resIdIndex = 0;
					for (U32 i = 0; i < header.ResourceCount; ++i)
					{
						auto& entry = resourceTable[i];
						EID resId = resourceIds.at(resIdIndex++);

						switch (entry.Type)
						{
						case IO::Format::ResourcePackEntryType::Geometry:
						{
							// Load and parse mesh data into correct asset
							GFX::Resource::MeshFileData data = {};
							data.MeshID = resId;
							data.MeshDataOffset = entry.Geometry.Offset;
							data.VertexCount = entry.Geometry.VertexCount;
							data.IndexCount = entry.Geometry.IndexCount;
							data.SourceBytes = entry.Geometry.Bytes;
							data.UncompressedSize = entry.Geometry.UncompressedSize;
							data.VertexSize = entry.Geometry.VertexSize;
							data.IndexFormat = entry.Geometry.IndexBufferFormat;
							data.Compression = entry.Geometry.Compression;
							assets.emplace<GFX::Resource::Mesh>(resId, dev, diskManager, data, file);
							break;
						}
						case IO::Format::ResourcePackEntryType::Material:
						case IO::Format::ResourcePackEntryType::Buffer:
						case IO::Format::ResourcePackEntryType::Textures:
						default:
							break;
						}
					}
					break;
				}
				default:
				{
					result = IO::FileStatus::ErrorUnknowVersion;
					break;
				}
				}
				// If error occured abort all resources
				if (result != IO::FileStatus::Ok)
					assets.destroy(resourceIds.begin(), resourceIds.end());

				return result;
			});
	}

	Task<IO::FileStatus> AssetsStreamer::SaveResourcePack(GFX::Device& dev, std::string_view packFile, U16 packId, IO::CompressionFormat defaultCompression)
	{
		return Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
			[&]() -> IO::FileStatus
			{
				Storage& assets = Settings::Data;

				// Gather all resources for given group
				std::vector<EID> resourceIds;
				for (EID entity : assets.view<PackID>())
					if (assets.get<PackID>(entity).ID == packId)
						resourceIds.emplace_back(entity);

				if (resourceIds.size() == 0)
					return IO::FileStatus::ErrorNoResources;

				IO::File file;
				if (!file.Open(diskManager, packFile, IO::FileFlag::WriteOnly))
					return IO::FileStatus::ErrorOpeningFile;

				// Save general header info
				IO::Format::ResourcePackFileHeader header = {};
				header.Signature[0] = IO::Format::ResourcePackFileHeader::SIGNATURE_STR[0];
				header.Signature[1] = IO::Format::ResourcePackFileHeader::SIGNATURE_STR[1];
				header.Signature[2] = IO::Format::ResourcePackFileHeader::SIGNATURE_STR[2];
				header.Signature[3] = IO::Format::ResourcePackFileHeader::SIGNATURE_STR[3];
				header.Version = Utils::MakeVersion(1, 0, 0);
				header.ResourceCount = Utils::SafeCast<U32>(resourceIds.size());
				header.TexturesCount = 0;
				header.NameSectionSize = 0;
				header.ID = packId;
				header.Flags = IO::Format::ResourcePackFlag::None;

				std::vector<std::pair<U32, std::future<U32>>> results;
				results.emplace_back(Utils::SafeCast<U32>(sizeof(header)), file.WriteAsync(&header, sizeof(header), 0));

				// Prepare resource table
				auto resourceInfoTable = std::make_unique<IO::Format::ResourcePackEntry[]>(header.ResourceCount);
				// Offset for files that will be written into disk
				U64 dataOffset = 0;
				U32 nameOffset = 0;
				for (U32 i = 0; EID entity : resourceIds)
				{
					auto& entry = resourceInfoTable[i++];
					entry.NameIndex = nameOffset;
					entry.NameSize = Utils::SafeCast<U16>(assets.get<std::string>(entity).size());
					nameOffset += entry.NameSize;

					if (auto* mesh = assets.try_get<GFX::Resource::Mesh>(entity))
					{
						entry.Type = IO::Format::ResourcePackEntryType::Geometry;
						entry.Geometry.Offset = dataOffset;
						entry.Geometry.Bytes = mesh->GetSize(); // TODO NOW: Currently no compression, add simple zlib ones
						entry.Geometry.UncompressedSize = mesh->GetSize();
						entry.Geometry.BoxCenter = assets.get<Math::BoundingBox>(entity).Center;
						entry.Geometry.BoxExtents = assets.get<Math::BoundingBox>(entity).Extents;
						entry.Geometry.VertexCount = mesh->GetVertexCount();
						entry.Geometry.IndexCount = mesh->GetIndexCount();
						entry.Geometry.VertexSize = mesh->GetVertexSize();
						entry.Geometry.IndexBufferFormat = mesh->GetIndexFormat();

						// If custom compression specified then use this one
						auto* compression = assets.try_get<IO::CompressionFormat>(entity);
						entry.Geometry.Compression = compression ? *compression : defaultCompression;

						if (entry.Geometry.IndexBufferFormat == PixelFormat::R8_UInt)
						{
							// TODO: save index buffer as R16 format due to compatibility with other RHI
						}
						// TODO: load data from GPU and write to file
					}

					// TODO: handle other types of resources
				}
				return IO::FileStatus::Ok;
			});
	}

#if _ZE_EXTERNAL_MODEL_LOADING
	Task<MeshID> AssetsStreamer::ParseMesh(GFX::Device& dev, const aiMesh& mesh)
	{
		return Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
			[&]() -> MeshID
			{
				GFX::Resource::MeshData meshData = {};
				meshData.VertexCount = mesh.mNumVertices;
				meshData.IndexCount = mesh.mNumFaces * 3;
				meshData.VertexSize = sizeof(GFX::Vertex);

				// Gather index data and parse it into continuous array
				if (meshData.IndexCount >= UINT16_MAX)
				{
					meshData.IndexSize = sizeof(U32);
					meshData.PackedMesh = std::make_shared<U8[]>(meshData.IndexCount * sizeof(U32) + meshData.VertexCount * sizeof(GFX::Vertex));
					ParseIndices(reinterpret_cast<U32*>(meshData.PackedMesh.get()), mesh);
				}
				else if (!Settings::IsEnabledU8IndexBuffers() || meshData.IndexCount >= UINT8_MAX)
				{
					meshData.IndexSize = sizeof(U16);
					meshData.PackedMesh = std::make_shared<U8[]>(Math::AlignUp(meshData.IndexCount * sizeof(U16), static_cast<U64>(GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT)) + meshData.VertexCount * sizeof(GFX::Vertex));
					ParseIndices(reinterpret_cast<U16*>(meshData.PackedMesh.get()), mesh);
				}
				else
				{
					meshData.IndexSize = sizeof(U8);
					meshData.PackedMesh = std::make_shared<U8[]>(Math::AlignUp(meshData.IndexCount, GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT) + meshData.VertexCount * sizeof(GFX::Vertex));
					ParseIndices(meshData.PackedMesh.get(), mesh);
				}

				// Parse vertex data into structured format just after index array
				Vector min = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
				Vector max = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };

				GFX::Vertex* vertices = reinterpret_cast<GFX::Vertex*>(meshData.PackedMesh.get() + Math::AlignUp(meshData.IndexCount * meshData.IndexSize, GFX::Resource::MeshData::VERTEX_BUFFER_ALIGNMENT));
				for (U32 i = 0; i < mesh.mNumVertices; ++i)
				{
					GFX::Vertex& vertex = vertices[i];

					vertex.Position = *reinterpret_cast<Float3*>(mesh.mVertices + i);
					ZE_ASSERT(vertex.Position.x == mesh.mVertices[i].x
						&& vertex.Position.y == mesh.mVertices[i].y
						&& vertex.Position.z == mesh.mVertices[i].z,
						"Position data is incorrect, type of aiMesh::mVertices changed!");
					// Find min/max position values to determine bounding box
					const Vector pos = Math::XMLoadFloat3(&vertex.Position);
					min = Math::XMVectorMin(min, pos);
					max = Math::XMVectorMax(max, pos);

					vertex.Normal = *reinterpret_cast<Float3*>(mesh.mNormals + i);
					ZE_ASSERT(vertex.Normal.x == mesh.mNormals[i].x
						&& vertex.Normal.y == mesh.mNormals[i].y
						&& vertex.Normal.z == mesh.mNormals[i].z,
						"Normal data is incorrect, type of aiMesh::mNormals changed!");

					if (mesh.HasTextureCoords(0))
					{
						vertex.UV = *reinterpret_cast<Float2*>(mesh.mTextureCoords[0] + i);
						ZE_ASSERT(vertex.UV.x == mesh.mTextureCoords[0][i].x
							&& vertex.UV.y == mesh.mTextureCoords[0][i].y,
							"UV data is incorrect, type of aiMesh::mTextureCoords changed!");
					}
					if (mesh.HasTangentsAndBitangents())
					{
						vertex.Tangent.x = mesh.mTangents[i].x;
						vertex.Tangent.y = mesh.mTangents[i].y;
						vertex.Tangent.z = mesh.mTangents[i].z;
						//vertex.Tangent.w = -1.0f; // TODO: Check somehow
					}
				}

				// Create main mesh data
				EID meshId = Settings::CreateEntity();
				Storage& assets = Settings::Data;
				meshData.MeshID = meshId;

				// Load custom data by default to resource pack 0
				assets.emplace<PackID>(meshId).ID = 0;
				assets.emplace<std::string>(meshId, mesh.mName.length != 0
					? mesh.mName.C_Str() : "mesh_" + std::to_string(static_cast<U64>(meshId)));
				assets.emplace<Math::BoundingBox>(meshId, Math::GetBoundingBox(max, min));

				// Load parsed mesh data into correct mesh and start it's upload to GPU
				assets.emplace<GFX::Resource::Mesh>(meshId, dev, diskManager, meshData);
				return { meshId };
			});
	}

	Task<MaterialID> AssetsStreamer::ParseMaterial(GFX::Device& dev, const aiMaterial& material, const std::string& path)
	{
		return Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
			[&, path = path]() -> MaterialID
			{
				EID materialId = Settings::CreateEntity();
				Storage& assets = Settings::Data;

				assets.emplace<std::string>(materialId, material.GetName().length != 0
					? material.GetName().C_Str() : "material_" + std::to_string(static_cast<U64>(materialId)));

				MaterialPBR& data = assets.emplace<MaterialPBR>(materialId);
				PBRFlags& flags = assets.emplace<PBRFlags>(materialId);

				const GFX::Resource::Texture::Schema& texSchema = texSchemaLib.Get(MaterialPBR::TEX_SCHEMA_NAME);
				GFX::Resource::Texture::PackDesc texDesc;
				texDesc.Init(texSchema);
				texDesc.ResourceID = materialId;

				aiString texFile;
				bool notSolid = false;

				// Get diffuse texture
				if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFile) == aiReturn_SUCCESS)
				{
					std::vector<GFX::Surface> surfaces;
					if (surfaces.emplace_back().Load(path + texFile.C_Str()))
					{
						notSolid |= surfaces.back().HasAlpha();

						texDesc.AddTexture(texSchema, MaterialPBR::TEX_COLOR_NAME, std::move(surfaces));
						flags |= MaterialPBR::Flag::UseTexture;
					}
				}

				// Get normal map texture
				if (material.GetTexture(aiTextureType_NORMALS, 0, &texFile) == aiReturn_SUCCESS)
				{
					std::vector<GFX::Surface> surfaces;
					if (surfaces.emplace_back().Load(path + texFile.C_Str()))
					{
						texDesc.AddTexture(texSchema, MaterialPBR::TEX_NORMAL_NAME, std::move(surfaces));
						flags |= MaterialPBR::Flag::UseNormal;
					}
				}

				// Get specular data
				if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFile) == aiReturn_SUCCESS)
				{
					std::vector<GFX::Surface> surfaces;
					if (surfaces.emplace_back().Load(path + texFile.C_Str()))
					{
						if (surfaces.back().HasAlpha())
							data.Flags |= MaterialPBR::Flag::UseSpecularPowerAlpha;
						texDesc.AddTexture(texSchema, MaterialPBR::TEX_SPECULAR_NAME, std::move(surfaces));
						flags |= MaterialPBR::Flag::UseSpecular;
					}
				}

				// Get parallax map texture
				if (material.GetTexture(aiTextureType_HEIGHT, 0, &texFile) == aiReturn_SUCCESS)
				{
					std::vector<GFX::Surface> surfaces;
					if (surfaces.emplace_back().Load(path + texFile.C_Str()))
					{
						texDesc.AddTexture(texSchema, MaterialPBR::TEX_HEIGHT_NAME, std::move(surfaces));
						flags |= MaterialPBR::Flag::UseParallax;
						notSolid = true;
					}
				}

				if (material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor4D&>(data.Color)) != aiReturn_SUCCESS)
					data.Color = { 0.0f, 0.8f, 1.0f };
				else if (data.Color.RGBA.w != 1.0f)
					notSolid = true;

				if (material.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(data.Specular)) != aiReturn_SUCCESS)
					data.Specular = { 1.0f, 1.0f, 1.0f };

				if (material.Get(AI_MATKEY_SHININESS_STRENGTH, data.SpecularIntensity) != aiReturn_SUCCESS)
					data.SpecularIntensity = 0.9f;

				if (material.Get(AI_MATKEY_SHININESS, data.SpecularPower) != aiReturn_SUCCESS)
					data.SpecularPower = 0.409f;
				else if (data.SpecularPower > 1.0f)
					data.SpecularPower = Utils::SafeCast<float>(log(static_cast<double>(data.SpecularPower)) / log(8192.0));

				if (material.Get(AI_MATKEY_BUMPSCALING, data.ParallaxScale) != aiReturn_SUCCESS)
					data.ParallaxScale = 0.1f;

				// Indicate that material require special handling
				if (notSolid)
					assets.emplace<MaterialNotSolid>(materialId);

				// Load custom data by default to resource pack 0
				assets.emplace<PackID>(materialId).ID = 0;
				// Start upload of buffer data and textures to GPU
				assets.emplace<MaterialBuffersPBR>(materialId, dev, diskManager, data, texDesc);
				return { materialId };
			});
	}
#endif

	void AssetsStreamer::ShowWindow(GFX::Device& dev)
	{
		if (ImGui::Begin("Resource Manager"))
		{
			if (ImGui::CollapsingHeader("Resource packs"))
			{
				if (auto path = GUI::DialogWindow::FileBrowserButton("Load pack", RESOURCE_DIR, GUI::DialogWindow::FileType::ResourcePack))
				{
					auto result = LoadResourcePack(dev, path.value());
					auto val = result.Get();
					if (val != IO::FileStatus::Ok)
						throw ZE_IO_EXCEPT(("Cannot load resource pack \"" + path.value() + "\"! Error: ") + IO::GetFileStatusString(val));
				}
			}
			ImGui::End();
		}
	}
}