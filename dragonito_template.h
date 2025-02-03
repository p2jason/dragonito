#if DRGNT_TEMPLATE_64
#define drgnt_sint drgnt_i64
#define drgnt_uint drgnt_u64
#define drgnt_srt_result drgnt_srt_result64

#define drgnt_prepare_float  drgnt_prepare_f64
#define drgnt_split_float    drgnt_split_f64
#define drgnt_shortest_float drgnt_shortest_f64
#define drgnt_float_to_str          drgnt_f64_to_str
#define drgnt_float_to_str_shortest drgnt_f64_to_str_shortest
#define drgnt_estimate_len     drgnt_estimate_len64
#define drgnt_estimate_srt_len drgnt_estimate_srt_len64

#define drgnt_make_srt_result  drgnt_make_srt_result64
#define drgnt_is_tie_uncentered drgnt_is_tie_uncentered64
#define drgnt_is_pow5_multiple  drgnt_is_pow5_multiple64
#define drgnt_to_decimal     drgnt_to_decimal64
#define drgnt_to_decimal_srt drgnt_to_decimal_srt64
#define drgnt_print_int_tlz drgnt_print_int19_tlz
#define drgnt_print_int     drgnt_print_int19
#define drgnt_bsf drgnt_bsf64
#define drgnt_bsr drgnt_bsr64

#define DRGNT_MANTISSA_BITS DRGNT_MANTISSA_BITS64
#define DRGNT_EXPONENT_BITS DRGNT_EXPONENT_BITS64
#define DRGNT_EXPONENT_BIAS DRGNT_EXPONENT_BIAS64
#define DRGNT_EXPONENT_MIN  DRGNT_EXPONENT_MIN64
#define DRGNT_SEGMENT_LEN   DRGNT_SEGMENT_LEN64

#define DRGNT_SEG_START_IPART DRGNT_SEG_START_IPART64
#define DRGNT_SEG_START_FPART DRGNT_SEG_START_FPART64
#define DRGNT_SEG_THRESHOLD_IPART DRGNT_SEG_THRESHOLD_IPART64
#define DRGNT_SEG_THRESHOLD_FPART DRGNT_SEG_THRESHOLD_FPART64
#define DRGNT_IPART_INDEX_TABLE DRGNT_IPART_INDEX_TABLE64
#define DRGNT_FPART_INDEX_TABLE DRGNT_FPART_INDEX_TABLE64
#define DRGNT_SRT_TABLE DRGNT_SRT_TABLE64
#else
#define drgnt_sint drgnt_i32
#define drgnt_uint drgnt_u32
#define drgnt_srt_result drgnt_srt_result32

#define drgnt_prepare_float  drgnt_prepare_f32
#define drgnt_split_float    drgnt_split_f32
#define drgnt_shortest_float drgnt_shortest_f32
#define drgnt_float_to_str          drgnt_f32_to_str
#define drgnt_float_to_str_shortest drgnt_f32_to_str_shortest
#define drgnt_estimate_len     drgnt_estimate_len32
#define drgnt_estimate_srt_len drgnt_estimate_srt_len32

#define drgnt_make_srt_result  drgnt_make_srt_result32
#define drgnt_is_tie_uncentered drgnt_is_tie_uncentered32
#define drgnt_is_pow5_multiple  drgnt_is_pow5_multiple32
#define drgnt_to_decimal     drgnt_to_decimal32
#define drgnt_to_decimal_srt drgnt_to_decimal_srt32
#define drgnt_print_int_tlz drgnt_print_int9_tlz
#define drgnt_print_int     drgnt_print_int9
#define drgnt_bsf drgnt_bsf32
#define drgnt_bsr drgnt_bsr32

#define DRGNT_MANTISSA_BITS DRGNT_MANTISSA_BITS32
#define DRGNT_EXPONENT_BITS DRGNT_EXPONENT_BITS32
#define DRGNT_EXPONENT_BIAS DRGNT_EXPONENT_BIAS32
#define DRGNT_EXPONENT_MIN  DRGNT_EXPONENT_MIN32
#define DRGNT_SEGMENT_LEN   DRGNT_SEGMENT_LEN32

#define DRGNT_SEG_START_IPART DRGNT_SEG_START_IPART32
#define DRGNT_SEG_START_FPART DRGNT_SEG_START_FPART32
#define DRGNT_SEG_THRESHOLD_IPART DRGNT_SEG_THRESHOLD_IPART32
#define DRGNT_SEG_THRESHOLD_FPART DRGNT_SEG_THRESHOLD_FPART32
#define DRGNT_IPART_INDEX_TABLE DRGNT_IPART_INDEX_TABLE32
#define DRGNT_FPART_INDEX_TABLE DRGNT_FPART_INDEX_TABLE32
#define DRGNT_SRT_TABLE DRGNT_SRT_TABLE32
#endif

