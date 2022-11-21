#pragma once
#include "Types.h"
#include <type_traits>
#include <cstdlib>
#include <cstring>

namespace ZE
{
	template<typename T>
	concept TableIndex = std::is_unsigned_v<T>;

	// Structure holding table sizes
	template<TableIndex I>
	struct TableInfo
	{
		I Size = 0;
		I Allocated = 0;
	};

	// Utility class for managing operations on tables
	class Table final
	{
		template<typename T, TableIndex I, U8 ALIGNMENT_POWER = 0>
		static constexpr T* Alloc(I count) noexcept;
		template<typename T, TableIndex I, U8 ALIGNMENT_POWER = 0>
		static constexpr T* IncrementAlloc(Ptr<T> data, I oldSize, I newSize) noexcept;
		template<typename T, TableIndex I, U8 ALIGNMENT_POWER = 0>
		static constexpr T* DecrementAlloc(Ptr<T> data, I newSize) noexcept;

	public:
		Table() = delete;

		template<typename T, TableIndex I, U8 ALIGNMENT_POWER = 0>
		static constexpr T* Create(I size) noexcept;
		template<typename T, TableIndex I, U8 ALIGNMENT_POWER = 0>
		static constexpr T* Create(const TableInfo<I>& info) noexcept;
		template<typename T, TableIndex I, U8 ALIGNMENT_POWER = 0>
		static constexpr T* Create(I size, const T& initData) noexcept;
		template<typename T, TableIndex I, U8 ALIGNMENT_POWER = 0>
		static constexpr T* Create(I size, Ptr<T> sourceData) noexcept;

		template<typename T, TableIndex I>
		static constexpr void Clear(I size, Ptr<T>& data) noexcept;
		template<typename T, TableIndex I>
		static constexpr void Clear(TableInfo<I>& info, Ptr<T>& data) noexcept;

		// Standard resize operation for single-column table
		template<TableIndex I, typename T, U8 ALIGNMENT_POWER = 0>
		static constexpr void Resize(TableInfo<I>& info, Ptr<T>& data, I newSize) noexcept;
		// Begins resize operation on multi-column tables. Call this on each column
		template<TableIndex I, typename T, U8 ALIGNMENT_POWER = 0>
		static constexpr void ResizeBegin(const TableInfo<I>& info, Ptr<T>& data, I newSize) noexcept;
		// Finishes resize operation on multi-column tables. Call this once after every column in a table is resized
		template<TableIndex I>
		static constexpr void ResizeEnd(TableInfo<I>& info, I newSize) noexcept;

		// Standard insert operation for single-column table
		template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER = 0, typename ...P>
		static constexpr void Insert(TableInfo<I>& info, Ptr<T>& data, I index, P&&... params) noexcept;

		// Standard append operation for single-column table
		template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER = 0, typename ...P>
		static constexpr void Append(TableInfo<I>& info, Ptr<T>& data, P&&... params) noexcept;
		// Begins append operation on multi-column tables. Call this on each column
		template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER = 0, typename ...P>
		static constexpr void AppendBegin(const TableInfo<I>& info, Ptr<T>& data, P&&... params) noexcept;
		// Finishes append operation on multi-column tables. Call this once after every column in a new row is added
		template<U64 CHUNK_SIZE, TableIndex I>
		static constexpr void AppendEnd(TableInfo<I>& info) noexcept;

		// Standard remove element operation for single-column table
		template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER = 0>
		static constexpr void Remove(TableInfo<I>& info, Ptr<T>& data, I element) noexcept;

		// Standard pop back operation for single-column table
		template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER = 0>
		static constexpr void PopBack(TableInfo<I>& info, Ptr<T>& data) noexcept;
		// Begins pop back operation on multi-column tables. Call this on each column
		template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER = 0>
		static constexpr void PopBackBegin(const TableInfo<I>& info, Ptr<T>& data) noexcept;
		// Finishes pop back operation on multi-column tables. Call this once after every column in an old row is deleted
		template<U64 CHUNK_SIZE, TableIndex I>
		static constexpr void PopBackEnd(TableInfo<I>& info) noexcept;
	};

#pragma region Functions
	template<typename T, TableIndex I, U8 ALIGNMENT_POWER>
	constexpr T* Table::Alloc(I count) noexcept
	{
		static_assert(ALIGNMENT_POWER < 64, "ALIGNMENT_POWER of Table must be smaller than 64!");

		if constexpr (ALIGNMENT_POWER == 0)
			return reinterpret_cast<T*>(malloc(sizeof(T) * count));
		else
			return reinterpret_cast<T*>(aligned_alloc(1 << ALIGNMENT_POWER, sizeof(T) * count));
	}

