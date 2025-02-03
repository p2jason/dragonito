import table_gen as tg

def incchr(c):
	return chr(ord(c) + 1)

def decode_float(bits, m_bits, e_bits, e_bias):
	m_mask = (1 << m_bits) - 1
	e_mask = (1 << e_bits) - 1
	
	m_shift = 0
	e_shift = m_bits

	exponent = (bits >> e_shift) & e_mask
	mantissa = (bits >> m_shift) & m_mask

	if exponent == e_mask:
		raise Exception()

	if exponent != 0:
		m_unified = mantissa | (1 << m_bits)
		e_unified = exponent - e_bias - m_bits
	else:
		m_unified = mantissa
		e_unified = 1 - e_bias - m_bits
	
	if e_unified < 0:
		if e_unified >= -m_bits:
			m_int = m_unified >> -e_unified
			m_fract = m_unified & ((1 << -e_unified) - 1)
		else:
			m_int = 0
			m_fract = m_unified

		e_int = 0
		e_fract = -e_unified
	else:
		m_int = m_unified
		e_int = e_unified

		m_fract = 0
		e_fract = 0

	return (m_int, e_int, m_fract, e_fract)

def decode_f32(bits): return decode_float(bits, tg.f32_mantissa_bits, tg.f32_exponent_bits, tg.f32_exponent_bias)
def decode_f64(bits): return decode_float(bits, tg.f64_mantissa_bits, tg.f64_exponent_bits, tg.f64_exponent_bias)

def float_to_str(mant_and_exp):
	m_int, e_int, m_fract, e_fract = mant_and_exp

	i = 1
	while (m_fract * 10**i) % 2**e_fract != 0:
		i += 1

	istr = str(m_int << e_int)
	fstr = str(m_fract * 10**i >> e_fract).rjust(i, '0')

	return istr + fstr, len(istr)

def exp_split(s, i):
	if s == "00":
		return "00", 1, 0

	s0 = s.lstrip("0")
	e0 = (len(s0) - len(s)) if (i == 1 and s[0] == '0') else (i - 1)

	return (s0 if len(s0) > 1 else s0 + "0"), 1, e0

def apply_prec(s, i, e, prec):
	if prec < 0:
		return s, i, e

	round_idx = prec + i
	has_more = False

	if len(s) < round_idx:
		return s + "0" * (round_idx - len(s)), i, e

	while len(s) > round_idx + 1:
		has_more |= (s[-1] > '0')
		s = s[:-1]

	s = list(s)

	def round_up():
		nonlocal s
		nonlocal e
		nonlocal i

		for j in range(round_idx, 0, -1):
			c = s[j - 1]
			n = incchr(c)

			if c != '9':
				s[j - 1] = n
				return
			else:
				s[j - 1] = '0'

		s = ['1'] + s
		e += 1
		i += 1

	if len(s) > round_idx:
		if s[round_idx] > "5":
			round_up()
		elif s[round_idx] == "5" and has_more:
			round_up()
		elif s[round_idx] == "5" and (int(s[round_idx - 1]) % 2) == 1:
			round_up()

	# Recompute round index
	round_idx = prec + i
	s = ''.join(s[:round_idx])

	return s, i, e

def insert_dot(s, i):
	return s if len(s) <= i else (s[:i] + "." + s[i:])

def f2fix(mant_and_exp, prec=-1):
	s, i = float_to_str(mant_and_exp)
	s, i, e = apply_prec(s, i, 0, prec)

	return insert_dot(s, i)

def f2exp(mant_and_exp, prec=-1):
	s, i = float_to_str(mant_and_exp)
	s, i, e = exp_split(s, i)
	s, i, e = apply_prec(s, i, e, prec)
	
	i = 1
	p = 1 if prec < 0 else prec + 1
	s = s[:p] + s[p:].rstrip("0")

	if len(s) == i and prec != 0:
		s += "0"

	return insert_dot(s, i) + (f"e{e}" if e < 0 else f"e+{e}")

def write_line(f, s=""):
	f.write("\t")
	f.write(s)
	f.write("\n")

def write_f32_f2fix(f, bits, prec=-1): write_line(f, f"{{ 0x{bits:08x}ul, {prec:3}, false, \"{f2fix(decode_f32(bits), prec)}\" }},")
def write_f32_f2exp(f, bits, prec=-1): write_line(f, f"{{ 0x{bits:08x}ul, {prec:3},  true, \"{f2exp(decode_f32(bits), prec)}\" }},")
def write_f64_f2fix(f, bits, prec=-1): write_line(f, f"{{ 0x{bits:016x}ull, {prec:3}, false, \"{f2fix(decode_f64(bits), prec)}\" }},")
def write_f64_f2exp(f, bits, prec=-1): write_line(f, f"{{ 0x{bits:016x}ull, {prec:3},  true, \"{f2exp(decode_f64(bits), prec)}\" }},")

