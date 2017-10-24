#pragma once
#include <mutex>
#include <condition_variable>

namespace lib_pac
{
	class semaphore
	{
		std::mutex mutex;
		std::condition_variable cond;
		uint32_t count;

	public:
		explicit semaphore(uint32_t max);

		void notify();
		void wait();
	};

	class critical_section {
		semaphore &sem;
	public:
		critical_section(semaphore& s);
		~critical_section();
	};
}
