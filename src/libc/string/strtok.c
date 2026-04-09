#include <stddef.h>

char* strtok(char* str, const char* delim) {
	static char* buffer = NULL;
	if(str != NULL) {
		buffer = str;
	}

	if(buffer == NULL || *buffer == '\0') {
		return NULL;
	}

	char* tokenStart = buffer;
	while (*tokenStart != '\0') {
		const char* d = delim;
		while(*d != '\0') {
			if(*tokenStart == *d) break;
			d++;
		}
		if(*d == '\0') break;
		tokenStart++;
	}

	if(*tokenStart == '\0') {
		buffer = tokenStart;
		return NULL;
	}

	char* tokenEnd = tokenStart;
	while(*tokenEnd != '\0') {
		const char* d = delim;
		while(*d != '\0') {
			if(*tokenEnd == *d) {
				*tokenEnd = '\0';
				buffer = tokenEnd + 1;
				return tokenStart;
			}
			d++;
		}
		tokenEnd++;
	}

	buffer = tokenEnd;
	return tokenStart;
}
