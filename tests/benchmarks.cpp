#include <cstdint>
#include <random>
#include <charconv>

#include <benchmark/benchmark.h>

#define FMT_HEADER_ONLY 1
#define FMT_USE_FULL_CACHE_DRAGONBOX 1
#include <fmt/compile.h>
#include <fmt/format.h>

#define DRGNT_IMPLEMENTATION
#define DRGNT_PUB_API static
#define DRGNT_IMPL_API static
#include "dragonito.h"

template<typename Func>
static void BenchmarkFunc(benchmark::State& state, Func&& func)
{
	uint64_t value = state.range(0);
	int prec = state.range(1);

	benchmark::DoNotOptimize(value);

	for (auto _ : state)
		func(prec, value);
}

static float u32_to_f32(uint32_t i) { return *(float*)&i; }
static double u64_to_f64(uint64_t i) { return *(double*)&i; }

static void BM_Float32_Fix_Drgnt(benchmark::State& state)
{
	char buffer[1024];

	BenchmarkFunc(state, [&](int prec, uint64_t ival)
	{
		drgnt_f32_to_fix(buffer, ival, prec);
		benchmark::DoNotOptimize(buffer);
		benchmark::ClobberMemory();
	});
}

static void BM_Float64_Fix_Drgnt(benchmark::State& state)
{
	char buffer[2048];

	BenchmarkFunc(state, [&](int prec, uint64_t ival)
	{
		drgnt_f64_to_fix(buffer, ival, prec);
		benchmark::DoNotOptimize(buffer);
		benchmark::ClobberMemory();
	});
}

static void BM_Float32_Fix_Charconv(benchmark::State& state)
{
	char buffer[1024];

	BenchmarkFunc(state, [&](int prec, uint64_t ival)
	{
		std::to_chars(buffer, buffer + sizeof(buffer), u32_to_f32(ival), std::chars_format::fixed, prec);
		benchmark::DoNotOptimize(buffer);
		benchmark::ClobberMemory();
	});
}

static void BM_Float64_Fix_Charconv(benchmark::State& state)
{
	char buffer[2048];

	BenchmarkFunc(state, [&](int prec, uint64_t ival)
	{
		std::to_chars(buffer, buffer + sizeof(buffer), u64_to_f64(ival), std::chars_format::fixed, prec);
		benchmark::DoNotOptimize(buffer);
		benchmark::ClobberMemory();
	});
}

static void BM_Float32_Fix_Fmtlib(benchmark::State& state)
{
	char buffer[1024];

	BenchmarkFunc(state, [&](int prec, uint64_t ival)
	{
		fmt::v11::format_specs spec;
		spec.set_type(fmt::presentation_type::fixed);
		spec.precision = prec;

		fmt::detail::write_float<char>(buffer, u32_to_f32(ival), spec, fmt::detail::locale_ref());
		benchmark::DoNotOptimize(buffer);
		benchmark::ClobberMemory();
	});
}

static void BM_Float64_Fix_Fmtlib(benchmark::State& state)
{
	char buffer[2048];

	BenchmarkFunc(state, [&](int prec, uint64_t ival)
	{
		fmt::v11::format_specs spec;
		spec.set_type(fmt::presentation_type::fixed);
		spec.precision = prec;

		fmt::detail::write_float<char>(buffer, u64_to_f64(ival), spec, fmt::detail::locale_ref());
		benchmark::DoNotOptimize(buffer);
		benchmark::ClobberMemory();
	});
}

// 0x4450bb448efc7369 = 1234567891111123456789.0
// 0x3ff8000000000000 
// 0x3fbf9add37c1215e = 0.123456789123456789

BENCHMARK(BM_Float32_Fix_Drgnt)->Args({ 0x3fc00000, 0 })->Args({ 0x007FFFFF, 160 });
BENCHMARK(BM_Float64_Fix_Drgnt)->Args({ 0x28e7c08e74542cd9, 310 })->Args({ 0x000fffffffffffff, 1080 });
BENCHMARK(BM_Float32_Fix_Charconv)->Args({ 0x3fc00000, 0 })->Args({ 0x007FFFFF, 160 });
BENCHMARK(BM_Float64_Fix_Charconv)->Args({ 0x28e7c08e74542cd9, 310 })->Args({ 0x000fffffffffffff, 1080 });
BENCHMARK(BM_Float32_Fix_Fmtlib)->Args({ 0x3fc00000, 0 })->Args({ 0x007FFFFF, 160 });
BENCHMARK(BM_Float64_Fix_Fmtlib)->Args({ 0x28e7c08e74542cd9, 310 })->Args({ 0x000fffffffffffff, 1080 });

//sprintf(buffer, "%.17g", value);

BENCHMARK_MAIN();