typedef struct drgnt_srt_result
{
	drgnt_uint mantissa;
	drgnt_sint exponent;
} drgnt_srt_result;

static drgnt_srt_result drgnt_make_srt_result(drgnt_uint mantissa, drgnt_sint exponent)
{
	drgnt_srt_result result = { mantissa, exponent };
	return result;
}

static drgnt_sint drgnt_prepare_float(drgnt_uint bits, drgnt_uint* m_unified, drgnt_sint* e_unified, drgnt_bool* neg_sign)
{
	const drgnt_uint mantissa_bits = DRGNT_MANTISSA_BITS;
	const drgnt_sint exponent_bias = DRGNT_EXPONENT_BIAS;

	const drgnt_uint mantissa_mask = ((drgnt_uint)1 << DRGNT_MANTISSA_BITS) - 1;
	const drgnt_uint exponent_mask = ((drgnt_uint)1 << DRGNT_EXPONENT_BITS) - 1;

	const drgnt_uint exp_shift = DRGNT_MANTISSA_BITS;
	const drgnt_uint sign_shift = DRGNT_MANTISSA_BITS + DRGNT_EXPONENT_BITS;

	drgnt_uint sign = (bits >> sign_shift) & 0b1;
	drgnt_uint exponent = (bits >> exp_shift) & exponent_mask;
	drgnt_uint mantissa = bits & mantissa_mask;

	if (exponent == exponent_mask)
	{
		return (mantissa != 0) ? 1 : (sign == 0 ? 2 : 3);
	}
	else if (exponent == 0 && mantissa == 0)
	{
		*neg_sign = (drgnt_bool)(sign != 0);
		return 4;
	}

	//Compute the unified mantissa and exponent
	*m_unified = mantissa;
	*e_unified = (drgnt_sint)exponent;
	*neg_sign = (drgnt_bool)(sign != 0);

	if (exponent != 0)
	{
		*m_unified |= (drgnt_uint)1 << mantissa_bits;
		*e_unified -= exponent_bias + mantissa_bits;
	}
	else
	{
		*e_unified = (drgnt_sint)1 - exponent_bias - mantissa_bits;
	}

	return 0;
}

static void drgnt_split_float(drgnt_uint m_unified, drgnt_sint e_unified, drgnt_uint* m_int, drgnt_sint* e_int, drgnt_uint* m_fract, drgnt_sint* e_fract)
{
	//Compute a different mantissa and exponent for the integer and fractional parts
	if (e_unified < 0)
	{
		if (e_unified >= -(drgnt_sint)DRGNT_MANTISSA_BITS)
		{
			*m_int = m_unified >> -e_unified;
			*m_fract = m_unified & ((1 << -e_unified) - 1);
		}
		else
		{
			*m_int = 0;
			*m_fract = m_unified;
		}

		*e_int = 0;
		*e_fract = e_unified;
	}
	else
	{
		*m_int = m_unified;
		*e_int = e_unified;

		*m_fract = 0;
		*e_fract = 0;
	}
}

