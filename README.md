# Dragonito
This is a library C99 header-only library for converting IEEE-754 floating point numbers to string.

## Basic Usage
This is a header-only library. The headers can be copied anywhere else, as long as they are all in the same directory. A basic example is provided below:

```cpp
#include "dragonito.h" //Can be included anywhere
#include <stdio.h>

int main(void)
{
	float value = 123.456;
	drgnt_i32 max_length = drgnt_f32_to_default(NULL, value);
	
	char* buffer = malloc(max_length + 1);
	drgnt_i32 length = drgnt_f32_to_default(buffer, value);
	buffer[length] = 0;

	printf("%s\n", buffer); //Output: "123.456"

	return 0;
}

//Include only in a single .c file
#define DRGNT_IMPLEMENTATION
#include "dragonito.h"
```

## Advanced usage
This library also offers several adjustable formatting functions:

|   Function Name  | Input Value | Output |
| :--------------- | :---------: | :----: |
| drgnt_f32_to_fix | 11000000512 | 11000000512.0  |
| drgnt_f32_to_exp | 11000000512 | 1.1000000512e+10 |
| drgnt_f32_to_srt | 11000000512 | 1.1e+10 |
| drgnt_f32_to_gen | 11000000512 | 11000000000 |

*(The examples are using the functions for 32-bit floats, but 64-bit equivalents are also provided)*

### Example
Here is a simple example of how to use one of these formatting functions:
	
```cpp
//Extract the float's bits
drgnt_u32 bits = drgnt_f32_to_bits(123.5f);

//Estimate the output length so we can allocate enough memory
drgnt_u32 max_length = drgnt_estimate_srt_len32(bits, 1, 0); 

char* buffer = malloc(max_length + 1);

//Convert the float to a string using shortest-round-trip (SRT) formatting
drgnt_i32 length = drgnt_f32_to_srt(buffer, bits, 1);
buffer[length] = 0;

printf("%s\n", buffer); //Output: "123.5"
```