#ifndef DRAGONITO_H
#define DRAGONITO_H

#ifdef __cplusplus
extern "C" {
#endif

//This define will be added to all function forward declarations
#ifndef DRGNT_PUB_API
#define DRGNT_PUB_API
#endif

//This define will be added to all function definitions
#ifndef DRGNT_IMPL_API
#define DRGNT_IMPL_API
#endif

//IMPORTANT: If you're going to specify this define, you will also need to declare the following types yourself
#ifndef DRGNT_NO_STDINT
#include <stdint.h>

typedef uint8_t  drgnt_u8;
typedef uint16_t drgnt_u16;
typedef uint32_t drgnt_u32;
typedef uint64_t drgnt_u64;

typedef int8_t  drgnt_i8;
typedef int16_t drgnt_i16;
typedef int32_t drgnt_i32;
typedef int64_t drgnt_i64;

typedef char  drgnt_char;

#ifdef __cplusplus
typedef bool  drgnt_bool;
#else
typedef _Bool drgnt_bool;
#endif
#endif

typedef struct drgnt_rowptr
{
	drgnt_i16 idx, seg_start, seg_end;
} drgnt_rowptr;

/*
 Extract the bits of a 32-bit float
*/
DRGNT_PUB_API drgnt_u32 drgnt_f32_to_bits(float value);

/*
 Extract the bits of a 64-bit float
*/
DRGNT_PUB_API drgnt_u64 drgnt_f64_to_bits(double value);

/*
 Print a 32-bit float. Pass NULL to `str` to get an upper bound for the length of the produced output. Otherwise, the number of characters written will be returned.
*/
DRGNT_PUB_API drgnt_i32 drgnt_f32_to_default(drgnt_char* str, float value);

/*
 Print a 64-bit float. Pass NULL to `str` to get an upper bound for the length of the produced output. Otherwise, the number of characters written will be returned.
*/
DRGNT_PUB_API drgnt_i32 drgnt_f64_to_default(drgnt_char* str, double value);

/*
 Print a 32-bit float using fixed formatting. 

 - Argument `str` is a pointer to the output buffer. The buffer MUST be large enough to hold the produced
  output. Use `drgnt_estimate_len32` to get an estimate of the length of the output produced for a given
  value.
 - Argument `bits` stores the bits of the IEEE-754 floating point value that should be printed. Use
  `drgnt_f32_to_bits` to extra the bits of a 32-bit float.
 - Argument `precision` determines how many digits after the decimal separator will be printed. Pass a
  negative value to indicate that all digits should be printed (without printing any extra zeros at the end)
*/
DRGNT_PUB_API drgnt_i32 drgnt_f32_to_fix(drgnt_char* str, drgnt_u32 bits, drgnt_i32 precision);

/*
 Print a 32-bit float using scientific (exponent) formatting. 

 - Argument `str` is a pointer to the output buffer. The buffer MUST be large enough to hold the produced
  output. Use `drgnt_estimate_len32` to get an estimate of the length of the output produced for a given
  value.
 - Argument `bits` stores the bits of the IEEE-754 floating point value that should be printed. Use
  `drgnt_f32_to_bits` to extra the bits of a 32-bit float.
 - Argument `precision` determines how many digits after the decimal separator will be printed. Pass a
  negative value to indicate that all digits should be printed (without printing any extra zeros at the end
*/
DRGNT_PUB_API drgnt_i32 drgnt_f32_to_exp(drgnt_char* str, drgnt_u32 bits, drgnt_i32 precision);

/*
 Print a 32-bit float using shortest-round-trip (SRT) formatting. This mode will automatically switch between
 fixed formatting and scientific formatting based on which would produce the shortest string.

 NOTE: Given a large enough number, its shortest-round-trip representation will have fewer digits than its
 fixed representation. In such cases, when printing using fixed formatting, extra trailing zeros will be printed
 to make up for the trimmed digits. For example, the SRT representation of the number `33554472` is `3.355447e+1`
 and, therefore, will be printed as `33554470`. This behavior can be misleading. When `accurate_ipart_fix` is set
 to true, these cases will be detected and the "full", correct value of the float will be printed instead of the SRT.

 - Argument `str` is a pointer to the output buffer. The buffer MUST be large enough to hold the produced
  output. Use `drgnt_estimate_srt_len32` to get an estimate of the length of the output produced for a given
  value.
 - Argument `bits` stores the bits of the IEEE-754 floating point value that should be printed. Use
  `drgnt_f32_to_bits` to extra the bits of a 32-bit float.
 - Argument `accurate_ipart_fix` determines if the exact floating point value should be printed when fixed formatting
  is used (for the integer part only). Read NOTE for further details.
*/
DRGNT_PUB_API drgnt_i32 drgnt_f32_to_srt(drgnt_char* str, drgnt_u32 bits, drgnt_bool accurate_ipart_fix);

/*
 Print a 32-bit float using general formatting. This mode will convert a float to its shortest-round-trip
 representation, but won't automatically switch between fixed and scientific formatting.

 - Argument `str` is a pointer to the output buffer. The buffer MUST be large enough to hold the produced
  output. Use `drgnt_estimate_srt_len32` to get an estimate of the length of the output produced for a given
  value.
 - Argument `bits` stores the bits of the IEEE-754 floating point value that should be printed. Use
  `drgnt_f32_to_bits` to extra the bits of a 32-bit float.
 - Argument `is_exp` is used to determine which formatting is used. Pass true to use scientific formatting and
  and pass false to use fixed formatting.
 - Argument `accurate_ipart_fix` is the same as in `drgnt_f32_to_srt`. Please refer to that function's documentation for details.
*/
DRGNT_PUB_API drgnt_i32 drgnt_f32_to_gen(drgnt_char* str, drgnt_u32 bits, drgnt_bool is_exp, drgnt_bool accurate_ipart_fix);



/*
 Print a 64-bit float using fixed formatting.

 - Argument `str` is a pointer to the output buffer. The buffer MUST be large enough to hold the produced
  output. Use `drgnt_estimate_len64` to get an estimate of the length of the output produced for a given
  value.
 - Argument `bits` stores the bits of the IEEE-754 floating point value that should be printed. Use
  `drgnt_f64_to_bits` to extra the bits of a 64-bit float.
 - Argument `precision` determines how many digits after the decimal separator will be printed. Pass a
  negative value to indicate that all digits should be printed (without printing any extra zeros at the end)
*/
DRGNT_PUB_API drgnt_i32 drgnt_f64_to_fix(drgnt_char* str, drgnt_u64 bits, drgnt_i32 precision);

/*
 Print a 64-bit float using scientific formatting.

 - Argument `str` is a pointer to the output buffer. The buffer MUST be large enough to hold the produced
  output. Use `drgnt_estimate_len64` to get an estimate of the length of the output produced for a given
  value.
 - Argument `bits` stores the bits of the IEEE-754 floating point value that should be printed. Use
  `drgnt_f64_to_bits` to extra the bits of a 64-bit float.
 - Argument `precision` determines how many digits after the decimal separator will be printed. Pass a
  negative value to indicate that all digits should be printed (without printing any extra zeros at the end)
*/
DRGNT_PUB_API drgnt_i32 drgnt_f64_to_exp(drgnt_char* str, drgnt_u64 bits, drgnt_i32 precision);

/*
 Print a 64-bit float using shortest-round-trip (SRT) formatting. This mode will automatically switch between
 fixed formatting and scientific formatting based on which would produce the shortest string.

 NOTE: Given a large enough number, its shortest-round-trip representation will have fewer digits than its
 fixed representation. In such cases, when printing using fixed formatting, extra trailing zeros will be printed
 to make up for the trimmed digits. For example, the SRT representation of the number `33554472` is `3.355447e+1`
 and, therefore, will be printed as `33554470`. This behavior can be misleading. When `accurate_ipart_fix` is set
 to true, these cases will be detected and the "full", correct value of the float will be printed instead of the SRT.

 - Argument `str` is a pointer to the output buffer. The buffer MUST be large enough to hold the produced
  output. Use `drgnt_estimate_srt_len64` to get an estimate of the length of the output produced for a given
  value.
 - Argument `bits` stores the bits of the IEEE-754 floating point value that should be printed. Use
  `drgnt_f64_to_bits` to extra the bits of a 64-bit float.
 - Argument `accurate_ipart_fix` determines if the exact floating point value should be printed when fixed formatting
  is used (for the integer part only). Read NOTE for further details.
*/
DRGNT_PUB_API drgnt_i32 drgnt_f64_to_srt(drgnt_char* str, drgnt_u64 bits, drgnt_bool accurate_ipart_fix);

/*
 Print a 64-bit float using general formatting. This mode will convert a float to its shortest-round-trip
 representation, but won't automatically switch between fixed and scientific formatting.

 - Argument `str` is a pointer to the output buffer. The buffer MUST be large enough to hold the produced
  output. Use `drgnt_estimate_srt_len64` to get an estimate of the length of the output produced for a given
  value.
 - Argument `bits` stores the bits of the IEEE-754 floating point value that should be printed. Use
  `drgnt_f64_to_bits` to extra the bits of a 64-bit float.
 - Argument `is_exp` is used to determine which formatting is used. Pass true to use scientific formatting and
  and pass false to use fixed formatting.
 - Argument `accurate_ipart_fix` is the same as in `drgnt_f64_to_srt`. Please refer to that function's documentation for details.
*/
DRGNT_PUB_API drgnt_i32 drgnt_f64_to_gen(drgnt_char* str, drgnt_u64 bits, drgnt_bool is_exp, drgnt_bool accurate_ipart_fix);



/*
 Calculate an upper bound for the length of the output of `drgnt_f32_to_fix` and `drgnt_f32_to_exp`. 

 - Argument `bits` has the same meaning as its counterpart in `drgnt_f32_to_fix` and `drgnt_f32_to_exp`.
 - Argument `precision` has the same meaning as its counterpart in `drgnt_f32_to_fix` and `drgnt_f32_to_exp`.
 - Argument `is_exp` is used to determine which formatting is used. Pass true to use scientific formatting (`drgnt_f32_to_exp`)
  and pass false to use fixed formatting (`drgnt_f32_to_fix`)
*/
DRGNT_PUB_API drgnt_i32 drgnt_estimate_len32(drgnt_u32 bits, drgnt_i32 precision, drgnt_bool is_exp);

/*
 Calculate an upper bound for the length of the output of `drgnt_f64_to_fix` and `drgnt_f64_to_exp`.

 - Argument `bits` has the same meaning as its counterpart in `drgnt_f64_to_fix` and `drgnt_f64_to_exp`.
 - Argument `precision` has the same meaning as its counterpart in `drgnt_f64_to_fix` and `drgnt_f64_to_exp`.
 - Argument `is_exp` is used to determine which formatting is used. Pass true to use scientific formatting (`drgnt_f64_to_exp`)
  and pass false to use fixed formatting (`drgnt_f64_to_fix`)
*/
DRGNT_PUB_API drgnt_i32 drgnt_estimate_len64(drgnt_u64 bits, drgnt_i32 precision, drgnt_bool is_exp);


/*
 Calculate an upper bound for the length of the output of `drgnt_f32_to_srt` and `drgnt_f32_to_gen`.

 - Argument `bits` has the same meaning as its counterpart in `drgnt_f32_to_srt` and `drgnt_f32_to_gen`.
 - Argument `use_shortest` is true to compute the output length of `drgnt_f32_to_srt` and false for `drgnt_f32_to_gen`.
 - Argument `is_exp` is used to determine if scientific formatting is used. This option is ignored when `use_shortest` is true.
*/
DRGNT_PUB_API drgnt_i32 drgnt_estimate_srt_len32(drgnt_u32 bits, drgnt_bool use_shortest, drgnt_bool is_exp);

/*
 Calculate an upper bound for the length of the output of `drgnt_f64_to_srt` and `drgnt_f64_to_gen`.

 - Argument `bits` has the same meaning as its counterpart in `drgnt_f64_to_srt` and `drgnt_f64_to_gen`.
 - Argument `use_shortest` is true to compute the output length of `drgnt_f64_to_srt` and false for `drgnt_f64_to_gen`.
 - Argument `is_exp` is used to determine if scientific formatting is used. This option is ignored when `use_shortest` is true.
*/
DRGNT_PUB_API drgnt_i32 drgnt_estimate_srt_len64(drgnt_u64 bits, drgnt_bool use_shortest, drgnt_bool is_exp);

#ifdef __cplusplus
}
#endif 

#endif /* DRAGONITO_H */

#ifdef DRGNT_IMPLEMENTATION
#include "dragonito_impl.h"
#endif