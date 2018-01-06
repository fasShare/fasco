#include <stdio.h>
#include <iostream>

#include "McoRoutine.h"

using namespace std;

McoRoutine *co = nullptr, *co2 = nullptr, *co3 = nullptr;

void func3() {
	std::cout << "func3" << std::endl;
    McoYield(co3);
	std::cout << "end func3" << std::endl;
}

void func2() {
    std::cout << "func2" << std::endl;
    McoYield(co2);
	std::cout << "end func2" << std::endl;
}

void func() {
    std::cout << "func" << std::endl;
	McoYield(co);
    std::cout << "end func" << std::endl;
}


int main() {
    cout << "begin main" << std::endl;
    co = McoCreate(func);
	co2 = McoCreate(func2);
	co3 = McoCreate(func3);
    cout << "before resume" << endl;
    McoResume(co);
    McoResume(co2);
    McoResume(co3);
    McoResume(co);
    cout << "end main" << std::endl;
}
