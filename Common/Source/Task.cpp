#include "Task.h"
#include "ThreadPool.h"

namespace ZE
{
	bool JobSteal::TryStealing(std::coroutine_handle<>& handle, PromiseBase*& promise) noexcept
	{
		if (pool != nullptr)
			return pool->GetNextJob(handle, promise);
		return false;
	}
}