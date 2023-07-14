#pragma once
#include "LockGuard.h"
#include <memory>

namespace ZE::Allocator
{
	// Thread-safe FIFO queue
	template <typename T>
	class BlockingQueue final
	{
		struct Node
		{
			T Data;
			std::unique_ptr<Node> Next;
		};

		std::unique_ptr<Node> head = nullptr;
		Node* tail = nullptr;
		// Always lock them in this order
		mutable std::shared_mutex headMutex;
		mutable std::shared_mutex tailMutex;

	public:
		constexpr BlockingQueue() noexcept;
		constexpr BlockingQueue(BlockingQueue&& queue) noexcept;
		constexpr BlockingQueue(const BlockingQueue& queue) noexcept;
		constexpr BlockingQueue& operator=(BlockingQueue&& queue) noexcept;
		constexpr BlockingQueue& operator=(const BlockingQueue& queue) noexcept;
		~BlockingQueue() = default;

		constexpr bool IsEmpty() const noexcept;
		constexpr U64 Size() const noexcept;
		constexpr void Clear() noexcept;

		constexpr void PushBack(T&& obj) noexcept;
		constexpr void PushBack(const T& obj) noexcept;
		template <typename... Args>
		constexpr T& EmplaceBack(Args&&... args) noexcept;

		constexpr void PopFront() noexcept;
		constexpr bool TryPopFront(T& obj) noexcept;

		constexpr void Swap(BlockingQueue& queue) noexcept;
	};

#pragma region Functions
	template <typename T>
	constexpr BlockingQueue<T>::BlockingQueue() noexcept
	{
		// Waste a bit of space in exchange for better locking mechanism
		head = std::make_unique<Node>();
		tail = head.get();
	}

	template <typename T>
	constexpr BlockingQueue<T>::BlockingQueue(BlockingQueue&& queue) noexcept
	{
		LockGuardRW lockHead(queue.headMutex);
		LockGuardRW lockTail(queue.tailMutex);

		head = std::move(queue.head);
		tail = head.get();
		queue.head = std::make_unique<Node>();
		queue.tail = queue.head.get();
	}

	template <typename T>
	constexpr BlockingQueue<T>::BlockingQueue(const BlockingQueue& queue) noexcept
	{
		LockGuardRO lockHead(queue.headMutex);
		LockGuardRO lockTail(queue.tailMutex);

		head = std::make_unique<Node>();
		tail = head.get();
		Node* node = queue.head.get();
		while (node != queue.tail)
		{
			tail->Data = node->Data;
			tail->Next = std::make_unique<Node>();
			tail = tail->Next.get();
			node = node->Next.get();
		}
	}

	template <typename T>
	constexpr BlockingQueue<T>& BlockingQueue<T>::operator=(BlockingQueue&& queue) noexcept
	{
		std::unique_ptr<Node> q = std::make_unique<Node>();
		{
			LockGuardRW lockHead(queue.headMutex);
			LockGuardRW lockTail(queue.tailMutex);
			queue.head.swap(q);
			queue.tail = queue.head.get();
		}

		LockGuardRW lockHead(headMutex);
		LockGuardRW lockTail(tailMutex);
		head = std::move(q);
		tail = head.get();
		return *this;
	}

	template <typename T>
	constexpr BlockingQueue<T>& BlockingQueue<T>::operator=(const BlockingQueue& queue) noexcept
	{
		tail = head.get();
		Node* node = queue.head.get();
		// Replace all current elements
		while (node != queue.tail && tail->Next != nullptr)
		{
			tail->Data = node->Data;
			tail = tail->Next.get();
			node = node->Next.get();
		}
		// When more elements to add then just create them
		while (node != queue.tail)
		{
			tail->Data = node->Data;
			tail->Next = std::make_unique<Node>();
			tail = tail->Next.get();
			node = node->Next.get();
		}
		// Ensures that when copying smaller queue all older elements get destroyed
		tail->Next = nullptr;
		return *this;
	}

	template <typename T>
	constexpr bool BlockingQueue<T>::IsEmpty() const noexcept
	{
		LockGuardRO lockHead(headMutex);
		LockGuardRO lockTail(tailMutex);
		return head.get() == tail;
	}

	template <typename T>
	constexpr U64 BlockingQueue<T>::Size() const noexcept
	{
		LockGuardRO lockHead(headMutex);
		LockGuardRO lockTail(tailMutex);

		U64 size = 0;
		Node* node = head.get();
		while (node != tail)
		{
			++size;
			node = node->Next;
		}
		return size;
	}

	template <typename T>
	constexpr void BlockingQueue<T>::Clear() noexcept
	{
		LockGuardRW lockHead(headMutex);
		LockGuardRW lockTail(tailMutex);

		head = std::make_unique<Node>();
		tail = head.get();
	}

	template <typename T>
	constexpr void BlockingQueue<T>::PushBack(T&& obj) noexcept
	{
		std::unique_ptr<Node> node = std::make_unique<Node>();
		Node* newTail = node.get();

		LockGuardRW lockTail(tailMutex);
		tail->Data = std::move(obj);
		tail->Next = std::move(node);
		tail = newTail;
	}

	template <typename T>
	constexpr void BlockingQueue<T>::PushBack(const T& obj) noexcept
	{
		std::unique_ptr<Node> node = std::make_unique<Node>();
		Node* newTail = node.get();

		LockGuardRW lockTail(tailMutex);
		tail->Data = obj;
		tail->Next = std::move(node);
		tail = newTail;
	}

	template <typename T> template <typename... Args>
	constexpr T& BlockingQueue<T>::EmplaceBack(Args&&... args) noexcept
	{
		std::unique_ptr<Node> node = std::make_unique<Node>();
		Node* newTail = node.get();

		LockGuardRW lockTail(tailMutex);
		tail->Data = T(std::forward<Args>(args)...);
		tail->Next = std::move(node);
		tail = newTail;
		return tail->Data;
	}

	template <typename T>
	constexpr void BlockingQueue<T>::PopFront() noexcept
	{
		LockGuardRW lockHead(headMutex);
		{
			LockGuardRO lockTail(tailMutex);
			ZE_ASSERT(head.get() != tail, "Empty head!");
		}

		std::unique_ptr<Node> oldHead = std::move(head);
		head = std::move(oldHead->Next);
	}

	template <typename T>
	constexpr bool BlockingQueue<T>::TryPopFront(T& obj) noexcept
	{
		LockGuardRW lockHead(headMutex);
		{
			LockGuardRO lockTail(tailMutex);
			if (head.get() == tail)
				return false;
		}
		obj = std::move(head->Data);
		std::unique_ptr<Node> oldHead = std::move(head);
		head = std::move(oldHead->Next);

		return true;
	}

	template <typename T>
	constexpr void BlockingQueue<T>::Swap(BlockingQueue& queue) noexcept
	{
		// Don't block 2 locks at the same time to avoid deadlocks
		std::unique_ptr<Node> q = std::make_unique<Node>();
		{
			LockGuardRW lockHead(headMutex);
			LockGuardRW lockTail(tailMutex);
			head.swap(q);
			tail = head.get();
		}
		{
			LockGuardRW lockHead(queue.headMutex);
			LockGuardRW lockTail(queue.tailMutex);
			queue.head.swap(q);
			queue.tail = queue.head.get();
		}
		{
			LockGuardRW lockHead(headMutex);
			LockGuardRW lockTail(tailMutex);
			head.swap(q);
			tail = head.get();
		}
	}
#pragma endregion
}