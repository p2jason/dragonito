#ifndef DRAGONITO_IMPL_H
#define DRAGONITO_IMPL_H

#include "dragonito.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "dragonito_constants.h"

#define DRGNT_1E9  1000000000ul
#define DRGNT_1E19 10000000000000000000ull

#ifdef _MSC_VER
#include <intrin.h>

static drgnt_u32 drgnt_bsr32(drgnt_u32 x) { unsigned long idx; _BitScanReverse(&idx, x); return idx; }
static drgnt_u32 drgnt_bsr64(drgnt_u64 x) { unsigned long idx; _BitScanReverse64(&idx, x); return idx; }

static drgnt_u32 drgnt_bsf32(drgnt_u32 x) { unsigned long idx; _BitScanForward(&idx, x); return idx; }
static drgnt_u32 drgnt_bsf64(drgnt_u64 x) { unsigned long idx; _BitScanForward64(&idx, x); return idx; }

#else

static drgnt_u32 drgnt_bsr32(drgnt_u32 x) { return 31 - __builtin_clz(x); }
static drgnt_u32 drgnt_bsr64(drgnt_u64 x) { return 31 - __builtin_clzll(x); }

static drgnt_u32 drgnt_bsf32(drgnt_u32 x) { return __builtin_ctz(x); }
static drgnt_u32 drgnt_bsf64(drgnt_u64 x) { return __builtin_ctzll(x); }

#endif

static void drgnt_mul64(drgnt_u64 a, drgnt_u64 b, drgnt_u64* hi, drgnt_u64* lo)
{
#if defined(_MSC_VER) && defined(_M_X64)
	*lo = _umul128(a, b, hi);
#elif defined(_MSC_VER) && defined(_M_ARM64)
	*lo = a * b; *hi = __umulh(a, b);
#else
	unsigned __int128 p = (unsigned __int128)a * (unsigned __int128)b;

	*lo = (drgnt_u64)p;
	*hi = (drgnt_u64)(p >> 64);
#endif
}

static drgnt_u64 drgnt_mul64_hi(drgnt_u64 a, drgnt_u64 b)
{
#if defined(_MSC_VER)
	return __umulh(a, b);
#else
	return (drgnt_u64)(((unsigned __int128)a * (unsigned __int128)b) >> 64);
#endif
}

static void drgnt_add128(drgnt_u64 bhi, drgnt_u64 blo, drgnt_u64* ahi, drgnt_u64* alo)
{
#if defined(_MSC_VER) && defined(_M_X64)
	_addcarry_u64(_addcarry_u64(0, *alo, blo, alo), *ahi, bhi, ahi);
#elif defined(_MSC_VER) && defined(_M_ARM64)
	*ahi += bhi + (blo > 0xFFFFFFFFFFFFFFFF - *alo);
	*alo += blo;
#else
	unsigned __int128 b = ((unsigned __int128)bhi << 64) | (unsigned __int128)blo;
	unsigned __int128 a = ((unsigned __int128)*ahi << 64) | (unsigned __int128)*alo;
	unsigned __int128 s = a + b;

	*alo = (drgnt_u64)s;
	*ahi = (drgnt_u64)(s >> 64);
#endif
}

/*
  Computes (a * b) % 1e19.

  It is based on: Improved Division by Invariant Integers (Moller and Granlund)
  The code adapted from: http://www.cecm.sfu.ca/CAG/code/TangentGraeffe/int128g (look at mulrec64)
*/
static drgnt_u64 drgnt_mul64_mod1e19(drgnt_u64 a, drgnt_u64 b)
{
	//These constants were generated from `./mod_gen.py`
	const drgnt_u64 v = 15581492618384294730ull;
	const drgnt_u64 d = 10000000000000000000ull;

	drgnt_u64 p_hi;
	drgnt_u64 p_lo;
	drgnt_mul64(a, b, &p_hi, &p_lo);

	drgnt_u64 q_hi;
	drgnt_u64 q_lo;
	drgnt_mul64(p_hi, v, &q_hi, &q_lo);

	drgnt_add128(p_hi + 1, p_lo, &q_hi, &q_lo);

	drgnt_u64 rem = p_lo - q_hi * d;

	if (rem > q_lo) rem += d;
	if (rem >= d)   rem -= d;

	return rem;
}

