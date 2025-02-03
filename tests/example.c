#include "dragonito.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
	/* ***** Basic usage ***** */
	{
		float value = 123.456f;
		drgnt_i32 max_length = drgnt_f32_to_default(NULL, value);

		char* buffer = malloc(max_length + 1);
		drgnt_i32 length = drgnt_f32_to_default(buffer, value);
		buffer[length] = 0;

		printf("%s\n", buffer);
	}

	/* ***** Advanced usage ***** */
	{
		//Extract the float's bits
		drgnt_u32 bits = drgnt_f32_to_bits(123.5f);

		//Estimate the output length so we can allocate enough memory
		drgnt_u32 max_length = drgnt_estimate_srt_len32(bits, 1, 0);

		char* buffer = malloc(max_length + 1);

		//Convert the float to a string using shortest-round-trip (SRT) formatting
		drgnt_i32 length = drgnt_f32_to_srt(buffer, bits, 1);
		buffer[length] = 0;

		printf("%s\n", buffer);
	}

	return 0;
}

#define DRGNT_IMPLEMENTATION
#include "dragonito.h"