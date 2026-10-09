#pragma once
#include "BasicTypes.h"
#include <coroutine>
#include <memory>

namespace ZE
{
#pragma region Type traits
	// Forward declaration of Task<R> to be used in type trait
	template <typename R>
	class Task;

	// Type trait to check if a type is an instantiation of Task<R>
	template <typename T>
	struct is_task : std::false_type {};

	template <typename U>
	struct is_task<Task<U>> : std::true_type {};

	// Helper to check if a type is an instantiation of Task<T>
	template <typename T>
	inline constexpr bool is_task_v = is_task<T>::value;

	// Helper to ensure always resolving to Task<T>
	template <typename T>
	struct to_task { using type = Task<T>; };

	template <typename T>
	struct to_task<Task<T>> { using type = Task<T>; };

	// Helper returning Task<T> for any type T, including Task<T> itself
	template <typename T>
	using to_task_t = typename to_task<T>::type;
#pragma endregion

#pragma region Promise declaration
	// Base type for all promises that avoids information about return parameter
	struct PromiseBase
	{
		enum class ExecutionState : U8
		{
			NotStarted = 0,
			Running = 1,
			Completed = 2
		};

		// Awaiter that allows for waking up threads waiting for task completion and performing symmetric transfer
		struct FinalAwaiter
		{
			constexpr bool await_ready() const noexcept { return false; }
			constexpr void await_resume() const noexcept {}

			template <typename P>
			constexpr std::coroutine_handle<> await_suspend(std::coroutine_handle<P> h) noexcept;
		};

		PromiseBase() = default;
		ZE_CLASS_DEFAULT(PromiseBase);
		virtual ~PromiseBase() = default;

		constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
		constexpr void unhandled_exception() const noexcept { /*std::current_exception();*/ }
		constexpr FinalAwaiter final_suspend() const noexcept { return {}; }

		virtual void DecrementRef() noexcept = 0;
		// When succedes, you must execute coroutine yourself
		virtual bool TryClaimExecution() noexcept = 0;
	};

	// Base class for promise_type to handle return values
	template <typename R>
	struct PromiseReturnBase : public PromiseBase
	{
		// Use std::monostate for void, std::optional<R> for non-void
		using StorageType = std::conditional_t<std::is_void_v<R>, std::monostate, std::optional<R>>;

		StorageType Storage;

		PromiseReturnBase() = default;
		ZE_CLASS_DEFAULT(PromiseReturnBase);
		virtual ~PromiseReturnBase() = default;

		// Only to be called once
		constexpr R ExtractResult() noexcept { if constexpr (!std::is_void_v<R>) return std::move(*Storage); }
		template <typename U>
			requires std::is_convertible_v<U, R>
		constexpr void return_value(U&& val) noexcept { Storage.emplace(std::forward<U>(val)); }
	};

	// Specialization for void return type
	template <>
	struct PromiseReturnBase<void> : public PromiseBase
	{
		constexpr void ExtractResult() const noexcept {}
		constexpr void return_void() const noexcept {}
	};
#pragma endregion

	// Utility class that allows automatic work stealing during waiting for task to be completed
	class JobSteal final
	{
		static inline class ThreadPool* pool = nullptr;

	public:
		JobSteal() = delete;

		static constexpr void RegisterThreadPool(class ThreadPool* steal) noexcept { pool = steal; }
		static bool TryStealing(std::coroutine_handle<>& handle, PromiseBase*& promise) noexcept;
	};

	// Information about asynchronous task scheduled to separate thread
	template <typename R>
	class Task final
	{
		friend class ThreadPool;

	public:
		struct promise_type final : public PromiseReturnBase<R>
		{
			UA8 RefCount = 1;
			std::atomic<PromiseBase::ExecutionState> State = PromiseBase::ExecutionState::NotStarted;
			std::atomic<std::coroutine_handle<>> Awaiter = {};

			promise_type() = default;
			ZE_CLASS_DEFAULT(promise_type);
			virtual ~promise_type() = default;

			constexpr Task<R> get_return_object() noexcept { return Task<R>{ HandleType::from_promise(*this) }; }

			constexpr void DecrementRef() noexcept override;
			constexpr bool TryClaimExecution() noexcept override;
		};
		using HandleType = std::coroutine_handle<promise_type>;

	private:
		// Main awaiter that should just perform symmetric trasfer, allowing Task<R> to be awaited in other coroutines
		struct Awaiter
		{
			HandleType Handle;

			constexpr bool await_ready() const noexcept { return Handle.promise().State.load(std::memory_order_acquire) == PromiseBase::ExecutionState::Completed; }
			constexpr R await_resume() noexcept { return Handle.promise().ExtractResult(); }

			constexpr std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaitingHandle) noexcept;
		};

		HandleType handle = nullptr;

		constexpr HandleType GetHandle() const noexcept { return handle; }
		// Allow for thread pool to have reference to promise memory
		constexpr void SetThreadPoolOwnership() noexcept { if (handle) handle.promise().RefCount.fetch_add(1, std::memory_order_release); }

		constexpr void Release() noexcept;

	public:
		Task() = default;
		constexpr explicit Task(HandleType h) noexcept : handle(h) {}

		template <typename Func, typename... Args> // Avoid shadowing move constructor in ctor that enables lambda capture
			requires (!std::is_same_v<std::remove_cvref_t<Func>, Task<R>>) && std::invocable<std::decay_t<Func>, std::decay_t<Args>...>
		constexpr explicit Task(Func&& f, Args&&... args) noexcept;

		// Disable copies to prevent multiple handle destructions
		ZE_CLASS_NO_COPY(Task);
		constexpr Task(Task&& task) noexcept : handle(std::exchange(task.handle, nullptr)) {}
		constexpr Task& operator=(Task&& task) noexcept;
		~Task() { Release(); }