static drgnt_u64 drgnt_to_decimal64(drgnt_u64 mantissa, drgnt_i64 idx, drgnt_bool is_fpart)
{
	const drgnt_u64* coeffs = is_fpart ? DRGNT_FPART_COEFF_TABLE64[idx] : DRGNT_IPART_COEFF_TABLE64[idx];

	drgnt_u64 iprod = drgnt_mul64_mod1e19(mantissa, coeffs[0]);
	drgnt_u64 fprod = drgnt_mul64_hi(mantissa, coeffs[1]);

	drgnt_u64 coeff3_hi;
	drgnt_u64 coeff3_lo;
	drgnt_mul64(mantissa, coeffs[3], &coeff3_hi, &coeff3_lo);

	drgnt_u64 coeff2_hi;
	drgnt_u64 coeff2_lo;
	drgnt_mul64(fprod + 1, coeffs[2], &coeff2_hi, &coeff2_lo);

	drgnt_u64 total = iprod + fprod;

	if (coeff3_hi > coeff2_hi || (coeff3_hi == coeff2_hi && coeff3_lo > coeff2_lo))
		total += 1;

	if (total  > DRGNT_1E19)
		total -= DRGNT_1E19;

	return total;
}

static drgnt_u32 drgnt_to_decimal32(drgnt_u32 mantissa, drgnt_i32 idx, drgnt_bool is_fpart)
{
	const drgnt_u32* coeffs = is_fpart ? DRGNT_FPART_COEFF_TABLE32[idx] : DRGNT_IPART_COEFF_TABLE32[idx];

	drgnt_u64 m64 = (drgnt_u64)mantissa;
	drgnt_u64 iprod = m64 * coeffs[0];
	drgnt_u64 fprod = (m64 * coeffs[1]) >> 32;

	if (m64 * coeffs[3] > coeffs[2] * (fprod + 1))
		fprod += 1;

	return (drgnt_u32)((iprod + fprod) % DRGNT_1E9);
}

static drgnt_u32 drgnt_to_decimal_srt32(drgnt_u32 mantissa, const drgnt_u32* coeffs)
{
	drgnt_u64 m64 = (drgnt_u64)mantissa;

	drgnt_u64 iprod = (m64 * coeffs[0]);
	drgnt_u64 fprod = (m64 * coeffs[1]) >> 32;

	if (m64 * coeffs[3] > coeffs[2] * (fprod + 1))
		fprod += 1;

	return (drgnt_u32)(iprod + fprod);
}

static drgnt_u64 drgnt_to_decimal_srt64(drgnt_u64 mantissa, const drgnt_u64* coeffs)
{
	drgnt_u64 m64 = (drgnt_u64)mantissa;

	drgnt_u64 iprod = m64 * coeffs[0];
	drgnt_u64 fprod = drgnt_mul64_hi(mantissa, coeffs[1]);

	drgnt_u64 coeff3_hi;
	drgnt_u64 coeff3_lo;
	drgnt_mul64(mantissa, coeffs[3], &coeff3_hi, &coeff3_lo);

	drgnt_u64 coeff2_hi;
	drgnt_u64 coeff2_lo;
	drgnt_mul64(fprod + 1, coeffs[2], &coeff2_hi, &coeff2_lo);

	drgnt_u64 total = iprod + fprod;

	if (coeff3_hi > coeff2_hi || (coeff3_hi == coeff2_hi && coeff3_lo > coeff2_lo))
		total += 1;

	return iprod + fprod;
}

static drgnt_bool drgnt_is_pow5_multiple32(drgnt_u32 m, drgnt_i32 p)
{
	//TODO: Could make this faster. Use the minverse algorithm (https://accu.org/journals/overload/27/154/neri_2722/)
	static const drgnt_u32 DRGNT_POWERS_OF_5[] = { 1ull, 5ull, 25ull, 125ull, 625ull, 3125ull, 15625ull, 78125ull, 390625ull, 1953125ull, 9765625ull, 48828125ull, 244140625ull, 1220703125ull };
	static const drgnt_i32 DRGNT_POW5_COUNT = sizeof(DRGNT_POWERS_OF_5) / sizeof(DRGNT_POWERS_OF_5[0]);

	return (p >= 0 && p < DRGNT_POW5_COUNT) && (m % DRGNT_POWERS_OF_5[p] == 0);
}

