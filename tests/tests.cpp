#define DRGNT_IMPLEMENTATION
#define DRGNT_PUB_API static
#define DRGNT_IMPL_API static
#include "dragonito.h"
#include "test_bank.h"

#include <cstdio>
#include <iostream>
#include <charconv>
#include <thread>
#include <vector>
#include <cassert>

#include <ryu/f2s.c>

template<typename Func>
bool process_range_threaded(uint64_t minValue, uint64_t maxValue, Func&& func)
{
#if NDEBUG
	uint64_t threadCount = std::thread::hardware_concurrency();
#else
	uint64_t threadCount = 1;
#endif

	uint64_t count = maxValue - minValue;

	std::atomic_bool succeeded(true);
	std::atomic_uint64_t total_progress(0);

	auto threadLogic = [&](int index)
	{
		for (uint64_t last_prog = 0; true;)
		{
			uint64_t prog = total_progress++;

			if (prog >= count) break;
			if (prog < last_prog) break;
			last_prog = prog;

			if (!func(prog)) succeeded = false;

			if (prog % 2000000 == 0) printf("Progress %llu (%.1f%%)\n", prog, 100.0f * (double)prog / (double)count);
		}
	};

	std::vector<std::unique_ptr<std::thread>> threads;

	for (int i = 0; i < threadCount; i++)
		threads.push_back(std::make_unique<std::thread>(threadLogic, i));

	for (int i = 0; i < threadCount; i++)
		threads[i]->join();

	return succeeded;
}

template<typename Func>
bool process_single_value(uint64_t value, Func&& func)
{
	return func(value);
}

static bool compare_exponents(const char* buffer0, const char* buffer1, drgnt_u32& n0, drgnt_u32& n1, bool trim_buffer1 = false)
{
	drgnt_u32 l0 = n0;
	drgnt_u32 l1 = n1;

	while (l0 > 0 && buffer0[l0 -= 1] != 'e');
	while (l1 > 0 && buffer1[l1 -= 1] != 'e');

	if (l0 <= 0 || l1 <= 0) throw new std::exception();

	drgnt_i32 t0 = l0 + 1;
	drgnt_i32 t1 = l1 + 1;

	if (buffer0[t0] == '+') t0++;
	if (buffer1[t1] == '+') t1++;

	drgnt_i32 e0 = 0;
	drgnt_i32 e1 = 0;

	if ((int)std::from_chars(buffer0 + t0, buffer0 + n0, e0).ec != 0) throw new std::exception();
	if ((int)std::from_chars(buffer1 + t1, buffer1 + n1, e1).ec != 0) throw new std::exception();

	while (trim_buffer1 && l1 > 2 && buffer1[l1 - 1] == '0' && buffer1[l1 - 2] != '.') l1--;

	n0 = l0;
	n1 = l1;

	return e0 == e1;
}

static bool test_f32_to_str()
{
	int index = 0;
	int test_index = -1;
	int test_size = sizeof(f32_test_suite) / sizeof(f32_test_suite[0]);

	bool success = true;

	char buffer[1024];

	for (int i = 0; i < test_size; i++)
	{
		if (test_index == i) __debugbreak();
		else if (test_index >= 0) continue;

		const auto& test = f32_test_suite[i];
		const float valf = *(float*)&test.value;
		const auto format = test.exponent_form ? std::chars_format::scientific : std::chars_format::fixed;

		memset(buffer, 0, sizeof(buffer));

		drgnt_i32 maxlen = drgnt_estimate_len32(test.value, test.precision, test.exponent_form);
		drgnt_u32 length = drgnt_f32_to_str(buffer, test.value, test.precision, test.exponent_form);
		buffer[length] = 0;
		buffer[1023] = 0;

		drgnt_i32 written_length = 0;
		while (buffer[written_length] != 0) written_length++;

		uint32_t expected_length = strlen(test.expected);

		if (expected_length != length || memcmp(buffer, test.expected, expected_length) != 0)
		{
			printf("Test failed for (f32) value=%08x, prec=%d, exp=%d, index=%d:\n", test.value, test.precision, test.exponent_form ? 1 : 0, i);
			printf("    Expected: %s (%d chars)\n", test.expected, expected_length);
			printf("    Got     : %s (%d chars)\n", buffer, length);
			printf("\n");

			success &= false;
		}

		if (written_length > maxlen)
		{
			printf("More characters were written than the estimated length:\n");
			printf("    Estimated: %d\n", maxlen);
			printf("    Got      : %d\n", written_length);
			printf("\n");

			success &= false;
		}
	}

	return success;
}

