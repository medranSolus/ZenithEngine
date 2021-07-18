#pragma once
#include "Types.h"
#include <type_traits>
#include <cstdlib>
#include <cstring>

namespace ZE
{
	template<typename T>
	concept TableIndex = std::is_unsigned_v<T>;

	template<TableIndex I>
	struct TableInfo
	{
		I Size;
		I Allocated;
	};

	// Utility class for managing operations on tables
	class Table final
	{
		template<typename T, TableIndex I, U8 AlignmentPower = 0>
		static constexpr T* Alloc(I count) noexcept;
		template<typename T, TableIndex I, U8 AlignmentPower = 0>
		static constexpr T* IncrementAlloc(TableInfo<I>& info, T* data, I newSize) noexcept;
		template<typename T, TableIndex I, U8 AlignmentPower = 0>
		static constexpr T* DecrementAlloc(TableInfo<I>& info, T* data, I newSize) noexcept;

	public:
		Table() = delete;

		template<typename T, TableIndex I, U8 AlignmentPower = 0>
		static constexpr T* Create(I size) noexcept;
		template<typename T, TableIndex I, U8 AlignmentPower = 0>
		static constexpr T* Create(I size, const T& initData) noexcept;
		template<typename T, TableIndex I, U8 AlignmentPower = 0>
		static constexpr T* Create(I size, T* sourceData) noexcept;

		template<typename T, TableIndex I>
		static constexpr void Clear(I size, T* data) noexcept;
		template<typename T, TableIndex I, U8 AlignmentPower = 0>
		static constexpr T* Resize(TableInfo<I>& info, T* data, I newSize) noexcept;

		template<typename T, TableIndex I, U64 ChunkSize = 8, U8 AlignmentPower = 0>
		static constexpr T* Append(TableInfo<I>& info, T* data, T&& element) noexcept;
		template<typename T, TableIndex I, U64 ChunkSize = 8, U8 AlignmentPower = 0, typename ...P>
		static constexpr T* Append(TableInfo<I>& info, T* data, P&&... params) noexcept;
		template<typename T, TableIndex I, U64 ChunkSize = 8, U8 AlignmentPower = 0>
		static constexpr T* PopBack(TableInfo<I>& info, T* data) noexcept;
	};

#pragma region Functions
	template<typename T, TableIndex I, U8 AlignmentPower>
	constexpr T* Table::Alloc(I count) noexcept
	{
		static_assert(AlignmentPower < 64, "AlignmentPower of Table must be smaller than 64!");

		if constexpr (AlignmentPower == 0)
			return reinterpret_cast<T*>(malloc(sizeof(T) * count));
		else
			return reinterpret_cast<T*>(aligned_alloc(1 << AlignmentPower, sizeof(T) * count));
	}

	template<typename T, TableIndex I, U8 AlignmentPower>
	constexpr T* Table::IncrementAlloc(TableInfo<I>& info, T* data, I newSize) noexcept
	{
		assert(data && newSize > info.Size);
		if constexpr (AlignmentPower == 0)
			return reinterpret_cast<T*>(realloc(data, sizeof(T) * newSize));
		else
		{
			T* ptr = Alloc<T>(newSize);
			memcpy(ptr, data, sizeof(T) * info.Size);
			free(data);
			return ptr;
		}
	}

	template<typename T, TableIndex I, U8 AlignmentPower>
	constexpr T* Table::DecrementAlloc(TableInfo<I>& info, T* data, I newSize) noexcept
	{
		assert(data && newSize < info.Size);
		T* ptr = reinterpret_cast<T*>(realloc(data, sizeof(T) * newSize));
		// Check if realloc returns same region of memory,
		// if no then handling for alingned mem would be needed
		assert(ptr == data);
		return ptr;
	}

	template<typename T, TableIndex I, U8 AlignmentPower>
	constexpr T* Table::Create(I size) noexcept
	{
		T* data = Alloc<T>(size);
		if constexpr (!std::is_integral_v<T>)
			for (I i = 0; i < size; ++i)
				new(data + i) T;
		return data;
	}

	template<typename T, TableIndex I, U8 AlignmentPower>
	constexpr T* Table::Create(I size, const T& initData) noexcept
	{
		T* data = Alloc<T>(size);
		for (I i = 0; i < size; ++i)
			new(data + i) T(initData);
		return data;
	}

	template<typename T, TableIndex I, U8 AlignmentPower>
	constexpr T* Table::Create(I size, T* sourceData) noexcept
	{
		assert(sourceData);
		T* data = Alloc<T>(size);
		if constexpr (std::is_trivial_v<T>)
			memcpy(data, sourceData, sizeof(T) * size);
		else
			for (I i = 0; i < size; ++i)
				new(data + i) T(sourceData[i]);
		return data;
	}

	template<typename T, TableIndex I>
	constexpr void Table::Clear(I size, T* data) noexcept
	{
		assert(size && data);
		if constexpr (!std::is_trivially_destructible_v<T>)
			for (I i = 0; i < size; ++i)
				data[i].~T();
		free(data);
	}

	template<typename T, TableIndex I, U8 AlignmentPower>
	constexpr T* Table::Resize(TableInfo<I>& info, T* data, I newSize) noexcept
	{
		assert(data && info.Size != newSize);
		if (info.Size > newSize)
		{
			data = DecrementAlloc(info, data, newSize);
			info.Allocated = newSize;
			if constexpr (!std::is_trivially_destructible_v<T>)
				for (I i = newSize; i < info.Size; ++i)
					data[i].~T();
		}
		else
		{
			if (info.Allocated != newSize)
			{
				data = IncrementAlloc(info, data, newSize);
				info.Allocated = newSize;
			}
			if constexpr (!std::is_integral_v<T>)
				for (I i = info.Size; i < newSize; ++i)
					new(data + i) T;
		}
		info.Size = newSize;
		return data;
	}
	
	template<typename T, TableIndex I, U64 ChunkSize, U8 AlignmentPower>
	constexpr T* Table::Append(TableInfo<I>& info, T* data, T&& element) noexcept
	{
		if (info.Size >= info.Allocated)
		{
			info.Allocated += ChunkSize;
			data = IncrementAlloc(info, data, info.Allocated);
		}
		new (data + info.Size) T(std::forward<T>(element));
		info.Size++;
		return data;
	}

	template<typename T, TableIndex I, U64 ChunkSize, U8 AlignmentPower, typename ...P>
	constexpr T* Table::Append(TableInfo<I>& info, T* data, P&&... params) noexcept
	{
		if (info.Size >= info.Allocated)
		{
			info.Allocated += ChunkSize;
			data = IncrementAlloc(info, data, info.Allocated);
		}
		new (data + info.Size) T(std::forward<P>(params)...);
		info.Size++;
		return data;
	}

	template<typename T, TableIndex I, U64 ChunkSize, U8 AlignmentPower>
	constexpr T* Table::PopBack(TableInfo<I>& info, T* data) noexcept
	{
		--info.Size;
		if constexpr (!std::is_trivially_destructible_v<T>)
			data[info.Size].~T();
		if (info.Size < info.Allocated - ChunkSize)
		{
			info.Allocated -= ChunkSize;
			data = DecrementAlloc(info, data, info.Allocated);
		}
		return data;
	}
#pragma endregion
}