static drgnt_i32 drgnt_float_to_str(drgnt_char* str, drgnt_uint bits, drgnt_i32 precision, drgnt_bool exp_form)
{
	//Compute the mantissa and exponent for the integer and fractional parts.
	//Also, early-out if the value is inf or nan.
	drgnt_uint m_unified;
	drgnt_sint e_unified;
	drgnt_bool neg_sign;
	drgnt_sint prep = drgnt_prepare_float(bits, &m_unified, &e_unified, &neg_sign);

	if (prep == 1) return drgnt_print_nan(str);
	else if (prep == 2) return drgnt_print_posinf(str);
	else if (prep == 3) return drgnt_print_neginf(str);
	else if (prep == 4) return drgnt_print_zeros(str, neg_sign, precision, exp_form);

	drgnt_uint m_int, m_fract;
	drgnt_sint e_int, e_fract;
	drgnt_split_float(m_unified, e_unified, &m_int, &e_int, &m_fract, &e_fract);

	//Compute formatting flags
	drgnt_bool no_prec = precision < 0;

	drgnt_bool has_ipart = m_int > 0;
	drgnt_bool has_fpart = m_fract > 0;

	drgnt_bool has_dot_fix = !exp_form && precision != 0;
	drgnt_bool has_dot_exp = exp_form && precision != 0;

	drgnt_bool trim_ipart_trailing_zeros = exp_form && !has_fpart && no_prec;
	drgnt_bool trim_fpart_leading_zeros = exp_form && !has_ipart;

	drgnt_bool print_ipart_zero = !exp_form || !has_fpart;
	drgnt_bool print_fpart_zero = !exp_form && no_prec;

	/* *********** Print floating point digits *********** */

	drgnt_char* buffer_ptr = str;
	drgnt_char* literal_ptr = buffer_ptr + neg_sign;
	drgnt_char* ipart_end = buffer_ptr;

	drgnt_bool more_digits = 0;
	drgnt_sint pow10_exp = 0;
	drgnt_sint prec_digits =
		precision
		+ exp_form //In exp form, we start counting precision from the first digit. This is here so we start counting from the second digit
		+ 1;       //Because we want to include the digit after the precision ends (so we can do rounding)

	if (neg_sign) *buffer_ptr++ = '-';
	if (no_prec) prec_digits = 0x7FFFFFFF;
	if (has_dot_exp) buffer_ptr++;

	if (has_ipart)
	{
		//Get the "pointer" to the "row" for the current exponent in the compressed coefficient table
		const drgnt_rowptr row_ptr = DRGNT_IPART_INDEX_TABLE[e_int];
		const drgnt_sint i_to_seg_idx = row_ptr.idx - row_ptr.seg_start;

		//Compute the index of the first (highest) non-zero segment
		const drgnt_uint seg_start = DRGNT_SEG_START_IPART[e_int];
		const drgnt_uint *seg_threshold = DRGNT_SEG_THRESHOLD_IPART[seg_start];
		const drgnt_uint seg_offset = ((drgnt_sint)seg_threshold[1] == e_int) && (m_int >= seg_threshold[0]);

		drgnt_uint dec_val, first_len;
		drgnt_sint i = seg_start + seg_offset;
		drgnt_sint end_idx = 0;

		/*
		  Since everything after `seg_end` is zero, `i` SHOULD always be less than `seg_end`. I don't have a
		 proof for this, but it makes sense that this would be the case. If that turns out to be wrong, this
		 would be a simple fix:

			if (i >= row_ptr.seg_end) i = row_ptr.seg_end - 1;
		*/

		//Print the first segment
		dec_val = (drgnt_uint)(drgnt_to_decimal(m_int, i + i_to_seg_idx, 0));
		first_len = (drgnt_sint)(drgnt_print_int_tlz(buffer_ptr, dec_val) - buffer_ptr);

		pow10_exp = first_len + (DRGNT_SEGMENT_LEN * i) - 1;
		buffer_ptr += first_len;

		end_idx = exp_form ? (pow10_exp + 1) - prec_digits : 0;
		prec_digits -= exp_form ? first_len : 0;

		//Print the following segments
		for (i--; i >= row_ptr.seg_start; i--)
		{
			if (prec_digits <= 0) break;
			if (prec_digits > 0 && exp_form) prec_digits -= DRGNT_SEGMENT_LEN;

			dec_val = drgnt_to_decimal(m_int, i + i_to_seg_idx, 0);
			buffer_ptr = drgnt_print_int(buffer_ptr, dec_val);
		}

		if (!trim_ipart_trailing_zeros)
		{
			//Print the remaining segments with all coefficients being zero
			for (; i >= 0; i--)
			{
				if (prec_digits <= 0) break;
				if (prec_digits > 0 && exp_form) prec_digits -= DRGNT_SEGMENT_LEN;

				buffer_ptr = drgnt_print_int(0, dec_val);
			}
		}
		else
		{
			//Trim trailing zeros
			while (buffer_ptr[-1] == '0') buffer_ptr--;
		}

		//If `end_idx` is negative, it means there are digits in the fractional part. If so, we'll
		//let the fractional part determine if there are more digits after the end
		if (end_idx > 0)
		{
			//Are there any non-zero digits after `end_idx`?
			more_digits = 0
				|| end_idx > e_int + (drgnt_sint)drgnt_bsf(m_int) //p > e + drgnt_bsf(m)
				|| !drgnt_is_pow5_multiple(m_int, end_idx);
		}
	}
	else if (print_ipart_zero)
	{
		*buffer_ptr++ = '0';
	}

	ipart_end = buffer_ptr;

	if (has_dot_fix)
		*buffer_ptr++ = '.';

	if (has_fpart)
	{
		//Get the "pointer" to the "row" for the current exponent in the compressed coefficient table
		const drgnt_rowptr row_ptr = DRGNT_FPART_INDEX_TABLE[-e_fract - 1];
		const drgnt_sint i_to_seg_idx = row_ptr.idx - row_ptr.seg_start;

		//Compute the index of the last segment
		const drgnt_sint end_idx = -e_fract - drgnt_bsf(m_fract);
		const drgnt_sint end_seg = (end_idx + DRGNT_SEGMENT_LEN - 1) / DRGNT_SEGMENT_LEN;
		drgnt_char* end_ptr = buffer_ptr + end_idx;

		drgnt_sint leading_zeros = 0;
		drgnt_sint start_prec_digits = prec_digits;

		drgnt_uint dec_val, first_len;
		drgnt_sint i = 0;

		if (trim_fpart_leading_zeros)
		{
			//Compute the index of the first non-zero segment
			const drgnt_sint m_bits = DRGNT_MANTISSA_BITS;
			const drgnt_sint m_bsr = m_bits - drgnt_bsr(m_fract);
			const drgnt_uint m_shifted = m_fract << m_bsr;
			const drgnt_sint e_shifted = -e_fract + m_bsr;

			const drgnt_uint seg_start = DRGNT_SEG_START_FPART[e_shifted - m_bits - 1];
			const drgnt_uint *seg_threshold = DRGNT_SEG_THRESHOLD_FPART[seg_start];
			const drgnt_uint seg_offset = ((drgnt_sint)seg_threshold[1] == e_shifted) && (m_shifted < seg_threshold[0]);

			i = seg_start + seg_offset;

			/*
			  Like with the ipart, since everything before `seg_start` is zero, `i` SHOULD always be greater than or
			 equal to `seg_start`. If that turns out to be wrong, this would be a simple fix:

				if (i < row_ptr.seg_start) i = row_ptr.seg_start;
			*/

			//Print the first segment
			dec_val = (drgnt_uint)(drgnt_to_decimal(m_fract, i + i_to_seg_idx, 1));
			first_len = (drgnt_sint)(drgnt_print_int_tlz(buffer_ptr, dec_val) - buffer_ptr);

			//Compute the pow10 exponent and adjust the end pointer
			leading_zeros = (DRGNT_SEGMENT_LEN * i) + (DRGNT_SEGMENT_LEN - first_len);
			pow10_exp = -leading_zeros - 1;
			end_ptr -= leading_zeros;

			buffer_ptr += first_len;
			prec_digits -= first_len;

			i += 1;
		}
		else
		{
			//Print the segments that are all zero
			for (; i < row_ptr.seg_start; i++)
			{
				if (prec_digits <= 0) break;
				if (prec_digits > 0) prec_digits -= DRGNT_SEGMENT_LEN;

				buffer_ptr = drgnt_print_int(buffer_ptr, 0);
			}
		}

		//Print the (remaining) fractional part
		for (; i < end_seg; i++)
		{
			if (prec_digits <= 0) break;
			if (prec_digits > 0) prec_digits -= DRGNT_SEGMENT_LEN;

			dec_val = drgnt_to_decimal(m_fract, i + i_to_seg_idx, 1);
			buffer_ptr = drgnt_print_int(buffer_ptr, dec_val);
		}

		//Set the end pointer to "trim" trailing zeros
		more_digits |= leading_zeros + start_prec_digits < (drgnt_sint)end_idx;
		buffer_ptr = no_prec ? end_ptr : buffer_ptr;
	}
	else if (print_fpart_zero)
	{
		*buffer_ptr++ = '0';
	}

	//Move the string back and add the '.' if we're in exponent form
	if (has_dot_exp)
	{
		literal_ptr[0] = literal_ptr[1];
		literal_ptr[1] = '.';
	}

	//Add a zero at the end. This covers the case where the integer part only
	//prints one digit and we are in exponent form with no precision.
	if (no_prec && buffer_ptr == literal_ptr + 2)
	{
		*buffer_ptr++ = '0';
	}

	//Perform precision adjustments
	if (!no_prec && prec_digits > 1)
	{
		//In this case, the float ends before we've reached the requested precision, so we just append zeros
		while (prec_digits-- > 1) *buffer_ptr++ = '0';
	}
	else if (!no_prec)
	{
		drgnt_char* prec_digit = buffer_ptr + prec_digits - 2;
		drgnt_char* round_digit = buffer_ptr + prec_digits - 1;
		drgnt_char  round_value = prec_digits < 1 ? *round_digit : '\0';

		drgnt_bool round_up_on_tie = *prec_digit & 1;
		drgnt_bool should_add_round = round_value > '5' || (round_value == '5' && (more_digits || round_up_on_tie));

		if (should_add_round)
		{
			for (drgnt_char* ptr = prec_digit; ptr >= literal_ptr; ptr -= 1)
			{
				if (*ptr == '.') { /* Do nothing */ }
				else if (*ptr == '9') { *ptr = '0'; }
				else if (*ptr != '9') { *ptr += 1; goto leave_rounding; }
			}

			//If we've reached here, it means we've reached the start of the buffer and we need to "prepend" another digit
			literal_ptr[0] = '1';
			pow10_exp += 1;

			if (!exp_form)
			{
				ipart_end[0] = '0';
				ipart_end[1] = '.';

				round_digit[0] = '0';
				buffer_ptr += 1;
			}
		}

	leave_rounding:
		buffer_ptr += prec_digits - 1;
	}

	//Print the exponent
	if (exp_form)
	{
		buffer_ptr = drgnt_print_exponent(buffer_ptr, (drgnt_i32)pow10_exp);
	}

	return (drgnt_i32)(buffer_ptr - str);
}

