#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb_image.h"
#include "stb_image_write.h"
#include "qssf.h"

struct stbi_func_args {
	uint32_t* size;
	uint8_t** data;
};

void stbi_func(void* context, void* data, int size) {
	struct stbi_func_args* args = context;
	*args->data = malloc(size);
	memcpy(*args->data, data, size);
	*args->size = size;
}

int EncodePng(uint32_t width, uint32_t height, uint8_t numChannels, uint8_t* pixel_data, uint8_t** out_data, uint32_t* out_size) {
	*pixel_data = 0;

	struct stbi_func_args args = {0};
	args.size = out_size;
	args.data = out_data;

	if (stbi_write_png_to_func(stbi_func, &args, width, height, numChannels, pixel_data, width * numChannels) == 0) {
		return 1;
	}

	return 0;
}

int WriteQSSF(const struct QSSF_Image* img, const char* filePath) {
	uint8_t* out = NULL;
	uint32_t outSize = 0;

	if (QSSF_ImageEncode(img, &out, &outSize, EncodePng) != 0) {
		printf("[ERROR] QSSF_ImageEncode(...) failed\n");
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
	FILE* f = fopen(filePath, "rb");
	if (f == NULL) {
		printf("[ERROR] fopen(...) failed\n");
		return 1;
	}

	fseek(f, 0, SEEK_END);
	uint32_t fSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8_t* fData = malloc(fSize);

	if (fread(fData, fSize, 1, f) != 1) {
		free(fData);
		fclose(f);
		printf("[ERROR] fread(...) failed\n");
		return 1;
	}

	if (QSSF_ImageDecode(img, fData, fSize) != 0) {
		free(fData);
		fclose(f);
		printf("[ERROR] QSSF_ImageDecode(...) failed\n");
		return 1;
	}

	free(fData);
	fclose(f);
	f = NULL;

	return 0;
}

const char* files[] = {
	"./example/mac.png",
	"./example/text.png",
	"./example/bg.png"
};

int main(void) {
	struct QSSF_Image imgToWrite = {0};

	for (uint32_t i = 0; i < (sizeof(files)/sizeof(files[0])); i++) {
		int w, h, c;
		uint8_t* data = stbi_load(files[i], &w, &h, &c, 4);

		if (data == NULL || w < 1 || h < 1 || c != 4) {
			printf("[ERROR] stbi_load(...) failed for '%s'\n", files[i]);
			return 1;
		}

		imgToWrite.width = w;
		imgToWrite.height = h;
		imgToWrite.comp = QSSF_IMAGE_COMP_RGBA;

		if (QSSF_ImageAddLayer(&imgToWrite, "New Layer", 255, QSSF_LAYER_BLEND_NORMAL) != 0) {
			printf("[ERROR] QSSF_ImageAddLayer(...) failed\n");
			free(data);
			return 1;
		}
		imgToWrite.layers[imgToWrite.numLayers - 1].pixels = data;
	}

	if (WriteQSSF(&imgToWrite, "./test.qssf") == 0) {
		printf("[INFO] Wrote 'test.qssf'\n");
	} else {
		printf("[ERROR] WriteQSSF(...) failed\n");
		return 1;
	}

	QSSF_ImageDestroy(&imgToWrite);

	struct QSSF_Image imgToRead = {0};

	if (ReadQSSF(&imgToRead, "./test.qssf") == 0) {
		printf("[INFO] Read 'test.qssf'\n");
	} else {
		printf("[ERROR] ReadQSSF(...) failed\n");
		return 1;
	}

	printf(
		"[INFO] Resolution: %ux%u\n"
		"[INFO] Channels: %u\n"
		"[INFO] Layers: %u\n",
		imgToRead.width, imgToRead.height,
		imgToRead.comp, imgToRead.numLayers
	);

	if (imgToRead.layers != NULL) {
		for (uint32_t i = 0; i < imgToRead.numLayers; i++) {
			struct QSSF_Layer* layer = &imgToRead.layers[i];
			printf(
				"[INFO]   Layer %u:\n"
				"[INFO]     Name: '%s'\n"
				"[INFO]     Blend: %u\n"
				"[INFO]     Opacity: %u\n",
				i, layer->name, layer->blend, layer->opacity
			);
		}
	}

	QSSF_ImageDestroy(&imgToRead);

	return 0;
}

