#pragma once
#include "GfxResPtr.h"

namespace ZE::GFX::Resource
{
	template<ShaderType T>
	class NullShader : public IBindable
	{
	public:
		constexpr NullShader(Graphics& gfx) noexcept {}
		virtual ~NullShader() = default;

		static std::string GenerateRID() noexcept { return std::to_string(T); }
		static GfxResPtr<NullShader> Get(Graphics& gfx) noexcept { return Codex::Resolve<NullShader>(gfx); }
		std::string GetRID() const noexcept override { return GenerateRID(); }

		void Bind(Graphics& gfx) const override;
	};

	template<ShaderType T>
	struct is_resolvable_by_codex<NullShader<T>>
	{
		static constexpr bool GENERATE_ID{ true };
	};

#pragma region Functions
	template<ShaderType T>
	void NullShader<T>::Bind(Graphics& gfx) const
	{
		if constexpr (T == ShaderType::Geometry)
			GetContext(gfx)->GSSetShader(nullptr, nullptr, 0);
		else if constexpr (T == ShaderType::Pixel)
			GetContext(gfx)->PSSetShader(nullptr, nullptr, 0);
		else
		{
			assert(false && "Not all null shaders have defined Bind function!");
		}
	}
#pragma endregion

	typedef NullShader<ShaderType::Geometry> NullGeometryShader;
	typedef NullShader<ShaderType::Pixel> NullPixelShader;
}