def write_f32_tests(f):
	write_line(f, "//Special cases (zero, inf, nan)")
	write_line(f, "{ 0x7f800000, -1, false, \"+inf\" },")
	write_line(f, "{ 0xff800000, -1, false, \"-inf\" },")
	write_line(f, "{ 0x7f800000, -1, true, \"+inf\" },")
	write_line(f, "{ 0xff800000, -1, true, \"-inf\" },")
	write_line(f)
	write_line(f, "{ 0x7f800001, -1, false, \"NaN\" },")
	write_line(f, "{ 0xff800001, -1, false, \"NaN\" },")
	write_line(f, "{ 0x7f800001, -1, true, \"NaN\" },")
	write_line(f, "{ 0xff800001, -1, true, \"NaN\" },")
	write_line(f)
	write_line(f, "{ 0x00000000, -1, false, \"0.0\" },")
	write_line(f, "{ 0x80000000, -1, false, \"-0.0\" },")
	write_line(f, "{ 0x00000000, -1, true, \"0.0e+0\" },")
	write_line(f, "{ 0x80000000, -1, true, \"-0.0e+0\" },")
	write_line(f)
	write_line(f, "{ 0x00000000,  0, false, \"0\" },")
	write_line(f, "{ 0x80000000,  0, false, \"-0\" },")
	write_line(f, "{ 0x00000000,  0, true, \"0e+0\" },")
	write_line(f, "{ 0x80000000,  0, true, \"-0e+0\" },")
	write_line(f)
	write_line(f, "{ 0x00000000,  1, false, \"0.0\" },")
	write_line(f, "{ 0x80000000,  1, false, \"-0.0\" },")
	write_line(f, "{ 0x00000000,  1, true, \"0.0e+0\" },")
	write_line(f, "{ 0x80000000,  1, true, \"-0.0e+0\" },")
	write_line(f)
	write_line(f, "{ 0x00000000,  2, false, \"0.00\" },")
	write_line(f, "{ 0x80000000,  2, false, \"-0.00\" },")
	write_line(f, "{ 0x00000000,  2, true, \"0.00e+0\" },")
	write_line(f, "{ 0x80000000,  2, true, \"-0.00e+0\" },")
	write_line(f)
	write_line(f, "{ 0x00000000,  3, false, \"0.000\" },")
	write_line(f, "{ 0x80000000,  3, false, \"-0.000\" },")
	write_line(f, "{ 0x00000000,  3, true, \"0.000e+0\" },")
	write_line(f, "{ 0x80000000,  3, true, \"-0.000e+0\" },")
	write_line(f)
	write_line(f)
	
	write_f32_f2fix(f, 0x7f7fffff, -1) # Max normal
	write_f32_f2fix(f, 0x00800000, -1) # Min normal
	write_f32_f2fix(f, 0x007fffff, -1) # Max subnormal
	write_f32_f2fix(f, 0x00000001, -1) # Min subnormal
	write_line(f)

	write_f32_f2exp(f, 0x7f7fffff, -1)
	write_f32_f2exp(f, 0x00800000, -1)
	write_f32_f2exp(f, 0x007fffff, -1)
	write_f32_f2exp(f, 0x00000001, -1)
	write_line(f)

	write_f32_f2fix(f, 0x7f7fffff, -2)
	write_f32_f2fix(f, 0x00800000, -2)
	write_f32_f2exp(f, 0x7f7fffff, -2)
	write_f32_f2exp(f, 0x00800000, -2)
	write_line(f)

	# ############################### #
	#            Tie Break            #
	# ############################### #
	write_line(f)

	write_f32_f2fix(f, 0x3f000000,  0) # 0.5
	write_f32_f2fix(f, 0x3fc00000,  0) # 1.5
	write_f32_f2fix(f, 0x40200000,  0) # 2.5
	write_f32_f2fix(f, 0x40600000,  0) # 3.5
	write_f32_f2fix(f, 0x40900000,  0) # 4.5
	write_f32_f2fix(f, 0x41180000,  0) # 9.5
	write_f32_f2fix(f, 0x41280000,  0) # 10.5
	write_line(f)
	write_f32_f2exp(f, 0x3f000000,  0) # 0.5
	write_f32_f2exp(f, 0x3fc00000,  0) # 1.5
	write_f32_f2exp(f, 0x40200000,  0) # 2.5
	write_f32_f2exp(f, 0x40600000,  0) # 3.5
	write_f32_f2exp(f, 0x40900000,  0) # 4.5
	write_f32_f2exp(f, 0x41180000,  0) # 9.5
	write_f32_f2exp(f, 0x41280000,  0) # 10.5
	write_line(f)
	write_f32_f2exp(f, 0x41700000,  0) # 15.0
	write_f32_f2exp(f, 0x41c80000,  0) # 25.0
	write_f32_f2exp(f, 0x420c0000,  0) # 35.0
	write_f32_f2exp(f, 0x42340000,  0) # 45.0
	write_f32_f2exp(f, 0x42be0000,  0) # 95.0
	write_f32_f2exp(f, 0x42d20000,  0) # 105.0
	write_line(f)
	write_f32_f2exp(f, 0x43160000,  0) # 150.0
	write_f32_f2exp(f, 0x437a0000,  0) # 250.0
	write_f32_f2exp(f, 0x43af0000,  0) # 350.0
	write_f32_f2exp(f, 0x43e10000,  0) # 450.0
	write_f32_f2exp(f, 0x446d8000,  0) # 950.0
	write_f32_f2exp(f, 0x44834000,  0) # 1050.0
	write_line(f)
	write_f32_f2fix(f, 0x3ec00000,  2) # 0.375
	write_f32_f2exp(f, 0x3ec00000,  1) # 0.375
	write_f32_f2fix(f, 0x3d800000,  3) # 0.0625
	write_f32_f2exp(f, 0x3d800000,  1) # 0.0625
	write_line(f)

	# ############################### #
	#          Round Cascade          #
	# ############################### #
	write_line(f)

	# 1.99999988079071044921875
	write_f32_f2fix(f, 0x3fffffff, 8)
	write_f32_f2fix(f, 0x3fffffff, 7)
	write_f32_f2fix(f, 0x3fffffff, 6)
	write_f32_f2exp(f, 0x3fffffff, 8)
	write_f32_f2exp(f, 0x3fffffff, 7)
	write_f32_f2exp(f, 0x3fffffff, 6)
	write_f32_f2exp(f, 0x3fffffff, 0)
	write_line(f)

	# 19.9999980926513671875
	write_f32_f2fix(f, 0x419fffff, 6)
	write_f32_f2fix(f, 0x419fffff, 5)
	write_f32_f2exp(f, 0x419fffff, 7)
	write_f32_f2exp(f, 0x419fffff, 6)
	write_f32_f2exp(f, 0x419fffff, 0)
	write_line(f)

	# 19999980.0
	write_f32_f2fix(f, 0x4b989676, 0)
	write_f32_f2exp(f, 0x4b989676, 6)
	write_f32_f2exp(f, 0x4b989676, 5)
	write_f32_f2exp(f, 0x4b989676, 0)
	write_line(f)

	# 9999996.0
	write_f32_f2exp(f, 0x4b18967c, 5)
	write_f32_f2exp(f, 0x4b18967c, 0)
	write_line(f)

	write_f32_f2fix(f, 0x00c00000, 126)
	write_f32_f2exp(f, 0x00c00000, 88)
	write_line(f)

	# ############################## #
	#            Round Up            #
	# ############################## #
	write_line(f)

	write_f32_f2exp(f, 0x41880000, 0) # 17.0
	write_f32_f2exp(f, 0x3fcccccd, 1) # 1.60000002384185791015625
	write_f32_f2exp(f, 0x3fcccccd, 0) # 1.60000002384185791015625
	write_f32_f2exp(f, 0x35880000, 10) # 0.000001013278961181640625
	write_f32_f2fix(f, 0x42f73333, 0) # 123.59999847412109375
	write_f32_f2fix(f, 0x42f73333, 1) # 123.59999847412109375
	write_f32_f2fix(f, 0x42f73334, 0) # 123.600006103515625
	write_line(f)

	# ############################## #
	#           Round Down           #
	# ############################## #
	write_line(f)

	write_f32_f2exp(f, 0x41600000, 0) # 14.0
	write_f32_f2exp(f, 0x3fb33334, 0) # 1.400000095367431640625
	write_f32_f2exp(f, 0x3fb33334, 1) # 1.400000095367431640625
	write_f32_f2exp(f, 0x3912ccf7, 5) # 0.000140000003739260137081146240234375
	write_f32_f2fix(f, 0x42f6cccd, 0) # 123.40000152587890625
	write_f32_f2fix(f, 0x3dcccccd, 0) # 0.100000001490116119384765625
	write_line(f)

	# ############################### #
	#           Pow10 Round           #
	# ############################### #
	write_line(f)

	write_f32_f2exp(f, 0x41c80000, 0) # 25.0
	write_f32_f2exp(f, 0x437a0000, 0) # 250.0
	write_f32_f2exp(f, 0x451c4000, 0) # 2500.0
	write_f32_f2exp(f, 0x46c35000, 0) # 25000.0
	write_f32_f2exp(f, 0x48742400, 0) # 250000.0
	write_f32_f2exp(f, 0x4a189680, 0) # 2500000.0
	write_f32_f2exp(f, 0x4bbebc20, 0) # 25000000.0
	write_f32_f2exp(f, 0x4d6e6b28, 0) # 250000000.0
	write_f32_f2exp(f, 0x4f1502f9, 0) # 2500000000.0
	write_line(f)

	write_f32_f2exp(f, 0x437b0000, 0) # 251.0
	write_f32_f2exp(f, 0x451c5000, 0) # 2501.0
	write_f32_f2exp(f, 0x46c35200, 0) # 25001.0
	write_f32_f2exp(f, 0x48742400, 0) # 250001.0
	write_f32_f2exp(f, 0x4a189684, 0) # 2500001.0
	write_f32_f2exp(f, 0x4bbebc21, 0) # 25000002.0
	write_f32_f2exp(f, 0x4d6e6b29, 0) # 250000016.0
	write_f32_f2exp(f, 0x4f1502fa, 0) # 2500000256.0
	write_f32_f2exp(f, 0x50ba43b8, 0) # 25000001536.0
	write_f32_f2exp(f, 0x5268d4a6, 0) # 250000015360.0
	write_f32_f2exp(f, 0x541184e8, 0) # 2500000219136.0
	write_f32_f2exp(f, 0x55b5e621, 0) # 25000000094208.0
	write_f32_f2exp(f, 0x57635faa, 0) # 250000013524992.0
	write_f32_f2exp(f, 0x590e1bca, 0) # 2500000068141056.0
	write_f32_f2exp(f, 0x5ab1a2bd, 0) # 25000001755152384.0
	write_line(f)

	# ############################### #
	#           Value Tests           #
	# ############################### #
	write_line(f)

	values = [
		# All the powers of ten that fit within a 32bit float
		0x00000007, 0x00000047, 0x000002ca, 0x00001be0, 0x000116c2, 0x000ae398, 0x006ce3ee, 0x02081cea, 0x03aa2425, 0x0554ad2e, 0x0704ec3d, 0x08a6274c, 0x0a4fb11f, 0x0c01ceb3,
		0x0da24260, 0x0f4ad2f8, 0x10fd87b6, 0x129e74d2, 0x14461206, 0x15f79688, 0x179abe15, 0x19416d9a, 0x1af1c901, 0x1c971da0, 0x1e3ce508, 0x1fec1e4a, 0x219392ef, 0x233877aa, 
		0x24e69595, 0x26901d7d, 0x283424dc, 0x29e12e13, 0x2b8cbccc, 0x2d2febff, 0x2edbe6ff, 0x3089705f, 0x322bcc77, 0x33d6bf95, 0x358637bd, 0x3727c5ac, 0x38d1b717, 0x3a83126f, 
		0x3c23d70a, 0x3dcccccd, 0x3f800000, 0x41200000, 0x42c80000, 0x447a0000, 0x461c4000, 0x47c35000, 0x49742400, 0x4b189680, 0x4cbebc20, 0x4e6e6b28, 0x501502f9, 0x51ba43b7, 
		0x5368d4a5, 0x551184e7, 0x56b5e621, 0x58635fa9, 0x5a0e1bca, 0x5bb1a2bc, 0x5d5e0b6b, 0x5f0ac723, 0x60ad78ec, 0x6258d727, 0x64078678, 0x65a96816, 0x6753c21c, 0x69045951, 
		0x6aa56fa6, 0x6c4ecb8f, 0x6e013f39, 0x6fa18f08, 0x7149f2ca, 0x72fc6f7c, 0x749dc5ae, 0x76453719, 0x77f684df, 0x799a130c, 0x7b4097ce, 0x7cf0bdc2, 0x7e967699, 
	]

	# Min/max of all powers of two
	values.extend([ (i << 23) | 0x000000 for i in range(255) ])
	values.extend([ (i << 23) | 0x7fffff for i in range(255) ])

	# Print the damn things
	for p in values: write_f32_f2fix(f, p, -1)
	write_line(f)
	for p in values: write_f32_f2exp(f, p, -1)