static bool test_f64_to_str()
{
	int index = 0;
	int test_index = -1;
	int test_size = sizeof(f64_test_suite) / sizeof(f64_test_suite[0]);

	bool success = true;

	char* buffer = new char[2048];

	for (int i = 0; i < test_size; i++)
	{
		if (test_index == i) __debugbreak();
		else if (test_index >= 0) continue;

		const auto& test = f64_test_suite[i];
		const double valf = *(double*)&test.value;
		const auto format = test.exponent_form ? std::chars_format::scientific : std::chars_format::fixed;

		memset(buffer, 0, sizeof(buffer));

		drgnt_u32 length = drgnt_f64_to_str(buffer, test.value, test.precision, test.exponent_form);
		buffer[length] = 0;
		buffer[2047] = 0;

		uint32_t expected_length = strlen(test.expected);

		if (expected_length != length || memcmp(buffer, test.expected, expected_length) != 0)
		{
			printf("Test failed for (f64) value=%016llx, prec=%d, exp=%d, index=%d:\n", test.value, test.precision, test.exponent_form ? 1 : 0, i);
			printf("    Expected: %s (%d chars)\n", test.expected, expected_length);
			printf("    Got     : %s (%d chars)\n", buffer, length);
			printf("\n");

			success &= false;
		}
	}

	delete[] buffer;

	return success;
}

static bool compare_values(uint64_t val, bool exponent_form, int prec)
{
	const float valf = *(float*)&val;
	const auto format = exponent_form ? std::chars_format::scientific : std::chars_format::fixed;

	const int drgnt_prec = prec;
	const int charconv_prev = prec < 0 ? 900 : prec;

	//Skip infinity
	if (val == 0x7F800000)
		return true;

	char buffer0[1024];
	char buffer1[1024];

	drgnt_u32 n0 = drgnt_f32_to_str(buffer0, val, drgnt_prec, exponent_form);
	drgnt_u32 n1 = std::to_chars(buffer1, buffer1 + sizeof(buffer1), valf, format, charconv_prev).ptr - buffer1;

	buffer0[n0] = 0;
	buffer1[n1] = 0;

	drgnt_i32 maxlen = drgnt_estimate_len32(val, drgnt_prec, exponent_form);
	drgnt_i32 written_length = 0;
	while (buffer0[written_length] != 0) written_length++;

	bool wrong_e = exponent_form && !compare_exponents(buffer0, buffer1, n0, n1);

	if (prec < 0)
	{
		while (n1 > 0 && buffer1[n1 - 1] == '0') n1--;

		if (n1 > 0 && buffer1[n1 - 1] == '.' && buffer1[n1] == '0') n1++;
	}

	if (written_length > maxlen)
	{
		printf("Len Err  %llu (%d, %d)\n", val, prec, exponent_form ? 1 : 0);
		return false;
	}

	if (n1 != n0 || memcmp(buffer0, buffer1, n0) != 0 || wrong_e)
	{
		printf("Error    %llu (%d, %d)\n", val, prec, exponent_form ? 1 : 0);
		return false;
	}

	return true;
}

static bool check_values_srt(uint64_t val, bool compare_against_ryu)
{
	const float valf = *(float*)&val;

	//Skip infinity
	if (val == 0x7F800000 || val == 0)
		return true;

	bool has_error = false;

	if (compare_against_ryu) /* We want to compare against ryu (or teju) because charconv formats some values differently */
	{
		const uint32_t ieee_mantissa = val & ((1u << FLOAT_MANTISSA_BITS) - 1);
		const uint32_t ieee_exponent = (val >> FLOAT_MANTISSA_BITS) & ((1u << FLOAT_EXPONENT_BITS) - 1);
		const auto ryu_result = f2d(ieee_mantissa, ieee_exponent);

		drgnt_u32 m_unified;
		drgnt_i32 e_unified;
		drgnt_bool neg_sign;
		drgnt_i32 prep = drgnt_prepare_f32(val, &m_unified, &e_unified, &neg_sign);
		drgnt_srt_result32 drgnt_result = drgnt_shortest_f32(m_unified, e_unified);

		for (; drgnt_result.mantissa != 0; drgnt_result.exponent++)
		{
			if (drgnt_result.mantissa % 10 == 0)
				drgnt_result.mantissa /= 10;
			else break;
		}

		has_error = ryu_result.mantissa != drgnt_result.mantissa || ryu_result.exponent != drgnt_result.exponent;
	}
	else
	{
		char buffer0[1024];
		char buffer1[1024];

		memset(buffer0, 0, sizeof(buffer0));

		drgnt_u32 n1 = std::to_chars(buffer1, buffer1 + sizeof(buffer1), valf).ptr - buffer1;
		drgnt_bool n1_is_exp = strchr(buffer1, 'e') != NULL;
		drgnt_u32 n0 = drgnt_f32_to_str_shortest(buffer0, val, false, n1_is_exp, true);
		drgnt_i32 maxlen = drgnt_estimate_srt_len32(val, false, n1_is_exp);

		buffer0[n0] = 0;
		buffer1[n1] = 0;

		bool wrong_e = n1_is_exp && !compare_exponents(buffer0, buffer1, n0, n1);

		has_error = n1 != n0 || memcmp(buffer0, buffer1, n0) != 0 || wrong_e;

		//Check length
		drgnt_i32 written_length = 0;
		while (buffer0[written_length] != 0) written_length++;

		if (written_length > maxlen)
		{
			printf("Error    %llu (SRT, length)\n", val);
			return false;
		}
	}

	if (has_error)
	{
		printf("Error    %llu (SRT)\n", val);
		return false;
	}

	return true;
}

