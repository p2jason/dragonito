# Generate code for doing 128/64 bit division
# It is based on: Improved Division by Invariant Integers (Moller and Granlund)
# The code adapted from: http://www.cecm.sfu.ca/CAG/code/TangentGraeffe/int128g.c
import ctypes

divisor = 10_000_000_000_000_000_000

s = 0
p = divisor

while (p >> 63) == 0:
	p <<= 1
	s += 1

u0 = ctypes.c_ulonglong(-1).value
u1 = ctypes.c_ulonglong(-p - 1).value

v = ((u1 << 64) | u0) // p

print(f"s = {s}")
print(f"v = {v}")
print(f"d0 = {p}")
print(f"d1 = {p}")