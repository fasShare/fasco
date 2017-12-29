#include <stdio.h>
#include <iostream>

#include "McoRoutine.h"
#include "Mutex.h"
#include "MutexLocker.h"
#include "Thread.h"

using namespace std;
using namespace moxie;

McoRoutine *co = nullptr;

Mutex mutex;

void funcThread() {
	std::cout << "Begin funcThread" << std::endl;
	mutex.lock();
    McoResume(co);
	std::cout << "End funcThread" << std::endl;
	McoYield(co);
}

void func() {
    std::cout << "func" << std::endl;
	McoYield(co);
    std::cout << "mid func" << std::endl;
    McoResume(co);
	std::cout << "end func" << std::endl;
}


int main() {
    cout << "begin main" << std::endl;
    co = McoCreate(func);
	mutex.lock();
    
	McoResume(co);

	Thread thread(funcThread);

	thread.start();

	sleep(1);
	mutex.unlock();
	thread.join();
    cout << "end main" << std::endl;
}