static bool test_unlimited_fix(uint64_t val) { return compare_values(val, false, -1); }
static bool test_unlimited_exp(uint64_t val) { return compare_values(val, true, -1); }

static bool test_limited_fix0(uint64_t val) { return compare_values(val, false, 0); }
static bool test_limited_fix1(uint64_t val) { return compare_values(val, false, 1); }
static bool test_limited_fix2(uint64_t val) { return compare_values(val, false, 2); }
static bool test_limited_fix3(uint64_t val) { return compare_values(val, false, 3); }
static bool test_limited_fix4(uint64_t val) { return compare_values(val, false, 4); }
static bool test_limited_fix8(uint64_t val) { return compare_values(val, false, 8); }
static bool test_limited_fix9(uint64_t val) { return compare_values(val, false, 9); }

static bool test_limited_exp0(uint64_t val) { return compare_values(val, true, 0); }
static bool test_limited_exp1(uint64_t val) { return compare_values(val, true, 1); }
static bool test_limited_exp2(uint64_t val) { return compare_values(val, true, 2); }
static bool test_limited_exp3(uint64_t val) { return compare_values(val, true, 3); }
static bool test_limited_exp4(uint64_t val) { return compare_values(val, true, 4); }
static bool test_limited_exp8(uint64_t val) { return compare_values(val, true, 8); }
static bool test_limited_exp9(uint64_t val) { return compare_values(val, true, 9); }

static bool test_shortest_round_trip(uint64_t val) { return check_values_srt(val, false); }

bool run_test_suites()
{
	bool success = true;

	if (!test_f32_to_str())
	{
		printf("Some float32 tests failed\n");
		success &= false;
	}

	if (!test_f64_to_str())
	{
		printf("Some float64 tests failed\n");
		success &= false;
	}

	if (success)
	{
		printf("Float tests succeeded\n");
	}

	return success;
}

bool run_exhaustive_tests()
{
//	process_single_value(953269932, test_shortest_round_trip);

	bool success = true;
	success &= process_range_threaded(0, 0x7f7fffff, test_shortest_round_trip);

	success &= process_range_threaded(0, 0x7f7fffff, test_unlimited_fix);
	success &= process_range_threaded(0, 0x7f7fffff, test_unlimited_exp);

	success &= process_range_threaded(0, 0x7f7fffff, test_limited_fix0);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_fix1);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_fix2);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_fix3);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_fix4);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_fix8);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_fix9);

	success &= process_range_threaded(0, 0x7f7fffff, test_limited_exp0);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_exp1);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_exp2);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_exp3);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_exp4);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_exp8);
	success &= process_range_threaded(0, 0x7f7fffff, test_limited_exp9);

	if (success) printf("All good!\n");
	else printf("Wrong!\n");

	return success;
}

static drgnt_u32 drgnt_to_decimal32(drgnt_u32 mantissa, drgnt_u32* coeffs)
{
	drgnt_u64 m64 = (drgnt_u64)mantissa;
	drgnt_u64 iprod = m64 * coeffs[0];
	drgnt_u64 fprod = (m64 * coeffs[1]) >> 30;

	if (m64 * coeffs[3] > coeffs[2] * (fprod + 1))
		fprod += 1;

	return iprod + fprod;
}

int main()
{
#if 1
	return run_test_suites();
#else
	return run_exhaustive_tests();
#endif
}