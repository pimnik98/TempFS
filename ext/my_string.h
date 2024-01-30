#pragma once

#include <stdint.h>
static inline bool str_contains(const char* haystack, const char* needle) {
	const char* result = strstr(haystack, needle);

	return result != NULL;
}

bool strcmpn(const char *str1, const char *str2);

uint32_t str_cdsp2(const char a_str[], char del);
