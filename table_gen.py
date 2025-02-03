import math

def iter_float(max_e, min_e, max_p, min_p, segment_length):
	for e in range(max_e + 1):
		for p in range(max_p + 1):
			yield (e, p, 2**e, 10**(p*segment_length), True)

	for e in range(1, -min_e + 1):
		for p in range(1, min_p + 1):
			yield (e, p, 10**(p*segment_length), 2**e, False)

	return

def iter_float_srt(max_e, min_e):
	for e in range(min_e, max_e+1):
		E = e - 1
		F = int(math.floor(e * math.log10(2)))
		A = 1
		B = 1

		if E < 0: B *= 2**(-E)
		else:     A *= 2**(+E)

		if F < 0: A *= 10**(-F)
		else:     B *= 10**(+F)

		yield (e, A, B)

	return

def len2d(arr):
	return len(arr) * len(arr[0])

def array2d(rows, cols):
	return [[0 for _ in range(cols)] for _ in range(rows)]

def wrap_2d(arr, width, default=0):
	rows = (len(arr) + width - 1) // width
	idx = 0

	wrapped = array2d(rows, width)

	for i in range(rows):
		for j in range(width):
			wrapped[i][j] = default if idx >= len(arr) else arr[idx]
			idx += 1

	return wrapped

# `callback` returns true if we're "higher" than the desired value and None if we found it
def upper_bound(low, high, callback):
	last_val = None

	while low <= high:
		mid = (high + low) // 2

		if callback(mid):
			high = mid - 1
			last_val = mid
		else:
			low = mid + 1

	return last_val

def gcd(a, b):
	old_r, r = (a, b)
	
	while r != 0:
		quotient = old_r // r
		old_r, r = (r, old_r - quotient * r)
		
	return old_r

#
# Functions from Daniel Lemier:
#     https://github.com/lemire/exactshortlib
#

def gaps(z, M):
	""" We  want to find the locations of all of the minimum and
	maximum of (w * z) % M for w = 1, ..., M/gcd(z,M)
	"""
	if M <= 0:
		raise ValueError('M must be positive')
	if z < 0:
		raise ValueError('z must be positive')
	if (z % M) == 0:
		raise ValueError('z should not be a multiple of M')
	w = 1
	lambdal = []
	a = z % M
	alpha = 1
	b = z % M
	beta = 1
	while True:
		v = (a + b) % M
		if v < a:
			t = a // (M-b)
			if a % (M-b) == 0:
				lambdal.append(w)
				break
			lambdal.append(w)
			w = w + alpha + (t-1) * beta
			alpha = w
			a = (a + t*b) %M
		else: # elif v>b:
			t = (M-b-1)//a
			lambdal.append(w)
			w = w + beta + (t-1) * alpha
			beta = w
			b = (b + t*a) %M
	return lambdal

def find_min_max(z, M, b):
	""" We want to find the location of all of the minimum and
	maximum of (w * z + b) %M for w = 1...
	"""
	if M <= 0:
		raise ValueError('M must be positive')
	facts = gaps(z,M)
	# When w = 0, we have that (w * z + b) %M = b % M
	mi = (z+b)%M
	ma = (z+b)%M
	fact_index = 0
	minima = [1]
	maxima = [1]
	w = 1
	while True:
		offindex = facts[fact_index]
		candidate = (z * (w + offindex) + b)%M
		if w + offindex > M:
			break
		if candidate < mi:
			# we have a new minimum.
			w += offindex
			minima.append(w)
			mi = candidate
		elif candidate > ma:
			# we have a new maximum.
			w += offindex
			maxima.append(w)    
			ma = candidate
		else:
			fact_index += 1
			if fact_index == len(facts):
				break
	return (minima,maxima)

# Modified version of find_min_max
def find_min_max_fast(z, M, b, W):
	""" We want to find the location of all of the minimum and
	maximum of (w * z + b) %M for w = 1..., W
	"""
	if M <= 0:
		raise ValueError('M must be positive')
	facts = gaps(z,M)
	# When w = 0, we have that (w * z + b) %M = b % M
	mi = (z+b)%M
	ma = (z+b)%M
	fact_index = 0
	minima = [1]
	maxima = [1]
	w = 1
	limit = min(W, M)
	while True:
		offindex = facts[fact_index]
		candidate = (z * (w + offindex) + b)%M
		if w + offindex > limit:
			break
		if candidate < mi:
			# we have a new minimum.
			w += offindex
			minima.append(w)
			mi = candidate
		elif candidate > ma:
			n0 = (z * w + b) % M
			r0 = M - 1 - n0
			i0 = r0 // (z * offindex)
			
			# we have a new maximum.
			w += max(i0, 1) * offindex

			if w > limit:
				maxima.append(limit)
				break
			else:
				maxima.append(w)
				
			ma = candidate
		else:
			fact_index += 1
			if fact_index == len(facts):
				break
	return (minima,maxima)

