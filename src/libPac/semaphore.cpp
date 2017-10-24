#include "semaphore.h"

lib_pac::semaphore::semaphore(uint32_t n)
	: count(n)
{
}

void lib_pac::semaphore::notify()
{
	std::unique_lock<std::mutex> l(mutex);
	++count;
	cond.notify_one();
}

void lib_pac::semaphore::wait()
{
	std::unique_lock<std::mutex> l(mutex);
	cond.wait(l, [this] {return count != 0; });
	--count;
}

lib_pac::critical_section::critical_section(semaphore& ss): sem{ss}
{
	sem.wait();
}

lib_pac::critical_section::~critical_section()
{
	sem.notify();
}