DRGNT_IMPL_API drgnt_i32 drgnt_estimate_len(drgnt_uint bits, drgnt_i32 precision, drgnt_bool is_exp)
{
	drgnt_uint m_unified;
	drgnt_sint e_unified;
	drgnt_bool neg_sign;
	drgnt_sint prep = drgnt_prepare_float(bits, &m_unified, &e_unified, &neg_sign);

	if (prep == 1) return 3; /* 3 */
	else if (prep == 2) return 4; /* +inf */
	else if (prep == 3) return 4; /* -inf */
	else if (prep == 4)
	{
		return (neg_sign ? 1 : 0)
			 + (precision == 0 ? 1 : (precision < 0 ? 3 : 2 + precision))
			 + (is_exp ? 3 : 0);
	}

	drgnt_uint m_int, m_fract;
	drgnt_sint e_int, e_fract;
	drgnt_split_float(m_unified, e_unified, &m_int, &e_int, &m_fract, &e_fract);

	drgnt_sint ipart_length = 0;
	drgnt_sint fpart_start = 0;
	drgnt_sint fpart_length = 0;

	if (m_int > 0)
	{
		//Only use the start index. Don't check the threshold. Just add one. It's faster and simpler.
		ipart_length = DRGNT_SEGMENT_LEN * (DRGNT_SEG_START_IPART[e_int] + 1);
	}

	if (m_int == 0 && is_exp)
	{
		const drgnt_sint m_bits = DRGNT_MANTISSA_BITS;
		const drgnt_sint m_bsr = m_bits - drgnt_bsr(m_fract);
		const drgnt_sint e_shifted = -e_fract + m_bsr;

		//Unlike the case for ipart, we don't want to add one.
		fpart_start = DRGNT_SEGMENT_LEN * DRGNT_SEG_START_FPART[e_shifted - m_bits - 1];
	}

	if (m_fract > 0)
	{
		fpart_length = -e_fract - (drgnt_sint)drgnt_bsf(m_fract);
	}

	//Compute the length estimate
	drgnt_sint total_length = 0;

	if (neg_sign) total_length += 1;
	if (is_exp) total_length += 1 /*'e'*/ + 1 /*'+' or '-'*/ + 9 /* max digits in exponent (since we use print_int9, this needs to be 9 */;
	if (precision != 0) total_length += 1; /* The '.' in the middle/start of the string */

	if (precision < 0)
	{
		if (is_exp)
			total_length += m_int > 0 ? (ipart_length + fpart_length) : (fpart_length - fpart_start);
		else 
			total_length += ipart_length + fpart_length;
	}
	else if (precision >= 0)
	{
		if (is_exp)
			total_length += precision + 1/* +1 because we start counting after the first digit */;
		else
			total_length += ipart_length + precision;
	}

	//We print in batches and then trim the trailing zeros. We need to have enough space to print the
	//whole batch. Adding an extra batch size to the total length is a simple and easy way to gurantee
	//we have enouh space.
	total_length += DRGNT_SEGMENT_LEN;

	return (drgnt_i32)total_length;
}

