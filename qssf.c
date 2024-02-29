#include <stdlib.h>
#include <string.h>

#include "qssf.h"

static inline uint8_t Read_8(const uint8_t* in, uint32_t* pos) {
	return in[(*pos)++];
}

static inline uint16_t Read_16(const uint8_t* in, uint32_t* pos) {
	uint16_t res = 0;
	res += ((uint16_t)(in[(*pos)++])) << 0;
	res += ((uint16_t)(in[(*pos)++])) << 8;
	return res;
}

static inline uint32_t Read_32(const uint8_t* in, uint32_t* pos) {
	uint32_t res = 0;
	res += ((uint32_t)(in[(*pos)++])) << 0;
	res += ((uint32_t)(in[(*pos)++])) << 8;
	res += ((uint32_t)(in[(*pos)++])) << 16;
	res += ((uint32_t)(in[(*pos)++])) << 24;
	return res;
}

static inline void Write_8(uint8_t* out, uint32_t* pos, uint8_t val) {
	out[(*pos)++] = val;
}

static inline void Write_16(uint8_t* out, uint32_t* pos, uint16_t val) {
	out[(*pos)++] = (val & 0x00FF) >> 0;
	out[(*pos)++] = (val & 0xFF00) >> 8;
}

static inline void Write_32(uint8_t* out, uint32_t* pos, uint32_t val) {
	out[(*pos)++] = (val & 0x000000FF) >> 0;
	out[(*pos)++] = (val & 0x0000FF00) >> 8;
	out[(*pos)++] = (val & 0x00FF0000) >> 16;
	out[(*pos)++] = (val & 0xFF000000) >> 24;
}

#include <stdio.h>
int QSSF_ImageEncode(const struct QSSF_Image* img, uint8_t** outData, uint32_t* outSize) {
	uint32_t maxSize = (img->width * img->height * img->comp) + 100;
	uint32_t pos = 0;

	*outData = malloc(maxSize);

	Write_32(*outData, &pos, QSSF_MAGIC);
	Write_32(*outData, &pos, img->width);
	Write_32(*outData, &pos, img->height);
	Write_8(*outData, &pos, img->comp);
	Write_32(*outData, &pos, img->numLayers);

	*outData = realloc(*outData, pos);
	*outSize = pos;

	return 0;
}

int QSSF_ImageDecode(struct QSSF_Image* outImg, const uint8_t* inData, uint32_t inSize) {
	if (inSize < QSSF_HEADER_SIZE) return 1;

	uint32_t pos = 0;
	uint32_t magic = Read_32(inData, &pos);
	uint32_t width = Read_32(inData, &pos);
	uint32_t height = Read_32(inData, &pos);
	uint8_t  comp = Read_8(inData, &pos);
	uint32_t numLayers = Read_32(inData, &pos);

	if (
		magic != QSSF_MAGIC ||
		width < 1 || height < 1 ||
		(comp != QSSF_IMAGE_COMP_RGB && comp != QSSF_IMAGE_COMP_RGBA) ||
		numLayers < 1
	) {
		return 1;
	}

	outImg->width = width;
	outImg->height = height;
	outImg->comp = comp;
	outImg->numLayers = numLayers;

	return 0;
}

int QSSF_ImageAddLayer(struct QSSF_Image* img, const char* name, uint8_t opacity, enum QSSF_LayerBlend blend) {
	img->numLayers++;
	img->layers = realloc(img->layers, sizeof(*img->layers) * img->numLayers);
	if (img->layers == NULL) {
		img->layers--;
		return 1;
	}

	struct QSSF_Layer* layer = &img->layers[img->numLayers - 1];
	layer->opacity = opacity;
	layer->blend = blend;
	layer->name = malloc(strlen(name) + 1);
	strncpy(layer->name, name, strlen(name));

	return 0;
}

void QSSF_ImageFreeLayer(struct QSSF_Layer* layer) {
	free(layer->name);
	layer->name = NULL;

	free(layer->pixels);
	layer->pixels = NULL;
}

