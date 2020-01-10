#pragma once

#include "Runtime/Core/Public/Async/AsyncWork.h"

class FAsyncExecTask : public FNonAbandonableTask
{
private:
	friend class FAutoDeleteAsyncTask<FAsyncExecTask>;

private:
	TFunction<void()> Work;

public:
	// Constructor
	FAsyncExecTask(TFunction<void()> InWork) : Work(InWork) {}

	// Exec function
	void DoWork() { Work(); }

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncExecTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};