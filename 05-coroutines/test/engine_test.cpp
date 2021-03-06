#include "gtest/gtest.h"
#include <iostream>
#include <sstream>

#include <coroutine/engine.h>

void _calculator_add(int& result, int left, int right) {
    result = left + right;
}

TEST(CoroutineTest, SimpleStart) {
    Coroutine::Engine engine;

    int result;
    engine.start(_calculator_add, result, 1, 2);

    ASSERT_EQ(3, result);
}

void printa(Coroutine::Engine& pe, std::stringstream& out, void*& other) {
    out << "A1 ";
    pe.sched(other);

    out << "A2 ";
    pe.sched(other);

    out << "A3 ";
    pe.sched(other);
}

void printb(Coroutine::Engine& pe, std::stringstream& out, void*& other) {
    out << "B1 ";
    pe.sched(other);

    out << "B2 ";
    pe.sched(other);

    out << "B3 ";
}

std::stringstream out;
void *pa = nullptr, *pb = nullptr;
void _printer(Coroutine::Engine& pe, std::string& result) {
    // Create routines, note it doens't get control yet
    pa = pe.run(printa, pe, out, pb);
    pb = pe.run(printb, pe, out, pa);

    // Pass control to first routine, it will ping pong
    // between printa/printb greedely then we will get
    // contol back
    pe.sched(pa);
    out << "END";

    // done
    result = out.str();
}

TEST(CoroutineTest, Printer) {
    Coroutine::Engine engine;

    std::string result;
    engine.start(_printer, engine, result);
    ASSERT_STREQ("A1 B1 A2 B2 A3 B3 END", result.c_str());
}
