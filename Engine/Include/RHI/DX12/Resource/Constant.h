#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::DX12::Resource
{
	template<typename T>
	class Constant final
	{
		T data = {};

	public:
		Constant() = default;
		ZE_CLASS_MOVE(Constant);
		~Constant() = default;

		static constexpr Expected<Constant> Create(GFX::Device& dev, const T& value) noexcept;

		constexpr Status Set(GFX::Device& dev, const T& value) noexcept { data = value; return {}; }

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept;
	};

#pragma region Functions
	template<typename T>
	constexpr Expected<Constant<T>> Constant<T>::Create(GFX::Device& dev, const T& value) noexcept
	{
		Constant c = {};
		c.data = value;
		return c;
	}

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
			{
				ZE_DX_CHECK_FAILED(list->SetComputeRoot32BitConstant(bindCtx.Count++, *reinterpret_cast<U32*>(reinterpret_cast<uintptr_t>(&data)), 0), "Setting compute constant resulted in debug layer messages!");
			}
			else
			{
				ZE_DX_CHECK_FAILED(list->SetComputeRoot32BitConstants(bindCtx.Count++, sizeof(T) / 4, &data, 0), "Setting compute constants resulted in debug layer messages!");
			}
		}
		else
		{
			if constexpr (sizeof(T) == 4)
			{
				ZE_DX_CHECK_FAILED(list->SetGraphicsRoot32BitConstant(bindCtx.Count++, *reinterpret_cast<U32*>(reinterpret_cast<uintptr_t>(&data)), 0), "Setting GFX constant resulted in debug layer messages!");
			}
			else
			{
				ZE_DX_CHECK_FAILED(list->SetGraphicsRoot32BitConstants(bindCtx.Count++, sizeof(T) / 4, &data, 0), "Setting GFX constants resulted in debug layer messages!");
			}
		}
	}
#pragma endregion
}