static drgnt_bool drgnt_is_tie_uncentered(drgnt_sint e, drgnt_uint mantissa_size)
{
	return e > 0 && mantissa_size % 4 == 2;
}

static drgnt_srt_result drgnt_shortest_float(drgnt_uint m, drgnt_sint e)
{
	/*
	 This is a modified version of the core routine in Teju Jagua (https://github.com/cassioneri/teju_jagua). At the
	 time of writting this, that project is still in-progress and the author hasn't released a paper on it yet.
	*/
	// SPDX-License-Identifier: APACHE-2.0
	// SPDX-FileCopyrightText: 2021-2024 Cassio Neri <cassio.neri@gmail.com>

	const drgnt_uint mantissa_size = DRGNT_MANTISSA_BITS;
	const drgnt_uint mantissa_high = (drgnt_uint)1 << DRGNT_MANTISSA_BITS;

	drgnt_bool(* const is_tie)(drgnt_uint, drgnt_sint) = &drgnt_is_pow5_multiple;
	drgnt_bool(* const is_tie_uncentered)(drgnt_sint, drgnt_uint) = &drgnt_is_tie_uncentered;

	//Get the coefficients for the current exponent
	drgnt_sint f = drgnt_log10_pow2((drgnt_i32)e);

	const drgnt_uint* coeffs = DRGNT_SRT_TABLE[e - DRGNT_EXPONENT_MIN];

	//Find the shortest representation
	if (m != mantissa_high || e == DRGNT_EXPONENT_MIN)
	{
		drgnt_uint m_a = 2 * m - 1;
		drgnt_uint m_b = 2 * m + 1;
		drgnt_uint a = drgnt_to_decimal_srt(m_a, coeffs);
		drgnt_uint b = drgnt_to_decimal_srt(m_b, coeffs);
		drgnt_uint q = b / 10;
		drgnt_uint s = q * 10;

		if (s >= a)
		{
			if (s == b)
			{
				if (m % 2 == 0 || !is_tie(m_b, f))
					return drgnt_make_srt_result(q, f + 1);
			}
			else if (m % 2 == 0 && is_tie(m_a, f))
			{
				return drgnt_make_srt_result(q, f + 1);
			}
			else if (s > a)
			{
				return drgnt_make_srt_result(q, f + 1);
			}
		}

		if ((a + b) % 2 == 1)
			return drgnt_make_srt_result((a + b) / 2 + 1, f);

		drgnt_uint m_c = 2 * 2 * m;
		drgnt_uint c_2 = drgnt_to_decimal_srt(m_c, coeffs);
		drgnt_uint c = c_2 / 2;

		if (c_2 % 2 == 0 || c % 2 == 0 && is_tie(c_2, -f))
			return drgnt_make_srt_result(c, f);

		return drgnt_make_srt_result(c + 1, f);
	}

	drgnt_uint m_b = 2 * mantissa_high + 1;
	drgnt_uint m_a = 4 * mantissa_high - 1;
	drgnt_uint b = drgnt_to_decimal_srt(m_b, coeffs);
	drgnt_uint a = drgnt_to_decimal_srt(m_a, coeffs) / 2;

	if (b > a)
	{
		drgnt_uint q = b / 10;
		drgnt_uint s = q * 10;

		if (s > a || (s == a && is_tie_uncentered(f, mantissa_size)))
			return drgnt_make_srt_result(q, f + 1);

		drgnt_uint m_c = 2 * 2 * mantissa_high;
		drgnt_uint c_2 = drgnt_to_decimal_srt(m_c, coeffs);
		drgnt_uint c = c_2 / 2;

		if (c == a && !is_tie_uncentered(f, mantissa_size))
			return drgnt_make_srt_result(c + 1, f);

		if (c_2 % 2 == 0 || (c % 2 == 0 && is_tie(c_2, -f)))
			return drgnt_make_srt_result(c, f);

		return drgnt_make_srt_result(c + 1, f);
	}
	else if (is_tie_uncentered(f, mantissa_size))
	{
		return drgnt_make_srt_result(a, f);
	}

	drgnt_uint m_c = 10 * 2 * 2 * mantissa_high;
	drgnt_uint c_2 = drgnt_to_decimal_srt(m_c, coeffs);
	drgnt_uint c = c_2 / 2;

	if (c_2 % 2 == 0 || (c % 2 == 0 && is_tie(c_2, -f)))
		return drgnt_make_srt_result(c, f - 1);

	return drgnt_make_srt_result(c + 1, f - 1);
}