static drgnt_bool drgnt_is_pow5_multiple64(drgnt_u64 m, drgnt_i64 p)
{
	static const drgnt_u64 DRGNT_POWERS_OF_5[] =
	{
		1ull, 5ull, 25ull, 125ull, 625ull, 3125ull, 15625ull, 78125ull, 390625ull, 1953125ull, 9765625ull, 48828125ull, 244140625ull,
		1220703125ull, 6103515625ull, 30517578125ull, 152587890625ull, 762939453125ull, 3814697265625ull, 19073486328125ull, 95367431640625ull,
		476837158203125ull, 2384185791015625ull, 11920928955078125ull, 59604644775390625ull, 298023223876953125ull, 1490116119384765625ull,
		7450580596923828125ull
	};
	static const drgnt_i32 DRGNT_POW5_COUNT = sizeof(DRGNT_POWERS_OF_5) / sizeof(DRGNT_POWERS_OF_5[0]);

	return (p >= 0 && p < DRGNT_POW5_COUNT) && (m % DRGNT_POWERS_OF_5[p] == 0);
}

static drgnt_i32 drgnt_log10_pow2(drgnt_i32 e)
{
	return (1292913987ull * e) >> 32;
}

static drgnt_char* drgnt_trim_leading_zeros(drgnt_char* dst, const drgnt_char* src, const drgnt_char* src_end)
{
	while (src[0] == '0') src++;
	while (src < src_end) *dst++ = *src++;
	return dst;
}

static drgnt_char* drgnt_print_int9(drgnt_char* write_ptr, drgnt_u32 value)
{
	static const char DIGIT_TABLE[] =
	{
		"0001020304050607080910111213141516171819"
		"2021222324252627282930313233343536373839"
		"4041424344454647484950515253545556575859"
		"6061626364656667686970717273747576777879"
		"8081828384858687888990919293949596979899"
	};

	//Write the digits to the output buffer
	if (value != 0)
	{
		/*
		 There's a bug in LLVM that causes a division followed by modulo to not get optimized:
		  https://bugs.llvm.org/show_bug.cgi?id=38217

		 This is why all the modulo calculations are done manually (i.e. remainder = quotient - divisor * dividend)
		*/
		drgnt_u32 x0 = value / 10000;

		drgnt_u32 digit0123 = value - 10000 * x0;  /* = value % 10000           */
		drgnt_u32 digit8 = x0 / 10000;             /* = (value / 10000) / 10000 */
		drgnt_u32 digit4567 = x0 - 10000 * digit8; /* = (value / 10000) % 10000 */

		drgnt_u32 digit01 = digit0123 % 100;
		drgnt_u32 digit23 = digit0123 / 100;
		drgnt_u32 digit45 = digit4567 % 100;
		drgnt_u32 digit67 = digit4567 / 100;

		const char* c01 = &DIGIT_TABLE[digit01 * 2];
		const char* c23 = &DIGIT_TABLE[digit23 * 2];
		const char* c45 = &DIGIT_TABLE[digit45 * 2];
		const char* c67 = &DIGIT_TABLE[digit67 * 2];
		const char* c8  = &DIGIT_TABLE[digit8  * 2];

		write_ptr[0] = c8 [1];
		write_ptr[1] = c67[0];
		write_ptr[2] = c67[1];
		write_ptr[3] = c45[0];
		write_ptr[4] = c45[1];
		write_ptr[5] = c23[0];
		write_ptr[6] = c23[1];
		write_ptr[7] = c01[0];
		write_ptr[8] = c01[1];
	}
	else
	{
		for (int i = 0; i < 9; i++)
			write_ptr[i] = '0';
	}

	return write_ptr + 9;
}