	template<typename T, TableIndex I, U8 ALIGNMENT_POWER>
	constexpr T* Table::IncrementAlloc(Ptr<T> data, I oldSize, I newSize) noexcept
	{
		ZE_ASSERT(data, "Data empty!");
		ZE_ASSERT(newSize > oldSize, "Function not made for decrementation!");

		if constexpr (ALIGNMENT_POWER == 0)
			return reinterpret_cast<T*>(realloc(data, sizeof(T) * newSize));
		else
		{
			T* ptr = Alloc<T>(newSize);
			memcpy(ptr, data, sizeof(T) * oldSize);
			free(data);
			return ptr;
		}
	}

	template<typename T, TableIndex I, U8 ALIGNMENT_POWER>
	constexpr T* Table::DecrementAlloc(Ptr<T> data, I newSize) noexcept
	{
		T* ptr = reinterpret_cast<T*>(realloc(data, sizeof(T) * newSize));
		// Check if realloc returns same region of memory,
		// if no then handling for alingned mem would be needed
		ZE_ASSERT(ptr == static_cast<T*>(data), "Realloc moved memory!");
		return ptr;
	}

	template<typename T, TableIndex I, U8 ALIGNMENT_POWER>
	constexpr T* Table::Create(I size) noexcept
	{
		T* data = Alloc<T>(size);
		if constexpr (!std::is_integral_v<T>)
			for (I i = 0; i < size; ++i)
				new(data + i) T;
		return data;
	}

	template<typename T, TableIndex I, U8 ALIGNMENT_POWER>
	constexpr T* Table::Create(const TableInfo<I>& info) noexcept
	{
		T* data = Alloc<T>(info.Allocated);
		if constexpr (!std::is_integral_v<T>)
			for (I i = 0; i < info.Size; ++i)
				new(data + i) T;
		return data;
	}

	template<typename T, TableIndex I, U8 ALIGNMENT_POWER>
	constexpr T* Table::Create(I size, const T& initData) noexcept
	{
		T* data = Alloc<T>(size);
		for (I i = 0; i < size; ++i)
			new(data + i) T(initData);
		return data;
	}

	template<typename T, TableIndex I, U8 ALIGNMENT_POWER>
	constexpr T* Table::Create(I size, Ptr<T> sourceData) noexcept
	{
		ZE_ASSERT(sourceData, "Data empty!");
		T* data = Alloc<T>(size);
		if constexpr (std::is_trivial_v<T>)
			memcpy(data, sourceData, sizeof(T) * size);
		else
			for (I i = 0; i < size; ++i)
				new(data + i) T(sourceData[i]);
		return data;
	}

	template<typename T, TableIndex I>
	constexpr void Table::Clear(I size, Ptr<T>& data) noexcept
	{
		ZE_ASSERT(data, "Data empty!");
		if constexpr (!std::is_trivially_destructible_v<T>)
			for (I i = 0; i < size; ++i)
				data[i].~T();
		free(data);
		data = nullptr;
	}

	template<typename T, TableIndex I>
	constexpr void Table::Clear(TableInfo<I>& info, Ptr<T>& data) noexcept
	{
		Clear(info.Size, data);
	}

	template<TableIndex I, typename T, U8 ALIGNMENT_POWER>
	constexpr void Table::Resize(TableInfo<I>& info, Ptr<T>& data, I newSize) noexcept
	{
		ZE_ASSERT(data, "Data empty!");
		ZE_ASSERT(newSize != 0, "Cannot resize to empty table!");
		ZE_ASSERT(info.Size != newSize, "No need to change size to same one!");

		if (info.Size > newSize)
		{
			data = DecrementAlloc(data, newSize);
			info.Allocated = newSize;
			if constexpr (!std::is_trivially_destructible_v<T>)
				for (I i = newSize; i < info.Size; ++i)
					data[i].~T();
		}
		else
		{
			if (info.Allocated != newSize)
			{
				data = IncrementAlloc(data, info.Size, newSize);
				info.Allocated = newSize;
			}
			if constexpr (!std::is_integral_v<T>)
				for (I i = info.Size; i < newSize; ++i)
					new(data + i) T;
		}
		info.Size = newSize;
	}

