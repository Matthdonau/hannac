#include "benchmark/benchmark.h"
#include "mad_lib_template.hpp"
#include <vector>
#include <algorithm>

static void BM_AddInt(benchmark::State &state)
{
  std::vector<int> Test(state.range(0));
  std::generate(Test.begin(), Test.end(), std::rand);
  for (auto _ : state)
  {

    for (auto const &a : Test)
      (void)mad_lib_template::Add(a, a);
  }
}
BENCHMARK(BM_AddInt)->Range(8, 8 << 13)->Unit(benchmark::kMicrosecond);

static void BM_MultInt(benchmark::State &state)
{
  std::vector<int> Test(state.range(0));
  std::generate(Test.begin(), Test.end(), std::rand);
  for (auto _ : state)
  {

    for (auto const &a : Test)
      (void)mad_lib_template::Mult(a, a);
  }
}
BENCHMARK(BM_MultInt)->Range(8, 8 << 13)->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();