static drgnt_i32 drgnt_float_to_str_shortest(drgnt_char* str, drgnt_uint bits, drgnt_bool use_shortest, drgnt_bool force_exp_form, drgnt_bool use_accurate_ipart_fix)
{
	drgnt_char* buffer_ptr = str;
	drgnt_sint mantissa_len = 0;
	drgnt_sint exponent_len = 0;

	//Extract the mantissa and exponent
	drgnt_uint m_unified;
	drgnt_sint e_unified;
	drgnt_bool neg_sign;
	drgnt_sint prep = drgnt_prepare_float(bits, &m_unified, &e_unified, &neg_sign);

	if (prep == 1) return drgnt_print_nan(str);
	else if (prep == 2) return drgnt_print_posinf(str);
	else if (prep == 3) return drgnt_print_neginf(str);
	else if (prep == 4) return drgnt_print_zeros(str, neg_sign, 0, !use_shortest && force_exp_form);

	//Get the SRT representation of the float
	drgnt_srt_result result = drgnt_shortest_float(m_unified, e_unified);

	//Remove trailing zero from the mantissa and adjsut the exponent
	for (; result.mantissa != 0; result.exponent++)
	{
		if (result.mantissa % 10 == 0)
			result.mantissa /= 10;
		else break;
	}

	//Compute the length of the mantissa and exponent (in digits)
	if (result.mantissa >= 1u) mantissa_len++;
	if (result.mantissa >= 10u) mantissa_len++;
	if (result.mantissa >= 100u) mantissa_len++;
	if (result.mantissa >= 1000u) mantissa_len++;
	if (result.mantissa >= 10000u) mantissa_len++;
	if (result.mantissa >= 100000u) mantissa_len++;
	if (result.mantissa >= 1000000u) mantissa_len++;
	if (result.mantissa >= 10000000u) mantissa_len++;
	if (result.mantissa >= 100000000u) mantissa_len++;
	if (result.mantissa >= 1000000000u) mantissa_len++;
#if DRGNT_TEMPLATE_64
	if (result.mantissa >= 10000000000ull) mantissa_len++;
	if (result.mantissa >= 100000000000ull) mantissa_len++;
	if (result.mantissa >= 1000000000000ull) mantissa_len++;
	if (result.mantissa >= 10000000000000ull) mantissa_len++;
	if (result.mantissa >= 100000000000000ull) mantissa_len++;
	if (result.mantissa >= 1000000000000000ull) mantissa_len++;
	if (result.mantissa >= 10000000000000000ull) mantissa_len++;
	if (result.mantissa >= 100000000000000000ull) mantissa_len++;
	if (result.mantissa >= 1000000000000000000ull) mantissa_len++;
	if (result.mantissa >= 10000000000000000000ull) mantissa_len++;
#endif

	drgnt_sint abs_exponent = result.exponent < 0 ? -result.exponent : result.exponent;
	if (abs_exponent >= 1u) exponent_len++;
	if (abs_exponent >= 10u) exponent_len++;
	if (abs_exponent >= 100u) exponent_len++;
	if (abs_exponent >= 1000u) exponent_len++;

	//Print the float
	drgnt_sint fix_form_length = neg_sign + mantissa_len;
	drgnt_sint exp_form_length = neg_sign
		+ mantissa_len
		+ exponent_len
		+ (/* the '.' */ mantissa_len > 1)
		+ /* 'e+'/'e-' */ 2;

	if (0) {}
	else if (result.exponent <= -mantissa_len) fix_form_length += /* the '0.' */2 + /* leading zeros */ -(result.exponent + mantissa_len);
	else if (result.exponent >= 0) fix_form_length += result.exponent;
	else if (1) fix_form_length += /* the '.' */ 1;

	drgnt_bool print_in_exp_form = (use_shortest && exp_form_length < fix_form_length) || force_exp_form;

	if (neg_sign)
		*buffer_ptr++ = '-';

	if (print_in_exp_form)
	{
		//Print the mantissa
		if (mantissa_len == 1)
		{
			//If the mantissa only has a single digit, then we don't want to print the decimal separator.
			//For exmple, if we are printing the number 10, we want to print it as "1e+1" instead of "1.0e+1"
			*buffer_ptr++ = (drgnt_char)('0' + result.mantissa);
		}
		else
		{
			drgnt_char* start = buffer_ptr;

			buffer_ptr += 1;
			buffer_ptr = drgnt_print_int_tlz(buffer_ptr, result.mantissa);

			start[0] = start[1];
			start[1] = '.';
		}

		//Print the exponent
		buffer_ptr = drgnt_print_exponent(buffer_ptr, (drgnt_i32)(result.exponent + (mantissa_len - 1)));
	}
	else
	{
		drgnt_sint i = 0;
		drgnt_sint integer_digits = result.exponent + mantissa_len;

		if (result.exponent >= 0) /* Handle numbers that have no fractional part (e.g. 1000, 123, 99900000) */
		{
			if (use_accurate_ipart_fix)
			{
				//Print the exact value represented by the float
				return drgnt_float_to_str(str, bits, 0, 0);
			}
			else
			{
				//Print the mantissa
				buffer_ptr = drgnt_print_int_tlz(buffer_ptr, result.mantissa);

				//Print trailing zeros
				for (i = 0; i < result.exponent; i++)
					*buffer_ptr++ = '0';
			}
		}
		else if (integer_digits <= 0) /* Handle numbers that have no integer part (e.g. 0.1, 0.0625, 0.000001) */
		{
			*buffer_ptr++ = '0';
			*buffer_ptr++ = '.';

			//Print leading zeros
			for (i = 0; i < -integer_digits; i++)
				*buffer_ptr++ = '0';

			//Print the mantissa
			buffer_ptr = drgnt_print_int_tlz(buffer_ptr, result.mantissa);
		}
		else /* Handle numbers that have both an integer and fractional part (e.g. 1.1, 100.2, 12345.000001) */
		{
			drgnt_char* start = buffer_ptr;

			buffer_ptr += 1;
			buffer_ptr = drgnt_print_int_tlz(buffer_ptr, result.mantissa);

			for (i = 0; i < integer_digits; i++)
				start[i] = start[i + 1];
			
			start[integer_digits] = '.';
		}
	}

	return (drgnt_i32)(buffer_ptr - str);
}