	template<TableIndex I, typename T, U8 ALIGNMENT_POWER>
	constexpr void Table::ResizeBegin(const TableInfo<I>& info, Ptr<T>& data, I newSize) noexcept
	{
		ZE_ASSERT(data, "Data empty!");
		ZE_ASSERT(newSize != 0, "Cannot resize to empty table!");
		ZE_ASSERT(info.Size != newSize, "No need to change size to same one!");

		if (info.Size > newSize)
		{
			data = DecrementAlloc(data, newSize);
			if constexpr (!std::is_trivially_destructible_v<T>)
				for (I i = newSize; i < info.Size; ++i)
					data[i].~T();
		}
		else
		{
			if (info.Allocated != newSize)
				data = IncrementAlloc(data, info.Size, newSize);
			if constexpr (!std::is_integral_v<T>)
				for (I i = info.Size; i < newSize; ++i)
					new(data + i) T;
		}
	}

	template<TableIndex I>
	constexpr void Table::ResizeEnd(TableInfo<I>& info, I newSize) noexcept
	{
		info.Allocated = newSize;
		info.Size = newSize;
	}

	template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER, typename ...P>
	constexpr void Table::Insert(TableInfo<I>& info, Ptr<T>& data, I index, P&&... params) noexcept
	{
		if (info.Size == info.Allocated)
		{
			info.Allocated += CHUNK_SIZE;
			data = IncrementAlloc(data, info.Size, info.Allocated);
		}
		memcpy(data + index + 1, data + index, sizeof(T) * (info.Size - index));
		++info.Size;
		new (data + index) T(std::forward<P>(params)...);
	}

	template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER, typename ...P>
	constexpr void Table::Append(TableInfo<I>& info, Ptr<T>& data, P&&... params) noexcept
	{
		if (info.Size == info.Allocated)
		{
			info.Allocated += CHUNK_SIZE;
			data = IncrementAlloc(data, info.Size, info.Allocated);
		}
		new (data + info.Size) T(std::forward<P>(params)...);
		++info.Size;
	}

	template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER, typename ...P>
	constexpr void Table::AppendBegin(const TableInfo<I>& info, Ptr<T>& data, P&&... params) noexcept
	{
		if (info.Size == info.Allocated)
			data = IncrementAlloc(data, info.Size, info.Allocated + CHUNK_SIZE);
		new (data + info.Size) T(std::forward<P>(params)...);
	}

	template<U64 CHUNK_SIZE, TableIndex I>
	constexpr void Table::AppendEnd(TableInfo<I>& info) noexcept
	{
		if (info.Size == info.Allocated)
			info.Allocated += CHUNK_SIZE;
		++info.Size;
	}

	template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER>
	constexpr void Table::Remove(TableInfo<I>& info, Ptr<T>& data, I element) noexcept
	{
		ZE_ASSERT(element < info.Size, "Element out of range!");

		if constexpr (!std::is_trivially_destructible_v<T>)
			data[element].~T();
		memcpy(data + element, data + element + 1, sizeof(T) * (info.Size - (element + 1)));
		--info.Size;
		if (info.Size + CHUNK_SIZE < info.Allocated)
		{
			info.Allocated -= CHUNK_SIZE;
			data = DecrementAlloc(data, info.Allocated);
		}
	}

	template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER>
	constexpr void Table::PopBack(TableInfo<I>& info, Ptr<T>& data) noexcept
	{
		ZE_ASSERT(info.Size != 0, "Empty table!");

		--info.Size;
		if constexpr (!std::is_trivially_destructible_v<T>)
			data[info.Size].~T();
		if (info.Size + CHUNK_SIZE < info.Allocated)
		{
			info.Allocated -= CHUNK_SIZE;
			data = DecrementAlloc(data, info.Allocated);
		}
	}

	template<U64 CHUNK_SIZE, TableIndex I, typename T, U8 ALIGNMENT_POWER>
	constexpr void Table::PopBackBegin(const TableInfo<I>& info, Ptr<T>& data) noexcept
	{
		if constexpr (!std::is_trivially_destructible_v<T>)
			data[info.Size - 1].~T();
		if (info.Size + CHUNK_SIZE <= info.Allocated)
			data = DecrementAlloc(data, info.Allocated - CHUNK_SIZE);
	}

	template<U64 CHUNK_SIZE, TableIndex I>
	constexpr void Table::PopBackEnd(TableInfo<I>& info) noexcept
	{
		--info.Size;
		if (info.Size + CHUNK_SIZE < info.Allocated)
			info.Allocated -= CHUNK_SIZE;
	}
#pragma endregion
}