static drgnt_char* drgnt_print_int19(drgnt_char* write_ptr, drgnt_u64 value)
{
	static const char DIGIT_TABLE[] =
	{
		"000001002003004005006007008009010011012013014015016017018019020021022023024025026027028029030031032033034035036037038039040041042043044045046047048049050051052053054055056057058059060061062063064065066067068069070071072073074075076077078079080081082083084085086087088089090091092093094095096097098099"
		"100101102103104105106107108109110111112113114115116117118119120121122123124125126127128129130131132133134135136137138139140141142143144145146147148149150151152153154155156157158159160161162163164165166167168169170171172173174175176177178179180181182183184185186187188189190191192193194195196197198199"
		"200201202203204205206207208209210211212213214215216217218219220221222223224225226227228229230231232233234235236237238239240241242243244245246247248249250251252253254255256257258259260261262263264265266267268269270271272273274275276277278279280281282283284285286287288289290291292293294295296297298299"
		"300301302303304305306307308309310311312313314315316317318319320321322323324325326327328329330331332333334335336337338339340341342343344345346347348349350351352353354355356357358359360361362363364365366367368369370371372373374375376377378379380381382383384385386387388389390391392393394395396397398399"
		"400401402403404405406407408409410411412413414415416417418419420421422423424425426427428429430431432433434435436437438439440441442443444445446447448449450451452453454455456457458459460461462463464465466467468469470471472473474475476477478479480481482483484485486487488489490491492493494495496497498499"
		"500501502503504505506507508509510511512513514515516517518519520521522523524525526527528529530531532533534535536537538539540541542543544545546547548549550551552553554555556557558559560561562563564565566567568569570571572573574575576577578579580581582583584585586587588589590591592593594595596597598599"
		"600601602603604605606607608609610611612613614615616617618619620621622623624625626627628629630631632633634635636637638639640641642643644645646647648649650651652653654655656657658659660661662663664665666667668669670671672673674675676677678679680681682683684685686687688689690691692693694695696697698699"
		"700701702703704705706707708709710711712713714715716717718719720721722723724725726727728729730731732733734735736737738739740741742743744745746747748749750751752753754755756757758759760761762763764765766767768769770771772773774775776777778779780781782783784785786787788789790791792793794795796797798799"
		"800801802803804805806807808809810811812813814815816817818819820821822823824825826827828829830831832833834835836837838839840841842843844845846847848849850851852853854855856857858859860861862863864865866867868869870871872873874875876877878879880881882883884885886887888889890891892893894895896897898899"
		"900901902903904905906907908909910911912913914915916917918919920921922923924925926927928929930931932933934935936937938939940941942943944945946947948949950951952953954955956957958959960961962963964965966967968969970971972973974975976977978979980981982983984985986987988989990991992993994995996997998999"
	};

	if (value != 0)
	{
		drgnt_u64 x0 = value / 1000000;
		drgnt_u64 x1 = x0    / 1000000;
		drgnt_u64 x2 = x1    / 1000000;

		drgnt_u64 digit012345 = value - 1000000 * x0; /* = (value) % 1000000 */
		drgnt_u64 digit6789AB = x0    - 1000000 * x1; /* = (value / 1000000) % 1000000 */
		drgnt_u64 digitCDEFGH = x1    - 1000000 * x2; /* = (value / 1000000 / 1000000) % 1000000 */
		drgnt_u64 digitI      = x2;

		drgnt_u64 digit012 = digit012345 % 1000;
		drgnt_u64 digit345 = digit012345 / 1000;
		drgnt_u64 digit678 = digit6789AB % 1000;
		drgnt_u64 digit9AB = digit6789AB / 1000;
		drgnt_u64 digitCDE = digitCDEFGH % 1000;
		drgnt_u64 digitFGH = digitCDEFGH / 1000;

		const char* c012 = &DIGIT_TABLE[digit012 * 3];
		const char* c345 = &DIGIT_TABLE[digit345 * 3];
		const char* c678 = &DIGIT_TABLE[digit678 * 3];
		const char* c9AB = &DIGIT_TABLE[digit9AB * 3];
		const char* cCDE = &DIGIT_TABLE[digitCDE * 3];
		const char* cFGH = &DIGIT_TABLE[digitFGH * 3];
		const char* cI   = &DIGIT_TABLE[digitI   * 3];

		write_ptr[0]  = cI  [2];
		write_ptr[1]  = cFGH[0];
		write_ptr[2]  = cFGH[1];
		write_ptr[3]  = cFGH[2];
		write_ptr[4]  = cCDE[0];
		write_ptr[5]  = cCDE[1];
		write_ptr[6]  = cCDE[2];
		write_ptr[7]  = c9AB[0];
		write_ptr[8]  = c9AB[1];
		write_ptr[9]  = c9AB[2];
		write_ptr[10] = c678[0];
		write_ptr[11] = c678[1];
		write_ptr[12] = c678[2];
		write_ptr[13] = c345[0];
		write_ptr[14] = c345[1];
		write_ptr[15] = c345[2];
		write_ptr[16] = c012[0];
		write_ptr[17] = c012[1];
		write_ptr[18] = c012[2];
	}
	else
	{
		for (int i = 0; i < 19; i++)
			write_ptr[i] = '0';
	}

	return write_ptr + 19;
}

