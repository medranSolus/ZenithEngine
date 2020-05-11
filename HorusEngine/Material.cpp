#include "Material.h"

namespace GFX::Visual
{
	Material::Material(Graphics& gfx, Data::ColorFloat3 color, const std::string& name)
	{
		AddBind(Resource::PixelShader::Get(gfx, "SolidPS.cso"));
		vertexLayout = std::make_shared<Data::VertexLayout>();
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS.cso");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader->GetBytecode()));
		AddBind(std::move(vertexShader));

		GFX::Data::CBuffer::DCBLayout cbufferLayout;
		cbufferLayout.Add(DCBElementType::Color3, "solidColor"); // TODO: Maybe alpha not needed
		Data::CBuffer::DynamicCBuffer cbuffer(std::move(cbufferLayout));
		cbuffer["solidColor"] = std::move(color);
		pixelBuffer = Resource::ConstBufferExPixelCache::Get(gfx, name, std::move(cbuffer), 1U);
	}

	Material::Material(Graphics& gfx, aiMaterial& material, const std::string& path)
	{
		GFX::Data::CBuffer::DCBLayout cbufferLayout;
		cbufferLayout.Add(DCBElementType::Color3, "specularColor");
		cbufferLayout.Add(DCBElementType::Float, "specularIntensity");
		cbufferLayout.Add(DCBElementType::Float, "specularPower");
		bool hasAlpha = false;
		bool hasTexture = false;
		aiString texFile;
		std::string shaderCodePS = "PhongPS";
		std::string shaderCodeVS = "PhongVS";
		vertexLayout = std::make_shared<Data::VertexLayout>();
		vertexLayout->Append(VertexAttribute::Normal);

		// Get diffuse texture
		if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFile) == aiReturn_SUCCESS)
		{
			hasTexture = true;
			diffuseTexture = Resource::Texture::Get(gfx, path + std::string(texFile.C_Str()), 0U, true);
			hasAlpha = diffuseTexture->HasAlpha();
			shaderCodePS += "Texture";
			shaderCodeVS += "Texture";
			vertexLayout->Append(VertexAttribute::Texture2D);
		}
		else
			cbufferLayout.Add(DCBElementType::Color4, "materialColor"); // TODO: Maybe alpha not needed

		// Get normal map texture
		if (material.GetTexture(aiTextureType_NORMALS, 0, &texFile) == aiReturn_SUCCESS)
		{
			hasTexture = true;
			normalMap = Resource::Texture::Get(gfx, path + std::string(texFile.C_Str()), 1U);
			shaderCodePS += "Normal";
			shaderCodeVS += "Normal";
			vertexLayout->Append(VertexAttribute::Texture2D).Append(VertexAttribute::Tangent).Append(VertexAttribute::Bitangent);
			cbufferLayout.Add(DCBElementType::Float, "normalMapWeight");
		}

		// Get specular data
		if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFile) == aiReturn_SUCCESS)
		{
			hasTexture = true;
			specularMap = Resource::Texture::Get(gfx, path + std::string(texFile.C_Str()), 2U, true);
			shaderCodePS += "Specular";
			vertexLayout->Append(VertexAttribute::Texture2D);
			cbufferLayout.Add(DCBElementType::Bool, "useSpecularPowerAlpha");
		}

		// Common elements
		AddBind(Resource::Rasterizer::Get(gfx, hasAlpha)); // TODO: Better way to check for double sided meshes (and transparent too)
		AddBind(Resource::Blender::Get(gfx, false));
		AddBind(Resource::PixelShader::Get(gfx, shaderCodePS + ".cso"));
		auto vertexShader = Resource::VertexShader::Get(gfx, shaderCodeVS + ".cso");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader->GetBytecode()));
		AddBind(std::move(vertexShader));
		if (hasTexture)
			AddBind(Resource::Sampler::Get(gfx));

		// Material elements
		Data::CBuffer::DynamicCBuffer cbuffer(std::move(cbufferLayout));
		if (material.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(static_cast<Data::ColorFloat3&>(cbuffer["specularColor"]))) != aiReturn_SUCCESS)
			cbuffer["specularColor"] = std::move(Data::ColorFloat3(1.0f, 1.0f, 1.0f));
		if (material.Get(AI_MATKEY_SHININESS_STRENGTH, static_cast<float&>(cbuffer["specularIntensity"])) != aiReturn_SUCCESS)
			cbuffer["specularIntensity"] = 0.9f;
		if (material.Get(AI_MATKEY_SHININESS, static_cast<float&>(cbuffer["specularPower"])) != aiReturn_SUCCESS)
			cbuffer["specularPower"] = 40.0f;
		if (cbuffer["materialColor"].Exists())
			if (material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor4D&>(static_cast<Data::ColorFloat4&>(cbuffer["materialColor"]))) != aiReturn_SUCCESS)
				cbuffer["materialColor"] = std::move(Data::ColorFloat4(0.0f, 0.8f, 1.0f));
		cbuffer["normalMapWeight"].SetIfExists(1.0f);
		if (specularMap != nullptr)
			cbuffer["useSpecularPowerAlpha"] = specularMap->HasAlpha();
		// Maybe path needed too, TODO: Check this
		pixelBuffer = Resource::ConstBufferExPixelCache::Get(gfx, material.GetName().C_Str(), std::move(cbuffer), 1U);
	}

	void Material::Bind(Graphics& gfx) noexcept
	{
		IVisual::Bind(gfx);
		pixelBuffer->Bind(gfx);
		if (diffuseTexture)
			diffuseTexture->Bind(gfx);
		if (normalMap)
			normalMap->Bind(gfx);
		if (specularMap)
			specularMap->Bind(gfx);
	}

	void Material::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		// Add later for turning on or of some texture stuff
		pixelBuffer->Accept(gfx, probe);
	}
}