def write_f64_tests(f):
	write_line(f, "//Special cases (zero, inf, nan)")
	write_line(f, "{ 0x7ff0000000000000, -1, false, \"+inf\" },")
	write_line(f, "{ 0xfff0000000000000, -1, false, \"-inf\" },")
	write_line(f, "{ 0x7ff0000000000000, -1, true, \"+inf\" },")
	write_line(f, "{ 0xfff0000000000000, -1, true, \"-inf\" },")
	write_line(f)
	write_line(f, "{ 0x7ff8000000000001, -1, false, \"NaN\" },")
	write_line(f, "{ 0xfff8000000000001, -1, false, \"NaN\" },")
	write_line(f, "{ 0x7ff8000000000001, -1, true, \"NaN\" },")
	write_line(f, "{ 0xfff8000000000001, -1, true, \"NaN\" },")
	write_line(f)
	write_line(f, "{ 0x0000000000000000, -1, false, \"0.0\" },")
	write_line(f, "{ 0x8000000000000000, -1, false, \"-0.0\" },")
	write_line(f, "{ 0x0000000000000000, -1, true, \"0.0e+0\" },")
	write_line(f, "{ 0x8000000000000000, -1, true, \"-0.0e+0\" },")
	write_line(f)
	write_line(f)

	write_f64_f2fix(f, 0x7fefffffffffffff, -1) # Max normal
	write_f64_f2fix(f, 0x0010000000000000, -1) # Min normal
	write_f64_f2fix(f, 0x000fffffffffffff, -1) # Max subnormal
	write_f64_f2fix(f, 0x0000000000000001, -1) # Min subnormal
	write_line(f)

	write_f64_f2exp(f, 0x7fefffffffffffff, -1)
	write_f64_f2exp(f, 0x0010000000000000, -1)
	write_f64_f2exp(f, 0x000fffffffffffff, -1)
	write_f64_f2exp(f, 0x0000000000000001, -1)
	write_line(f)

	# ############################### #
	#           Pow10 Round           #
	# ############################### #
	write_line(f)

	write_f64_f2exp(f, 0x4039000000000000, 0) # 25.0
	write_f64_f2exp(f, 0x406f400000000000, 0) # 250.0
	write_f64_f2exp(f, 0x40a3880000000000, 0) # 2500.0
	write_f64_f2exp(f, 0x40d86a0000000000, 0) # 25000.0
	write_f64_f2exp(f, 0x410e848000000000, 0) # 250000.0
	write_f64_f2exp(f, 0x414312d000000000, 0) # 2500000.0
	write_f64_f2exp(f, 0x4177d78400000000, 0) # 25000000.0
	write_f64_f2exp(f, 0x41adcd6500000000, 0) # 250000000.0
	write_f64_f2exp(f, 0x41e2a05f20000000, 0) # 2500000000.0
	write_f64_f2exp(f, 0x42174876e8000000, 0) # 25000000000.0
	write_f64_f2exp(f, 0x424d1a94a2000000, 0) # 250000000000.0
	write_f64_f2exp(f, 0x4282309ce5400000, 0) # 2500000000000.0
	write_f64_f2exp(f, 0x42b6bcc41e900000, 0) # 25000000000000.0
	write_f64_f2exp(f, 0x42ec6bf526340000, 0) # 250000000000000.0
	write_f64_f2exp(f, 0x4321c37937e08000, 0) # 2500000000000000.0
	write_f64_f2exp(f, 0x4356345785d8a000, 0) # 25000000000000000.0
	write_f64_f2exp(f, 0x438bc16d674ec800, 0) # 250000000000000000.0
	write_f64_f2exp(f, 0x43c158e460913d00, 0) # 2500000000000000000.0
	write_f64_f2exp(f, 0x43f5af1d78b58c40, 0) # 25000000000000000000.0
	write_f64_f2exp(f, 0x442b1ae4d6e2ef50, 0) # 250000000000000000000.0
	write_f64_f2exp(f, 0x4460f0cf064dd592, 0) # 2500000000000000000000.0
	write_f64_f2exp(f, 0x44952d02c7e14af6, 0) # 25000000000000000000000.0
	write_line(f)

	write_f64_f2exp(f, 0x406f600000000000, 0) # 251.0
	write_f64_f2exp(f, 0x40a38a0000000000, 0) # 2501.0
	write_f64_f2exp(f, 0x40d86a4000000000, 0) # 25001.0
	write_f64_f2exp(f, 0x410e848800000000, 0) # 250001.0
	write_f64_f2exp(f, 0x414312d080000000, 0) # 2500001.0
	write_f64_f2exp(f, 0x4177d78410000000, 0) # 25000001.0
	write_f64_f2exp(f, 0x41adcd6502000000, 0) # 250000001.0
	write_f64_f2exp(f, 0x41e2a05f20200000, 0) # 2500000001.0
	write_f64_f2exp(f, 0x42174876e8040000, 0) # 25000000001.0
	write_f64_f2exp(f, 0x424d1a94a2008000, 0) # 250000000001.0
	write_f64_f2exp(f, 0x4282309ce5400800, 0) # 2500000000001.0
	write_f64_f2exp(f, 0x42b6bcc41e900100, 0) # 25000000000001.0
	write_f64_f2exp(f, 0x42ec6bf526340020, 0) # 250000000000001.0
	write_f64_f2exp(f, 0x4321c37937e08002, 0) # 2500000000000001.0
	write_f64_f2exp(f, 0x4356345785d8a001, 0) # 25000000000000004.0
	write_f64_f2exp(f, 0x438bc16d674ec801, 0) # 250000000000000032.0
	write_f64_f2exp(f, 0x43c158e460913d01, 0) # 2500000000000000512.0
	write_f64_f2exp(f, 0x43f5af1d78b58c41, 0) # 25000000000000004096.0
	write_f64_f2exp(f, 0x442b1ae4d6e2ef51, 0) # 250000000000000032768.0
	write_f64_f2exp(f, 0x4460f0cf064dd593, 0) # 2500000000000000524288.0
	write_f64_f2exp(f, 0x44952d02c7e14af7, 0) # 25000000000000002097152.0
	write_f64_f2exp(f, 0x44ca784379d99db5, 0) # 250000000000000029360128.0
	write_line(f)

	# ############################### #
	#           Value Tests           #
	# ############################### #
	write_line(f)

	values = [
		# All the powers of ten that fit within a 64bit float
		0x0000000000000002, 0x0000000000000014, 0x00000000000000ca, 0x00000000000007e8, 0x0000000000004f10, 0x00000000000316a2, 0x00000000001ee257, 0x000000000134d761, 0x000000000c1069cd,
		0x0000000078a42205, 0x00000004b6695433, 0x0000002f201d49fb, 0x000001d74124e3d1, 0x000012688b70e62b, 0x0000b8157268fdaf, 0x000730d67819e8d2, 0x0031fa182c40c60d, 0x0066789e3750f791,
		0x009c16c5c5253575, 0x00d18e3b9b374169, 0x0105f1ca820511c3, 0x013b6e3d22865634, 0x017124e63593f5e1, 0x01a56e1fc2f8f359, 0x01dac9a7b3b7302f, 0x0210be08d0527e1d, 0x0244ed8b04671da5,
		0x027a28edc580e50e, 0x02b059949b708f29, 0x02e46ff9c24cb2f3, 0x03198bf832dfdfb0, 0x034feef63f97d79c, 0x0383f559e7bee6c1, 0x03b8f2b061aea072, 0x03ef2f5c7a1a488e, 0x04237d99cc506d59,
		0x04585d003f6488af, 0x048e74404f3daadb, 0x04c308a831868ac9, 0x04f7cad23de82d7b, 0x052dbd86cd6238d9, 0x05629674405d6388, 0x05973c115074bc6a, 0x05cd0b15a491eb84, 0x060226ed86db3333,
		0x0636b0a8e891ffff, 0x066c5cd322b67fff, 0x06a1ba03f5b21000, 0x06d62884f31e93ff, 0x070bb2a62fe638ff, 0x07414fa7ddefe3a0, 0x0775a391d56bdc87, 0x07ab0c764ac6d3a9, 0x07e0e7c9eebc444a,
		0x081521bc6a6b555c, 0x084a6a2b85062ab3, 0x0880825b3323dab0, 0x08b4a2f1ffecd15c, 0x08e9cbae7fe805b3, 0x09201f4d0ff10390, 0x0954272053ed4474, 0x098930e868e89591, 0x09bf7d228322baf5,
		0x09f3ae3591f5b4d9, 0x0a2899c2f6732210, 0x0a5ec033b40fea93, 0x0a9338205089f29c, 0x0ac8062864ac6f43, 0x0afe07b27dd78b14, 0x0b32c4cf8ea6b6ec, 0x0b677603725064a8, 0x0b9d53844ee47dd1,
		0x0bd25432b14ecea3, 0x0c06e93f5da2824c, 0x0c3ca38f350b22df, 0x0c71e6398126f5cb, 0x0ca65fc7e170b33e, 0x0cdbf7b9d9cce00d, 0x0d117ad428200c08, 0x0d45d98932280f0a, 0x0d7b4feb7eb212cd,
		0x0db111f32f2f4bc0, 0x0de5566ffafb1eb0, 0x0e1aac0bf9b9e65c, 0x0e50ab877c142ffa, 0x0e84d6695b193bf8, 0x0eba0c03b1df8af6, 0x0ef047824f2bb6da, 0x0f245962e2f6a490, 0x0f596fbb9bb44db4,
		0x0f8fcbaa82a16121, 0x0fc3df4a91a4dcb5, 0x0ff8d71d360e13e2, 0x102f0ce4839198db, 0x1063680ed23aff89, 0x1098421286c9bf6b, 0x10ce5297287c2f45, 0x1102f39e794d9d8b, 0x1137b08617a104ee,
		0x116d9ca79d89462a, 0x11a281e8c275cbda, 0x11d72262f3133ed1, 0x120ceafbafd80e85, 0x124212dd4de70913, 0x12769794a160cb58, 0x12ac3d79c9b8fe2e, 0x12e1a66c1e139edd, 0x1316100725988694,
		0x134b9408eefea839, 0x13813c85955f2923, 0x13b58ba6fab6f36c, 0x13eaee90b964b047, 0x1420d51a73deee2d, 0x14550a6110d6a9b8, 0x148a4cf9550c5426, 0x14c0701bd527b498, 0x14f48c22ca71a1bd,
		0x1529af2b7d0e0a2d, 0x15600d7b2e28c65c, 0x159410d9f9b2f7f3, 0x15c91510781fb5f0, 0x15ff5a549627a36c, 0x16339874ddd8c623, 0x16687e92154ef7ac, 0x169e9e369aa2b597, 0x16d322e220a5b17e,
		0x1707eb9aa8cf1dde, 0x173de6815302e556, 0x1772b010d3e1cf56, 0x17a75c1508da432b, 0x17dd331a4b10d3f6, 0x18123ff06eea847a, 0x1846cfec8aa52598, 0x187c83e7ad4e6efe, 0x18b1d270cc51055f,
		0x18e6470cff6546b6, 0x191bd8d03f3e9864, 0x1951678227871f3e, 0x1985c162b168e70e, 0x19bb31bb5dc320d2, 0x19f0ff151a99f483, 0x1a253eda614071a4, 0x1a5a8e90f9908e0d, 0x1a90991a9bfa58c8,
		0x1ac4bf6142f8eefa, 0x1af9ef3993b72ab8, 0x1b303583fc527ab3, 0x1b6442e4fb671960, 0x1b99539e3a40dfb8, 0x1bcfa885c8d117a6, 0x1c03c9539d82aec8, 0x1c38bba884e35a7a, 0x1c6eea92a61c3118,
		0x1ca3529ba7d19eaf, 0x1cd8274291c6065b, 0x1d0e3113363787f2, 0x1d42deac01e2b4f7, 0x1d779657025b6235, 0x1dad7becc2f23ac2, 0x1de26d73f9d764b9, 0x1e1708d0f84d3de7, 0x1e4ccb0536608d61,
		0x1e81fee341fc585d, 0x1eb67e9c127b6e74, 0x1eec1e43171a4a11, 0x1f2192e9ee706e4b, 0x1f55f7a46a0c89dd, 0x1f8b758d848fac55, 0x1fc1297872d9cbb5, 0x1ff573d68f903ea2, 0x202ad0cc33744e4b,
		0x2060c27fa028b0ef, 0x2094f31f8832dd2a, 0x20ca2fe76a3f9475, 0x21005df0a267bcc9, 0x2134756ccb01abfb, 0x216992c7fdc216fa, 0x219ff779fd329cb9, 0x21d3faac3e3fa1f3, 0x2208f9574dcf8a70,
		0x223f37ad21436d0c, 0x227382cc34ca2428, 0x22a8637f41fcad32, 0x22de7c5f127bd87e, 0x23130dbb6b8d674f, 0x2347d12a4670c123, 0x237dc574d80cf16b, 0x23b29b69070816e3, 0x23e7424348ca1c9c,
		0x241d12d41afca3c3, 0x24522bc490dde65a, 0x2486b6b5b5155ff0, 0x24bc6463225ab7ec, 0x24f1bebdf578b2f4, 0x25262e6d72d6dfb0, 0x255bba08cf8c979d, 0x2591544581b7dec2, 0x25c5a956e225d672,
		0x25fb13ac9aaf4c0f, 0x2630ec4be0ad8f89, 0x2665275ed8d8f36c, 0x269a71368f0f3047, 0x26d086c219697e2c, 0x2704a8729fc3ddb7, 0x2739d28f47b4d525, 0x277023998cd10537, 0x27a42c7ff0054685,
		0x27d9379fec069826, 0x280f8587e7083e30, 0x2843b374f06526de, 0x2878a0522c7e7095, 0x28aec866b79e0cba, 0x28e33d4032c2c7f5, 0x29180c903f7379f2, 0x294e0fb44f50586e, 0x2982c9d0b1923745,
		0x29b77c44ddf6c516, 0x29ed5b561574765b, 0x2a225915cd68c9f9, 0x2a56ef5b40c2fc77, 0x2a8cab3210f3bb95, 0x2ac1eaff4a98553d, 0x2af665bf1d3e6a8d, 0x2b2bff2ee48e0530, 0x2b617f7d4ed8c33e,
		0x2b95df5ca28ef40d, 0x2bcb5733cb32b111, 0x2c0116805effaeaa, 0x2c355c2076bf9a55, 0x2c6ab328946f80ea, 0x2ca0aff95cc5b092, 0x2cd4dbf7b3f71cb7, 0x2d0a12f5a0f4e3e5, 0x2d404bd984990e6f,
		0x2d745ecfe5bf520b, 0x2da97683df2f268d, 0x2ddfd424d6faf031, 0x2e13e497065cd61f, 0x2e48ddbcc7f40ba6, 0x2e7f152bf9f10e90, 0x2eb36d3b7c36a91a, 0x2ee8488a5b445360, 0x2f1e5aacf2156838,
		0x2f52f8ac174d6123, 0x2f87b6d71d20b96c, 0x2fbda48ce468e7c7, 0x2ff286d80ec190dc, 0x3027288e1271f513, 0x305cf2b1970e7258, 0x309217aefe690777, 0x30c69d9abe034955, 0x30fc45016d841baa,
		0x3131ab20e472914a, 0x316615e91d8f359d, 0x319b9b6364f30304, 0x31d1411e1f17e1e3, 0x32059165a6ddda5b, 0x323af5bf109550f2, 0x3270d9976a5d5297, 0x32a50ffd44f4a73d, 0x32da53fc9631d10d,
		0x3310747ddddf22a8, 0x3344919d5556eb52, 0x3379b604aaaca626, 0x33b011c2eaabe7d8, 0x33e41633a556e1ce, 0x34191bc08eac9a41, 0x344f62b0b257c0d2, 0x34839dae6f76d883, 0x34b8851a0b548ea4,
		0x34eea6608e29b24d, 0x352327fc58da0f70, 0x3557f1fb6f10934c, 0x358dee7a4ad4b81f, 0x35c2b50c6ec4f313, 0x35f7624f8a762fd8, 0x362d3ae36d13bbce, 0x366244ce242c5561, 0x3696d601ad376ab9,
		0x36cc8b8218854567, 0x3701d7314f534b61, 0x37364cfda3281e39, 0x376be03d0bf225c7, 0x37a16c262777579c, 0x37d5c72fb1552d83, 0x380b38fb9daa78e4, 0x3841039d428a8b8f, 0x38754484932d2e72,
		0x38aa95a5b7f87a0f, 0x38e09d8792fb4c49, 0x3914c4e977ba1f5c, 0x3949f623d5a8a733, 0x398039d665896880, 0x39b4484bfeebc2a0, 0x39e95a5efea6b347, 0x3a1fb0f6be506019, 0x3a53ce9a36f23c10,
		0x3a88c240c4aecb14, 0x3abef2d0f5da7dd9, 0x3af357c299a88ea7, 0x3b282db34012b251, 0x3b5e392010175ee6, 0x3b92e3b40a0e9b4f, 0x3bc79ca10c924223, 0x3bfd83c94fb6d2ac, 0x3c32725dd1d243ac,
		0x3c670ef54646d497, 0x3c9cd2b297d889bc, 0x3cd203af9ee75616, 0x3d06849b86a12b9b, 0x3d3c25c268497682, 0x3d719799812dea11, 0x3da5fd7fe1796495, 0x3ddb7cdfd9d7bdbb, 0x3e112e0be826d695,
		0x3e45798ee2308c3a, 0x3e7ad7f29abcaf48, 0x3eb0c6f7a0b5ed8d, 0x3ee4f8b588e368f1, 0x3f1a36e2eb1c432d, 0x3f50624dd2f1a9fc, 0x3f847ae147ae147b, 0x3fb999999999999a, 0x3ff0000000000000,
		0x4024000000000000, 0x4059000000000000, 0x408f400000000000, 0x40c3880000000000, 0x40f86a0000000000, 0x412e848000000000, 0x416312d000000000, 0x4197d78400000000, 0x41cdcd6500000000,
		0x4202a05f20000000, 0x42374876e8000000, 0x426d1a94a2000000, 0x42a2309ce5400000, 0x42d6bcc41e900000, 0x430c6bf526340000, 0x4341c37937e08000, 0x4376345785d8a000, 0x43abc16d674ec800,
		0x43e158e460913d00, 0x4415af1d78b58c40, 0x444b1ae4d6e2ef50, 0x4480f0cf064dd592, 0x44b52d02c7e14af6, 0x44ea784379d99db4, 0x45208b2a2c280291, 0x4554adf4b7320335, 0x4589d971e4fe8402,
		0x45c027e72f1f1281, 0x45f431e0fae6d721, 0x46293e5939a08cea, 0x465f8def8808b024, 0x4693b8b5b5056e17, 0x46c8a6e32246c99c, 0x46fed09bead87c03, 0x4733426172c74d82, 0x476812f9cf7920e3,
		0x479e17b84357691b, 0x47d2ced32a16a1b1, 0x48078287f49c4a1d, 0x483d6329f1c35ca5, 0x48725dfa371a19e7, 0x48a6f578c4e0a061, 0x48dcb2d6f618c879, 0x4911efc659cf7d4c, 0x49466bb7f0435c9e,
		0x497c06a5ec5433c6, 0x49b18427b3b4a05c, 0x49e5e531a0a1c873, 0x4a1b5e7e08ca3a8f, 0x4a511b0ec57e649a, 0x4a8561d276ddfdc0, 0x4ababa4714957d30, 0x4af0b46c6cdd6e3e, 0x4b24e1878814c9ce,
		0x4b5a19e96a19fc41, 0x4b905031e2503da9, 0x4bc4643e5ae44d13, 0x4bf97d4df19d6057, 0x4c2fdca16e04b86d, 0x4c63e9e4e4c2f344, 0x4c98e45e1df3b015, 0x4ccf1d75a5709c1b, 0x4d03726987666191,
		0x4d384f03e93ff9f5, 0x4d6e62c4e38ff872, 0x4da2fdbb0e39fb47, 0x4dd7bd29d1c87a19, 0x4e0dac74463a989f, 0x4e428bc8abe49f64, 0x4e772ebad6ddc73d, 0x4eacfa698c95390c, 0x4ee21c81f7dd43a7,
		0x4f16a3a275d49491, 0x4f4c4c8b1349b9b5, 0x4f81afd6ec0e1411, 0x4fb61bcca7119916, 0x4feba2bfd0d5ff5b, 0x502145b7e285bf99, 0x50559725db272f7f, 0x508afcef51f0fb5f, 0x50c0de1593369d1b,
		0x50f5159af8044462, 0x512a5b01b605557b, 0x516078e111c3556d, 0x5194971956342ac8, 0x51c9bcdfabc1357a, 0x5200160bcb58c16c, 0x52341b8ebe2ef1c7, 0x526922726dbaae39, 0x529f6b0f092959c7,
		0x52d3a2e965b9d81d, 0x53088ba3bf284e24, 0x533eae8caef261ad, 0x53732d17ed577d0c, 0x53a7f85de8ad5c4f, 0x53ddf67562d8b363, 0x5412ba095dc7701e, 0x5447688bb5394c25, 0x547d42aea2879f2e,
		0x54b249ad2594c37d, 0x54e6dc186ef9f45c, 0x551c931e8ab87173, 0x5551dbf316b346e8, 0x558652efdc6018a2, 0x55bbe7abd3781eca, 0x55f170cb642b133f, 0x5625ccfe3d35d80e, 0x565b403dcc834e12,
		0x569108269fd210cb, 0x56c54a3047c694fe, 0x56fa9cbc59b83a3d, 0x5730a1f5b8132466, 0x5764ca732617ed80, 0x5799fd0fef9de8e0, 0x57d03e29f5c2b18c, 0x58044db473335def, 0x583961219000356b,
		0x586fb969f40042c5, 0x58a3d3e2388029bb, 0x58d8c8dac6a0342a, 0x590efb1178484135, 0x59435ceaeb2d28c1, 0x59783425a5f872f1, 0x59ae412f0f768fad, 0x59e2e8bd69aa19cc, 0x5a17a2ecc414a03f,
		0x5a4d8ba7f519c84f, 0x5a827748f9301d32, 0x5ab7151b377c247e, 0x5aecda62055b2d9e, 0x5b22087d4358fc82, 0x5b568a9c942f3ba3, 0x5b8c2d43b93b0a8c, 0x5bc19c4a53c4e697, 0x5bf6035ce8b6203d,
		0x5c2b843422e3a84d, 0x5c6132a095ce4930, 0x5c957f48bb41db7c, 0x5ccadf1aea12525b, 0x5d00cb70d24b7379, 0x5d34fe4d06de5057, 0x5d6a3de04895e46d, 0x5da066ac2d5daec4, 0x5dd4805738b51a75,
		0x5e09a06d06e26112, 0x5e400444244d7cab, 0x5e7405552d60dbd6, 0x5ea906aa78b912cc, 0x5edf485516e7577f, 0x5f138d352e5096af, 0x5f48708279e4bc5b, 0x5f7e8ca3185deb72, 0x5fb317e5ef3ab327,
		0x5fe7dddf6b095ff1, 0x601dd55745cbb7ed, 0x6052a5568b9f52f4, 0x60874eac2e8727b1, 0x60bd22573a28f19d, 0x60f2357684599702, 0x6126c2d4256ffcc3, 0x615c73892ecbfbf4, 0x6191c835bd3f7d78,
		0x61c63a432c8f5cd6, 0x61fbc8d3f7b3340c, 0x62315d847ad00087, 0x6265b4e5998400a9, 0x629b221effe500d4, 0x62d0f5535fef2084, 0x630532a837eae8a5, 0x633a7f5245e5a2cf, 0x63708f936baf85c1,
		0x63a4b378469b6732, 0x63d9e056584240fe, 0x64102c35f729689f, 0x6444374374f3c2c6, 0x647945145230b378, 0x64af965966bce056, 0x64e3bdf7e0360c36, 0x6518ad75d8438f43, 0x654ed8d34e547314,
		0x6583478410f4c7ec, 0x65b819651531f9e8, 0x65ee1fbe5a7e7861, 0x6622d3d6f88f0b3d, 0x665788ccb6b2ce0c, 0x668d6affe45f818f, 0x66c262dfeebbb0f9, 0x66f6fb97ea6a9d38, 0x672cba7de5054486,
		0x6761f48eaf234ad4, 0x679671b25aec1d89, 0x67cc0e1ef1a724eb, 0x680188d357087713, 0x6835eb082cca94d7, 0x686b65ca37fd3a0d, 0x68a11f9e62fe4448, 0x68d56785fbbdd55a, 0x690ac1677aad4ab1,
		0x6940b8e0acac4eaf, 0x6974e718d7d7625a, 0x69aa20df0dcd3af1, 0x69e0548b68a044d6, 0x6a1469ae42c8560c, 0x6a498419d37a6b8f, 0x6a7fe52048590673, 0x6ab3ef342d37a408, 0x6ae8eb0138858d0a,
		0x6b1f25c186a6f04c, 0x6b537798f4285630, 0x6b88557f31326bbb, 0x6bbe6adefd7f06aa, 0x6bf302cb5e6f642a, 0x6c27c37e360b3d35, 0x6c5db45dc38e0c82, 0x6c9290ba9a38c7d1, 0x6cc734e940c6f9c6,
		0x6cfd022390f8b837, 0x6d3221563a9b7323, 0x6d66a9abc9424feb, 0x6d9c5416bb92e3e6, 0x6dd1b48e353bce70, 0x6e0621b1c28ac20c, 0x6e3baa1e332d728f, 0x6e714a52dffc6799, 0x6ea59ce797fb817f,
		0x6edb04217dfa61df, 0x6f10e294eebc7d2c, 0x6f451b3a2a6b9c76, 0x6f7a6208b5068394, 0x6fb07d457124123d, 0x6fe49c96cd6d16cc, 0x7019c3bc80c85c7f, 0x70501a55d07d39cf, 0x708420eb449c8843,
		0x70b9292615c3aa54, 0x70ef736f9b3494e9, 0x7123a825c100dd11, 0x7158922f31411456, 0x718eb6bafd91596b, 0x71c33234de7ad7e3, 0x71f7fec216198ddc, 0x722dfe729b9ff153, 0x7262bf07a143f6d4,
		0x72976ec98994f489, 0x72cd4a7bebfa31ab, 0x73024e8d737c5f0b, 0x7336e230d05b76cd, 0x736c9abd04725481, 0x73a1e0b622c774d0, 0x73d658e3ab795204, 0x740bef1c9657a686, 0x74417571ddf6c814,
		0x7475d2ce55747a18, 0x74ab4781ead1989e, 0x74e10cb132c2ff63, 0x75154fdd7f73bf3c, 0x754aa3d4df50af0b, 0x7580a6650b926d67, 0x75b4cffe4e7708c0, 0x75ea03fde214caf1, 0x7620427ead4cfed6,
		0x7654531e58a03e8c, 0x768967e5eec84e2f, 0x76bfc1df6a7a61bb, 0x76f3d92ba28c7d15, 0x7728cf768b2f9c5a, 0x775f03542dfb8370, 0x779362149cbd3226, 0x77c83a99c3ec7eb0, 0x77fe494034e79e5c,
		0x7832edc82110c2f9, 0x7867a93a2954f3b8, 0x789d9388b3aa30a5, 0x78d27c35704a5e67, 0x79071b42cc5cf601, 0x793ce2137f743382, 0x79720d4c2fa8a031, 0x79a6909f3b92c83d, 0x79dc34c70a777a4d,
		0x7a11a0fc668aac70, 0x7a46093b802d578c, 0x7a7b8b8a6038ad6f, 0x7ab137367c236c65, 0x7ae585041b2c477f, 0x7b1ae64521f7595e, 0x7b50cfeb353a97db, 0x7b8503e602893dd2, 0x7bba44df832b8d46,
		0x7bf06b0bb1fb384c, 0x7c2485ce9e7a065f, 0x7c59a742461887f6, 0x7c9008896bcf54fa, 0x7cc40aabc6c32a38, 0x7cf90d56b873f4c7, 0x7d2f50ac6690f1f8, 0x7d63926bc01a973b, 0x7d987706b0213d0a,
		0x7dce94c85c298c4c, 0x7e031cfd3999f7b0, 0x7e37e43c8800759c, 0x7e6ddd4baa009303, 0x7ea2aa4f4a405be2, 0x7ed754e31cd072da, 0x7f0d2a1be4048f90, 0x7f423a516e82d9ba, 0x7f76c8e5ca239029,
		0x7fac7b1f3cac7433, 0x7fe1ccf385ebc8a0, 
	]

	# Min/max of all powers of two
	values.extend([ (i << 23) | 0x000000 for i in range(2048) ])
	values.extend([ (i << 23) | 0x7fffff for i in range(2048) ])

	# Print the damn things
	for p in values: write_f64_f2fix(f, p, -1)
	write_line(f)
	for p in values: write_f64_f2exp(f, p, -1)

with open("tests/test_bank.h", "w") as f:
	f.write("#pragma once\n")
	f.write("\n")
	
	f.write("#include <stdint.h>\n")
	f.write("\n")

	f.write("struct f32_to_str_test\n{\n")
	f.write("\tuint32_t value;\n")
	f.write("\tint32_t precision;\n")
	f.write("\tbool exponent_form;\n")
	f.write("\tconst char* expected;\n")
	f.write("};\n")
	f.write("\n")

	f.write("struct f64_to_str_test\n{\n")
	f.write("\tuint64_t value;\n")
	f.write("\tint32_t precision;\n")
	f.write("\tbool exponent_form;\n")
	f.write("\tconst char* expected;\n")
	f.write("};\n")
	f.write("\n")
	
	f.write("const f32_to_str_test f32_test_suite[] = \n{\n")
	write_f32_tests(f)
	f.write("};\n")
	f.write("\n")

	f.write("const f64_to_str_test f64_test_suite[] = \n{\n")
	write_f64_tests(f)
	f.write("};\n")