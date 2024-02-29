#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb_image.h"
#include "qssf.h"

int WriteQSSF(const struct QSSF_Image* img, const char* filePath) {
	uint8_t* out = NULL;
	uint32_t outSize = 0;

	if (QSSF_ImageEncode(img, &out, &outSize) != 0) {
		printf("[ERROR] QSSF_ImageEncode(...) Failed.\n");
		return 1;
	}
	FILE* f = fopen("./test.qssf", "wb");

	if (f == NULL) {
		free(out);
		printf("[ERROR] fopen(...) - %s\n", strerror(errno));
		return 1;
	}
	if (fwrite(out, outSize, 1, f) != 1) {
		fclose(f);
		free(out);
		printf("[ERROR] fwrite(...) - %s\n", strerror(errno));
		return 1;
	}

	fclose(f);
	free(out);

	return 0;
}

int ReadQSSF(struct QSSF_Image* img, const char* filePath) {
	return 0;
}

int main(void) {
	struct QSSF_Image imgToWrite = {0};

	if (WriteQSSF(&imgToWrite, "./test.qssf") == 0) {
		printf("[INFO] Wrote 'test.qssf'\n");
	} else {
		printf("[ERROR] WriteQSSF(...) failed\n");
		return 1;
	}

	struct QSSF_Image imgToRead = {0};

	if (ReadQSSF(&imgToRead, "./test.qssf") == 0) {
		printf("[INFO] Read 'test.qssf'\n");
	} else {
		printf("[ERROR] ReadQSSF(...) failed\n");
		return 1;
	}

	return 0;
}