//tlz = trim leading zeros
static drgnt_char* drgnt_print_int9_tlz(drgnt_char* write_start, drgnt_u32 value)
{
	drgnt_char temp[9];
	drgnt_print_int9(temp, value);

	return drgnt_trim_leading_zeros(write_start, temp, temp + 9);
}

static drgnt_char* drgnt_print_int19_tlz(drgnt_char* write_start, drgnt_u64 value)
{
	drgnt_char temp[19];
	drgnt_print_int19(temp, value);

	return drgnt_trim_leading_zeros(write_start, temp, temp + 19);
}

static drgnt_char* drgnt_print_exponent(drgnt_char* write_start, drgnt_i32 value)
{
	*write_start++ = 'e';
	*write_start++ = value < 0 ? '-' : '+';

	value = value < 0 ? -value : value;

	if (value == 0)
	{
		*write_start++ = '0';

		return write_start;
	}
	else
	{
		//TODO: Make faster
		return drgnt_print_int9_tlz(write_start, value);
	}
}

static int drgnt_print_posinf(drgnt_char* str)
{
	str[0] = '+';
	str[1] = 'i';
	str[2] = 'n';
	str[3] = 'f';

	return 4;
}

static int drgnt_print_neginf(drgnt_char* str)
{
	str[0] = '-';
	str[1] = 'i';
	str[2] = 'n';
	str[3] = 'f';

	return 4;
}

static int drgnt_print_nan(drgnt_char* str)
{
	str[0] = 'N';
	str[1] = 'a';
	str[2] = 'N';

	return 3;
}

static int drgnt_print_zeros(drgnt_char* str, drgnt_bool sign, drgnt_i32 precision, drgnt_bool exp_form)
{
	drgnt_i32 i = 0;

	if (sign)
	{
		str[i++] = '-';
	}

	if (precision < 0)
	{
		str[i++] = '0';
		str[i++] = '.';
		str[i++] = '0';
	}
	else if (precision == 0)
	{
		str[i++] = '0';
	}
	else
	{
		str[i++] = '0';
		str[i++] = '.';

		for (; precision > 0; precision--)
			str[i++] = '0';
	}

	if (exp_form)
	{
		str[i++] = 'e';
		str[i++] = '+';
		str[i++] = '0';
	}

	return i;
}

#define DRGNT_TEMPLATE_64 0 
#include "dragonito_template.h"
#undef DRGNT_TEMPLATE_64

#define DRGNT_TEMPLATE_64 1 
#include "dragonito_template.h"
#undef DRGNT_TEMPLATE_64

///////////////////////////////////////////////////////////////////////////////////////////////////
// Public API
///////////////////////////////////////////////////////////////////////////////////////////////////

