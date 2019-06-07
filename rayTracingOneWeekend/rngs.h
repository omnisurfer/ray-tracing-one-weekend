#pragma once

#include <random>

//setup RNG
//https://stackoverflow.com/questions/9878965/rand-between-0-and-1
//https://www.guyrutenberg.com/2014/05/03/c-mt19937-example/

std::mt19937_64 randomNumberGenerator;
uint64_t timeSeed;
std::uniform_real_distribution<double> unifRand(0.0, 1.0);