DRGNT_IMPL_API drgnt_i32 drgnt_estimate_srt_len(drgnt_uint bits, drgnt_bool use_shortest, drgnt_bool is_exp)
{
	if (use_shortest || is_exp)
		return DRGNT_SEGMENT_LEN
			+ /* the '.' */1
			+ /* the 'e+'/'e-' */2
			+ /* the exponent */DRGNT_SEGMENT_LEN32
			+ /* the minus sign */1;
	
	drgnt_uint m_unified;
	drgnt_sint e_unified;
	drgnt_bool neg_sign;
	drgnt_sint prep = drgnt_prepare_float(bits, &m_unified, &e_unified, &neg_sign);

	(void)prep;

	drgnt_uint m_int, m_fract;
	drgnt_sint e_int, e_fract;
	drgnt_split_float(m_unified, e_unified, &m_int, &e_int, &m_fract, &e_fract);

	drgnt_i32 ipart_length = 0;
	drgnt_i32 fpart_start = 0;

	if (m_int > 0)
	{
		//Only use the start index. Don't check the threshold. Just add one. It's faster and simpler.
		ipart_length = DRGNT_SEGMENT_LEN * (DRGNT_SEG_START_IPART[e_int] + 1);
	}
	else
	{
		const drgnt_sint m_bits = DRGNT_MANTISSA_BITS;
		const drgnt_sint m_bsr = m_bits - drgnt_bsr(m_fract);
		const drgnt_sint e_shifted = -e_fract + m_bsr;

		//Add one because this will give us the first SEGMENT that isn't zero. There could be more leading
		//zeros inside the segment, however. 
		fpart_start = DRGNT_SEGMENT_LEN * (DRGNT_SEG_START_FPART[e_shifted - m_bits - 1] + 1);
	}

	if (m_int)
		return ipart_length + /* just padding */DRGNT_SEGMENT_LEN + neg_sign;
	else if (m_fract)
		return fpart_start + /* the mantissa */DRGNT_SEGMENT_LEN + /* the '0.' */2 + neg_sign;
	else
		return DRGNT_SEGMENT_LEN + /* to '.' */1 + neg_sign;
}

