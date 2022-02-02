#pragma once
#include "GFX/Device.h"
#include "GFX/Binding/Context.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Resource
{
	template<typename T>
	class Constant final
	{
		T data;

	public:
		Constant() = default;
		constexpr Constant(GFX::Device& dev, const T& value) noexcept : data(value) {}
		ZE_CLASS_MOVE(Constant);
		~Constant() = default;

		constexpr T& GetData() noexcept { return data; }
		constexpr void Set(const T& value) noexcept { data = value; }

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
	};

#pragma region Functions
	template<typename T>
	void Constant<T>::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept
	{
		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::Constant,
			"Bind slot is not a constant! Wrong root signature or order of bindings!");

		auto* list = cl.Get().dx12.GetList();
		if (schema.IsCompute())
		{
			if constexpr (sizeof(T) == 4)
				list->SetComputeRoot32BitConstant(bindCtx.Count++, data, 0);
			else
				list->SetComputeRoot32BitConstants(bindCtx.Count++, sizeof(T) / 4, &data, 0);
		}
		else
		{
			if constexpr (sizeof(T) == 4)
				list->SetGraphicsRoot32BitConstant(bindCtx.Count++, data, 0);
			else
				list->SetGraphicsRoot32BitConstants(bindCtx.Count++, sizeof(T) / 4, &data, 0);
		}
	}
#pragma endregion
}