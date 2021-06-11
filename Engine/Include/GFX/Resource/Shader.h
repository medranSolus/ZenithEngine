#pragma once
#include "GfxResPtr.h"
#include "GfxDebugName.h"
#include "GFX/Graphics.h"
#include <d3dcompiler.h>

namespace ZE::GFX::Resource
{
	template<ShaderType T>
	class Shader : public IBindable
	{
		template<ShaderType>
		struct Desc {};

#pragma region Descriptor Info
		template<> struct Desc<ShaderType::Vertex>
		{
			using ApiShaderType = ID3D11VertexShader;
			static constexpr const char* TYPE_PREFIX = "VS#";
		};
		template<> struct Desc<ShaderType::Geometry>
		{
			using ApiShaderType = ID3D11GeometryShader;
			static constexpr const char* TYPE_PREFIX = "G#";
		};
		template<> struct Desc<ShaderType::Pixel>
		{
			using ApiShaderType = ID3D11PixelShader;
			static constexpr const char* TYPE_PREFIX = "P#";
		};
		template<> struct Desc<ShaderType::Compute>
		{
			using ApiShaderType = ID3D11ComputeShader;
			static constexpr const char* TYPE_PREFIX = "C#";
		};
#pragma endregion

		std::string name;
		Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
		Microsoft::WRL::ComPtr<typename Desc<T>::ApiShaderType> shader;

	public:
		Shader(Graphics& gfx, const std::string& name);
		virtual ~Shader() = default;

		static std::string GenerateRID(const std::string& name) noexcept { return Desc<T>::TYPE_PREFIX + name; }
		static GfxResPtr<Shader> Get(Graphics& gfx, const std::string& name) { return Codex::Resolve<Shader>(gfx, name); }

		constexpr const std::string& GetName() const noexcept { return name; }
		ID3DBlob* GetBytecode() const noexcept { return bytecode.Get(); }
		std::string GetRID() const noexcept override { return GenerateRID(name); }

		void Bind(Graphics& gfx) const override;
		void BindCompute(Graphics& gfx) const override;
	};

	template<ShaderType T>
	struct is_resolvable_by_codex<Shader<T>>
	{
		static constexpr bool GENERATE_ID{ true };
	};

#pragma region Functions
	template<ShaderType T>
	Shader<T>::Shader(Graphics& gfx, const std::string& name) : name(name)
	{
		ZE_GFX_ENABLE_ALL(gfx);
		ZE_GFX_THROW_FAILED(D3DReadFileToBlob(Utils::ToUtf8("Shaders/" + name + ".cso").c_str(), &bytecode));

		if constexpr (T == ShaderType::Vertex)
		{
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateVertexShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &shader));
		}
		else if constexpr (T == ShaderType::Geometry)
		{
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateGeometryShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &shader));
		}
		else if constexpr (T == ShaderType::Pixel)
		{
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreatePixelShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &shader));
		}
		else if constexpr (T == ShaderType::Compute)
		{
			ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateComputeShader(bytecode->GetBufferPointer(),
				bytecode->GetBufferSize(), nullptr, &shader));
		}
		else
			static_assert(false, "Not all shaders have defined constructor!");

		ZE_GFX_SET_RID(shader.Get());
	}

	template<ShaderType T>
	void Shader<T>::Bind(Graphics& gfx) const
	{
		if constexpr (T == ShaderType::Vertex)
			GetContext(gfx)->VSSetShader(shader.Get(), nullptr, 0);
		else if constexpr (T == ShaderType::Geometry)
			GetContext(gfx)->GSSetShader(shader.Get(), nullptr, 0);
		else if constexpr (T == ShaderType::Pixel)
			GetContext(gfx)->PSSetShader(shader.Get(), nullptr, 0);
		else if constexpr (T == ShaderType::Compute)
			BindCompute(gfx);
		else
			static_assert(false, "Not all shaders have defined Bind function!");
	}

	template<ShaderType T>
	void Shader<T>::BindCompute(Graphics& gfx) const
	{
		if constexpr (T == ShaderType::Compute)
			GetContext(gfx)->CSSetShader(shader.Get(), nullptr, 0);
		else
			IBindable::BindCompute(gfx);
	}
#pragma endregion

	typedef Shader<ShaderType::Vertex> VertexShader;
	typedef Shader<ShaderType::Geometry> GeometryShader;
	typedef Shader<ShaderType::Pixel> PixelShader;
	typedef Shader<ShaderType::Compute> ComputeShader;
}