		constexpr auto operator co_await() noexcept { return Awaiter{ handle }; }

		// Start task execution ahead of time
		constexpr void Start() noexcept;

		// Waits for scheduled task completion before returting data if any and optionally performs other tasks
		constexpr R Get(bool allowThreadPoolHelp = true) noexcept;
	};

#pragma region Functions
	template <typename P>
	constexpr std::coroutine_handle<> PromiseBase::FinalAwaiter::await_suspend(std::coroutine_handle<P> h) noexcept
	{
		auto& promise = h.promise();

		// Load faster to avoid hazard
		std::coroutine_handle<> continuation = promise.Awaiter.load(std::memory_order_seq_cst);

		// Signal completion to any thread waiting in Get()
		promise.State.store(PromiseBase::ExecutionState::Completed, std::memory_order_seq_cst);
		promise.State.notify_all();

		// Resume awaiting coroutine (symmetric transfer)
		if (continuation)
			return continuation;
		return std::noop_coroutine();
	}

	template <typename R>
	constexpr void Task<R>::promise_type::DecrementRef() noexcept
	{
		if (RefCount.fetch_sub(1, std::memory_order_release) == 1)
		{
			std::atomic_thread_fence(std::memory_order_acquire);
			HandleType::from_promise(*this).destroy();
		}
	}

	template <typename R>
	constexpr bool Task<R>::promise_type::TryClaimExecution() noexcept
	{
		PromiseBase::ExecutionState expected = PromiseBase::ExecutionState::NotStarted;
		return State.compare_exchange_strong(expected, PromiseBase::ExecutionState::Running,
			std::memory_order_acquire, std::memory_order_relaxed);
	}

	template <typename R>
	constexpr std::coroutine_handle<> Task<R>::Awaiter::await_suspend(std::coroutine_handle<> awaitingHandle) noexcept
	{
		// Register for symmetric transfer
		Handle.promise().Awaiter.store(awaitingHandle, std::memory_order_seq_cst);

		// Check if task is not being already executed
		if (Handle.promise().TryClaimExecution())
			return Handle;

		// Double check if the worker thread hit FinalAwaiter while we assiging Awaiter
		if (Handle.promise().State.load(std::memory_order_seq_cst) == PromiseBase::ExecutionState::Completed)
			return awaitingHandle;

		return std::noop_coroutine();
	}

	template <typename R>
	constexpr void Task<R>::Release() noexcept
	{
		if (handle)
			handle.promise().DecrementRef();
		handle = nullptr;
	}

	template <typename R>
	template <typename Func, typename... Args>
		requires (!std::is_same_v<std::remove_cvref_t<Func>, Task<R>>) && std::invocable<std::decay_t<Func>, std::decay_t<Args>...>
	constexpr Task<R>::Task(Func&& f, Args&&... args) noexcept
	{
		using Ret = std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>;

		if constexpr (is_task_v<Ret>)
		{
			// If the task is already a coroutine, just invoke the parameters and return the task
			static_assert(std::is_same_v<Ret, Task<R>>,
				"Coroutine lambda return type must match Task<R>");

			Task created = std::invoke(std::forward<Func>(f), std::forward<Args>(args)...);
			handle = std::exchange(created.handle, nullptr);
		}
		else
		{
			// Generic lambda coroutine, simple fire and forget or return value task
			auto makeTask = [](std::decay_t<Func> func, std::decay_t<Args>... boundArgs) -> Task<R>
				{
					if constexpr (std::is_void_v<R>)
					{
						std::invoke(std::move(func), std::move(boundArgs)...);
						co_return;
					}
					else
					{
						co_return std::invoke(std::move(func), std::move(boundArgs)...);
					}
				};

			// Call wrapper to instantiate the coroutine frame and steal the handle
			Task created = makeTask(std::forward<Func>(f), std::forward<Args>(args)...);
			handle = std::exchange(created.handle, nullptr);
		}
	}

	template <typename R>
	constexpr Task<R>& Task<R>::operator=(Task&& task) noexcept
	{
		if (this != &task)
		{
			Release();
			handle = std::exchange(task.handle, nullptr);
		}
		return *this;
	}

	template <typename R>
	constexpr void Task<R>::Start() noexcept
	{
		if (handle && handle.promise().TryClaimExecution())
			handle.resume();
	}

	template <typename R>
	constexpr R Task<R>::Get(bool allowThreadPoolHelp) noexcept
	{
		if (!handle)
			return R();

		auto& promise = handle.promise();

		// Check if already started executing
		if (promise.TryClaimExecution())
			handle.resume();

		// If already executing, start polling for result
		PromiseBase::ExecutionState current = promise.State.load(std::memory_order_acquire);
		while (current != PromiseBase::ExecutionState::Completed)
		{
			if (allowThreadPoolHelp)
			{
				// Steal single thread pool job instead of just waiting
				std::coroutine_handle<> stolenHandle = nullptr;
				PromiseBase* stolenPromise = nullptr;
				if (JobSteal::TryStealing(stolenHandle, stolenPromise))
				{
					if (stolenPromise->TryClaimExecution())
						stolenHandle.resume();
					stolenPromise->DecrementRef();
				}
				else
					promise.State.wait(current, std::memory_order_relaxed);
			}
			else
				promise.State.wait(current, std::memory_order_relaxed);

			current = promise.State.load(std::memory_order_acquire);
		}
		// Make sure that handle is not usable anymore and free any memory left behind
		if constexpr (!std::is_void_v<R>)
		{
			R ret = promise.ExtractResult();
			Release();
			return ret;
		}
		else
			Release();
	}
#pragma endregion
}