#undef drgnt_sint
#undef drgnt_uint
#undef drgnt_srt_result

#undef drgnt_prepare_float
#undef drgnt_split_float
#undef drgnt_shortest_float
#undef drgnt_float_to_str
#undef drgnt_float_to_str_shortest
#undef drgnt_estimate_len
#undef drgnt_estimate_srt_len

#undef drgnt_make_srt_result
#undef drgnt_is_tie_uncentered
#undef drgnt_is_pow5_multiple
#undef drgnt_to_decimal
#undef drgnt_to_decimal_srt
#undef drgnt_print_int_tlz
#undef drgnt_print_int
#undef drgnt_bsf
#undef drgnt_bsr

#undef DRGNT_MANTISSA_BITS
#undef DRGNT_EXPONENT_BITS
#undef DRGNT_EXPONENT_BIAS
#undef DRGNT_EXPONENT_MIN
#undef DRGNT_SEGMENT_LEN  

#undef DRGNT_SEG_START_IPART
#undef DRGNT_SEG_START_FPART
#undef DRGNT_SEG_THRESHOLD_IPART
#undef DRGNT_SEG_THRESHOLD_FPART
#undef DRGNT_IPART_INDEX_TABLE
#undef DRGNT_FPART_INDEX_TABLE
#undef DRGNT_SRT_TABLE