DRGNT_IMPL_API drgnt_u32 drgnt_f32_to_bits(float value)
{
	drgnt_u32 bits;

	((drgnt_u8*)&bits)[0] = ((drgnt_u8*)&value)[0];
	((drgnt_u8*)&bits)[1] = ((drgnt_u8*)&value)[1];
	((drgnt_u8*)&bits)[2] = ((drgnt_u8*)&value)[2];
	((drgnt_u8*)&bits)[3] = ((drgnt_u8*)&value)[3];

	return bits;
}

DRGNT_IMPL_API drgnt_u64 drgnt_f64_to_bits(double value)
{
	drgnt_u64 bits;

	((drgnt_u8*)&bits)[0] = ((drgnt_u8*)&value)[0];
	((drgnt_u8*)&bits)[1] = ((drgnt_u8*)&value)[1];
	((drgnt_u8*)&bits)[2] = ((drgnt_u8*)&value)[2];
	((drgnt_u8*)&bits)[3] = ((drgnt_u8*)&value)[3];
	((drgnt_u8*)&bits)[4] = ((drgnt_u8*)&value)[4];
	((drgnt_u8*)&bits)[5] = ((drgnt_u8*)&value)[5];
	((drgnt_u8*)&bits)[6] = ((drgnt_u8*)&value)[6];
	((drgnt_u8*)&bits)[7] = ((drgnt_u8*)&value)[7];

	return bits;
}

DRGNT_IMPL_API drgnt_i32 drgnt_f32_to_default(drgnt_char* str, float value)
{
	return str ? drgnt_f32_to_srt(str, drgnt_f32_to_bits(value), 1) : drgnt_estimate_srt_len32(drgnt_f32_to_bits(value), 1, 0);
}

DRGNT_IMPL_API drgnt_i32 drgnt_f64_to_default(drgnt_char* str, double value)
{
	return str ? drgnt_f64_to_srt(str, drgnt_f64_to_bits(value), 1) : drgnt_estimate_srt_len64(drgnt_f64_to_bits(value), 1, 0);
}

DRGNT_IMPL_API drgnt_i32 drgnt_f32_to_fix(drgnt_char* str, drgnt_u32 bits, drgnt_i32 precision)
{
	return drgnt_f32_to_str(str, bits, precision, 0);
}

DRGNT_IMPL_API drgnt_i32 drgnt_f32_to_exp(drgnt_char* str, drgnt_u32 bits, drgnt_i32 precision)
{
	return drgnt_f32_to_str(str, bits, precision, 1);
}

DRGNT_IMPL_API drgnt_i32 drgnt_f32_to_srt(drgnt_char* str, drgnt_u32 bits, drgnt_bool accurate_ipart_fix)
{
	return drgnt_f32_to_str_shortest(str, bits, 1, 0, accurate_ipart_fix);
}

DRGNT_IMPL_API drgnt_i32 drgnt_f32_to_gen(drgnt_char* str, drgnt_u32 bits, drgnt_bool is_exp, drgnt_bool accurate_ipart_fix)
{
	return drgnt_f32_to_str_shortest(str, bits, 0, is_exp, accurate_ipart_fix);
}

DRGNT_IMPL_API drgnt_i32 drgnt_f64_to_fix(drgnt_char* str, drgnt_u64 bits, drgnt_i32 precision)
{
	return drgnt_f64_to_str(str, bits, precision, 0);
}

DRGNT_IMPL_API drgnt_i32 drgnt_f64_to_exp(drgnt_char* str, drgnt_u64 bits, drgnt_i32 precision)
{
	return drgnt_f64_to_str(str, bits, precision, 1);
}

DRGNT_IMPL_API drgnt_i32 drgnt_f64_to_srt(drgnt_char* str, drgnt_u64 bits, drgnt_bool accurate_ipart_fix)
{
	return drgnt_f64_to_str_shortest(str, bits, 1, 0, accurate_ipart_fix);
}

DRGNT_IMPL_API drgnt_i32 drgnt_f64_to_gen(drgnt_char* str, drgnt_u64 bits, drgnt_bool is_exp, drgnt_bool accurate_ipart_fix)
{
	return drgnt_f64_to_str_shortest(str, bits, 0, is_exp, accurate_ipart_fix);
}

#ifdef __cplusplus
}
#endif 

#endif /* DRAGONITO_IMPL_H */