#
def count_fractional_digits(m, e):
	if e >= 0: raise Exception("e >= 0")
	
	for i in range(1, 10000):
		if (m * (10**i)) % (2**(-e)) == 0:
			return i
	
	raise Exception("Digit count higher than expected")

def count_integer_digits(m, e):
	return math.ceil(math.log10(m * 2**e))

# The brute-force version of find_best_m_fast
def find_best_m(C, B, M):
	m_best = 0
	x_best = 1

	for m in range(1, M + 1):
		x = (m * C) // B + 1

		if m_best * x < x_best * m:
			m_best = m
			x_best = x

	return m_best

def find_best_m_fast(C, B, M):
	mnma, mxma = find_min_max_fast(C, B, 0, M)
	
	Bp = B // gcd(C, B)
	
	prev_m = 0
	prev_r = 1
	
	for m in mnma + mxma:
		r = (m * C) % B
		n = m + Bp * ((M - m) // Bp)
	
		if prev_m * (B - r) < n * (B - prev_r):
			prev_m = n
			prev_r = r

	return prev_m

def compute_min_k_for_single(A, B, max_m):
	C = A % B

	if C == 0: return 0
	if B > max_m * C: return 0

	for i in range(0, 1024):
		k = 1 << i
		
		if k * C * max_m - B * max_m * ((k * C) // B) < k * B:
			return i
	
	raise Exception("Not enough bits in k")

def compute_min_k(max_m, max_e, min_e, max_p, min_p, segment_length):
	minimum_k = 0

	for (_, _, A, B, _) in iter_float(max_e, min_e, max_p, min_p, segment_length):
		# This is a comment
		minimum_k = max(minimum_k, compute_min_k_for_single(A, B, max_m))

	return minimum_k

def compute_min_k_srt(max_m, max_e, min_e):
	minimum_k = 0


	for (e, A, B) in iter_float_srt(max_e, min_e):
		# This is a comment
		minimum_k = max(minimum_k, compute_min_k_for_single(A, B, 40 * max_m))

	return minimum_k

def compute_coeffs(A, B, M, k, max_m):
	k_shift = 1 << k

	# Compute integer coefficient
	int_coeff = (A // B) % M

	# Compute C and return if C is too small
	C = A % B
	
	if C == 0 or B > max_m * C:
		return (int_coeff, 0, 0, 0)

	# Compute G and find the `m` for which the fraction floor(m * G / k + 1) / m is minimized
	G = (k_shift * C // B)
	
	lowest_m = find_best_m_fast(C, B, max_m)
	x_for_m = (lowest_m * G) // k_shift + 1

	return (int_coeff, G, lowest_m, x_for_m)

def compute_coeff_tables(max_m, max_e, min_e, max_p, min_p, segment_length, min_k):
	i_table = array2d(max_e + 1, max_p + 1)
	f_table = array2d(-min_e, min_p)

	for (e_idx, p_idx, A, B, is_ipart) in iter_float(max_e, min_e, max_p, min_p, segment_length):
		# This is a comment
		coeffs = compute_coeffs(A, B, 10**segment_length, min_k, max_m)

		if is_ipart:
			i_table[e_idx][p_idx] = coeffs
		else:
			f_table[e_idx - 1][p_idx - 1] = coeffs

	return i_table, f_table

def compute_srt_coeff_tables(max_m, max_e, min_e, min_k):
	table = []

	for (e, A, B) in iter_float_srt(max_e, min_e):
		table.append(compute_coeffs(A, B, 2**64, min_k, 40 * max_m))

	return table

def compress_coeff_table(table):
	index_table = []
	coeff_table = []

	for e in range(len(table)):
		cur_row = table[e]

		start = 0
		end = len(cur_row)
		idx = len(coeff_table)

		# Trim start
		while start < end:
			coeff = cur_row[start]

			if coeff == (0, 0, 0, 0):
				start += 1
			else:
				break

		# Trim end
		while start < end:
			coeff = cur_row[end - 1]

			if coeff == (0, 0, 0, 0):
				end -= 1
			else:
				break

		# Append the trimmed row to the coeff table
		coeff_table.extend(cur_row[start:end])
		index_table.append((idx, start, end))

		# Sanity check
		if start >= end:
			raise Exception("Uhhh... `start >= end` doesn't make sense. It would be the row is all zeros (or there's a bug in the compression method)")
	
	return coeff_table, index_table

def compute_start_tables(max_e, mantissa_bits, segment_length):
	is_pos = max_e >= 0
	max_e = abs(max_e)

	def calc_segment(m, e):
		digits = 0

		# Compute the start indices using integer arithmetic (instead of using log10 directly)
		# because of floating point accuracy issues that occur for some float64 inputs
		if e >= 0:
			# Equivalent to:
			# return +(int(math.log10(m) + math.log10(2**+e)) // segment_length)

			while (m * 2**e) >= 10**digits:
				digits += 1

			return (digits - 1) // segment_length
		else:
			# Equivalent to:
			#
			# # Divinding a negative number by a positive rounds down towards -inf, which is the 
			# # opposite of what we want. So instead of doing a // b, we do -(-a // b)
			# return -(int(math.log10(2**-e) - math.log10(m)) // segment_length)

			while ((m * 10**digits) >> -e) == 0:
				digits += 1

			return -((digits - 1) // segment_length)

	min_m = 1 << mantissa_bits
	max_m = (1 << (mantissa_bits + 1)) - 1

	exponent_starts = [ 0 for _ in range(max_e + 1) ]
	segment_threshold = [ (0, max_e+1) for _ in range(calc_segment(max_m, max_e)) ]

	for i in range(max_e + 1):
		exponent = i if is_pos else -(i + mantissa_bits + 1)

		min_seg = calc_segment(min_m, exponent)
		max_seg = calc_segment(max_m, exponent)
		seg_idx = min(abs(min_seg), abs(max_seg))

		if max_seg > min_seg + 1:
			raise Exception(f"The difference of the digit count of the min and max values of exponent {exponent} is greater than one")

		exponent_starts[i] = seg_idx

		if min_seg + 1 == max_seg:
			threshold_m = upper_bound(min_m, max_m, lambda m: calc_segment(m, exponent) > min_seg)
			segment_threshold[seg_idx] = (threshold_m, abs(exponent))

	return (exponent_starts, segment_threshold)


##
## 32-bit float values
##

f32_mantissa_bits = 23
f32_exponent_bits = 8
f32_exponent_bias = 127
f32_segment_length = 9

f32_min_k = 32 # 24
f32_min_k_srt = 32
f32_segment_bit_count = (10**f32_segment_length).bit_length()

f32_max_mantissa = (1 << (f32_mantissa_bits + 1)) - 1
f32_min_mantissa_i = 1 << f32_mantissa_bits
f32_min_mantissa_f = 1

# Exp (min: -149, max: 104)
f32_max_exponent = (1 << f32_exponent_bits) - 2 - f32_exponent_bias - f32_mantissa_bits
f32_min_exponent = 1 - f32_exponent_bias - f32_mantissa_bits

f32_max_digits = count_integer_digits(f32_max_mantissa, f32_max_exponent)
f32_min_digits = count_fractional_digits(f32_min_mantissa_f, f32_min_exponent)

f32_max_segments = (f32_max_digits + f32_segment_length - 1) // f32_segment_length
f32_min_segments = (f32_min_digits + f32_segment_length - 1) // f32_segment_length

##
## 64-bit float values
##

f64_mantissa_bits = 52
f64_exponent_bits = 11
f64_exponent_bias = 1023
f64_segment_length = 19

f64_min_k = 64 # 53
f64_min_k_srt = 64
f64_segment_bit_count = (10**f64_segment_length).bit_length()

f64_max_mantissa = (1 << (f64_mantissa_bits + 1)) - 1
f64_min_mantissa_i = 1 << f64_mantissa_bits
f64_min_mantissa_f = 1

# Exp (min: -1074, max: 972)
f64_max_exponent = ((1 << f64_exponent_bits) - 1) - f64_exponent_bias - f64_mantissa_bits
f64_min_exponent = 1 - f64_exponent_bias - f64_mantissa_bits

f64_max_digits = count_integer_digits(f64_max_mantissa, f64_max_exponent)
f64_min_digits = count_fractional_digits(f64_min_mantissa_f, f64_min_exponent)

f64_max_segments = (f64_max_digits + f64_segment_length - 1) // f64_segment_length
f64_min_segments = (f64_min_digits + f64_segment_length - 1) // f64_segment_length


def generate_constants():
	# Generate 32-bit table
	print("Computing 32-bit tables...")
	i_table32, f_table32 = compute_coeff_tables(f32_max_mantissa, f32_max_exponent, f32_min_exponent, f32_max_segments, f32_min_segments, f32_segment_length, f32_min_k)
	srt_table32 =      compute_srt_coeff_tables(f32_max_mantissa, f32_max_exponent, f32_min_exponent, f32_min_k_srt)

	# Generate 64-bit table
	print("Computing 64-bit tables...")
	i_table64, f_table64 = compute_coeff_tables(f64_max_mantissa, f64_max_exponent, f64_min_exponent, f64_max_segments, f64_min_segments, f64_segment_length, f64_min_k)
	srt_table64 =      compute_srt_coeff_tables(f64_max_mantissa, f64_max_exponent, f64_min_exponent, f64_min_k_srt)

	# Compress tables
	i_coeff_table32, i_index_table32 = compress_coeff_table(i_table32)
	f_coeff_table32, f_index_table32 = compress_coeff_table(f_table32)

	i_coeff_table64, i_index_table64 = compress_coeff_table(i_table64)
	f_coeff_table64, f_index_table64 = compress_coeff_table(f_table64)

	# Write to file
	print("Generating header...")

	with open("./dragonito_constants.h", "w") as f:
		# Generate constants
		f.write(f"static const drgnt_i32 DRGNT_MANTISSA_BITS32 = {f32_mantissa_bits};\n")
		f.write(f"static const drgnt_i32 DRGNT_EXPONENT_BITS32 = {f32_exponent_bits};\n")
		f.write(f"static const drgnt_i32 DRGNT_EXPONENT_BIAS32 = {f32_exponent_bias};\n")
		f.write(f"static const drgnt_i32 DRGNT_EXPONENT_MIN32  = {f32_min_exponent};\n")
		f.write(f"static const drgnt_i32 DRGNT_SEGMENT_LEN32   = {f32_segment_length};\n")
		f.write("\n")
		f.write(f"static const drgnt_i32 DRGNT_MANTISSA_BITS64 = {f64_mantissa_bits};\n")
		f.write(f"static const drgnt_i32 DRGNT_EXPONENT_BITS64 = {f64_exponent_bits};\n")
		f.write(f"static const drgnt_i32 DRGNT_EXPONENT_BIAS64 = {f64_exponent_bias};\n")
		f.write(f"static const drgnt_i32 DRGNT_EXPONENT_MIN64  = {f64_min_exponent};\n")
		f.write(f"static const drgnt_i32 DRGNT_SEGMENT_LEN64   = {f64_segment_length};\n")
		f.write("\n")

		# Generate start tables
		def print_seg_start_table(table, suffix):
			f.write(f"static const drgnt_u8  DRGNT_SEG_START_{suffix}[{len(table)}] = {{ ")
			f.write(", ".join([ f"{i}" for i in table ]))
			f.write(" };\n")

		def print_seg_threshold_table(arr_type, table, suffix):
			f.write(f"static const {arr_type} DRGNT_SEG_THRESHOLD_{suffix}[{len(table)}][2] = {{ ")
			f.write(", ".join([ f"{{{i[0]}, {i[1]}}}" for i in table ]))
			f.write(" };\n")

		def print_index_table(arr_type, arr_name, table):
			f.write(f"static const {arr_type} {arr_name}[{len(table)}] = {{ ")
			f.write(", ".join([ f"{{{idx[0]},{idx[1]},{idx[2]}}}" for idx in table ]))
			f.write(" };\n")

		def print_coeff_table(arr_type, arr_name, table, formatter, width):
			f.write(f"static const {arr_type} {arr_name}[{len(table)}][4] = \n{{\n")

			idx = 0

			while True:
				f.write("\t")

				for i in range(width):
					if idx >= len(table):
						break

					f.write(f"{{")
					f.write(f"{formatter(table[idx][0])},")
					f.write(f"{formatter(table[idx][1])},")
					f.write(f"{formatter(table[idx][2])},")
					f.write(f"{formatter(table[idx][3])}")
					f.write(f"}}, ")
					idx += 1

				f.write("\n")

				if idx >= len(table):
					break

			f.write("};\n\n")

		ipart_start32 = compute_start_tables(f32_max_exponent, f32_mantissa_bits, f32_segment_length)
		fpart_start32 = compute_start_tables(f32_min_exponent, f32_mantissa_bits, f32_segment_length)
		ipart_start64 = compute_start_tables(f64_max_exponent, f64_mantissa_bits, f64_segment_length)
		fpart_start64 = compute_start_tables(f64_min_exponent, f64_mantissa_bits, f64_segment_length)

		print_seg_start_table(ipart_start32[0], "IPART32")
		print_seg_start_table(fpart_start32[0], "FPART32")
		print_seg_start_table(ipart_start64[0], "IPART64")
		print_seg_start_table(fpart_start64[0], "FPART64")

		f.write("\n")

		print_seg_threshold_table("drgnt_u32", ipart_start32[1], "IPART32")
		print_seg_threshold_table("drgnt_u32", fpart_start32[1], "FPART32")
		print_seg_threshold_table("drgnt_u64", ipart_start64[1], "IPART64")
		print_seg_threshold_table("drgnt_u64", fpart_start64[1], "FPART64")

		f.write("\n")

		# Print float32 tables
		print_index_table("drgnt_rowptr", "DRGNT_IPART_INDEX_TABLE32", i_index_table32)
		print_index_table("drgnt_rowptr", "DRGNT_FPART_INDEX_TABLE32", f_index_table32)

		f.write("\n")

		print_coeff_table("drgnt_u32", "DRGNT_IPART_COEFF_TABLE32", i_coeff_table32, lambda x: f"0x{x:08x}ul", 4)
		print_coeff_table("drgnt_u32", "DRGNT_FPART_COEFF_TABLE32", f_coeff_table32, lambda x: f"0x{x:08x}ul", 4)

		# Print float64 tables
		print_index_table("drgnt_rowptr", "DRGNT_IPART_INDEX_TABLE64", i_index_table64)
		print_index_table("drgnt_rowptr", "DRGNT_FPART_INDEX_TABLE64", f_index_table64)

		f.write("\n")

		print_coeff_table("drgnt_u64", "DRGNT_IPART_COEFF_TABLE64", i_coeff_table64, lambda x: f"0x{x:016x}ull", 4)
		print_coeff_table("drgnt_u64", "DRGNT_FPART_COEFF_TABLE64", f_coeff_table64, lambda x: f"0x{x:016x}ull", 4)

		f.write("\n")

		# Print float32 SRT table
		f.write(f"static const drgnt_u32 DRGNT_SRT_TABLE32[{len(srt_table32)}][4] =\n")
		f.write("{\n")
		
		for i in range(len(srt_table32)):
			coeffs = srt_table32[i]
			f.write(f"\t{{ 0x{coeffs[0]:08x}ul, 0x{coeffs[1]:08x}ul, 0x{coeffs[2]:08x}ul, 0x{coeffs[3]:08x}ul }}, /* {i + f32_min_exponent} */\n")

		f.write("};\n")

		# Print float64 SRT table
		f.write(f"static const drgnt_u64 DRGNT_SRT_TABLE64[{len(srt_table64)}][4] =\n")
		f.write("{\n")
		
		for i in range(len(srt_table64)):
			coeffs = srt_table64[i]
			f.write(f"\t{{ 0x{coeffs[0]:016x}ull, 0x{coeffs[1]:016x}ull, 0x{coeffs[2]:016x}ull, 0x{coeffs[3]:016x}ull }}, /* {i + f64_min_exponent} */\n")

		f.write("};\n")

	print("Done")

if __name__ == "__main__":
	print("Computing minimum k: ")
	print("  Float32: ", flush=True, end='')
	print(compute_min_k(f32_max_mantissa, f32_max_exponent, f32_min_exponent, f32_max_segments, f32_min_segments, f32_segment_length))

	print("  Float64: ", flush=True, end='')
	print(compute_min_k(f64_max_mantissa, f64_max_exponent, f64_min_exponent, f64_max_segments, f64_min_segments, f64_segment_length))

	print("  Float32 SRT: ", flush=True, end='')
	print(compute_min_k_srt(f32_max_mantissa, f32_max_exponent, f32_min_exponent))

	print("  Float64 SRT: ", flush=True, end='')
	print(compute_min_k_srt(f64_max_mantissa, f64_max_exponent, f64_min_exponent))
	print()


	print("Selected properties (float 32):")
	print(f"  Minimum k: {f32_min_k}")
	print(f"  Segment length: {f32_segment_length}")
	print(f"  Segment bits: {f32_segment_bit_count}")
	print()
	print("Selected properties (float 64):")
	print(f"  Minimum k: {f64_min_k}")
	print(f"  Segment length: {f64_segment_length}")
	print(f"  Segment bits: {f64_segment_bit_count}")
	print()

	generate_constants()