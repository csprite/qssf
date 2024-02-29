#include <stdlib.h>
#include <string.h>

#include "qssf.h"

static inline uint8_t Read_8(const uint8_t* in) {
	return *in;
}

static inline uint16_t Read_16(const uint8_t* in) {
	uint16_t res = 0;
	res += ((uint16_t)*(in + 0)) << 0;
	res += ((uint16_t)*(in + 1)) << 8;
	return res;
}

static inline uint32_t Read_32(const uint8_t* in) {
	uint32_t res = 0;
	res += ((uint32_t)*(in + 0)) << 0;
	res += ((uint32_t)*(in + 1)) << 8;
	res += ((uint32_t)*(in + 2)) << 16;
	res += ((uint32_t)*(in + 3)) << 24;
	return res;
}

static inline void Write_8(uint8_t* out, uint8_t val) {
	*out = val;
}

static inline void Write_16(uint8_t* out, uint16_t val) {
	*(out + 0) = (val & 0x00FF) >> 0;
	*(out + 1) = (val & 0xFF00) >> 8;
}

static inline void Write_32(uint8_t* out, uint32_t val) {
	*(out + 0) = (val & 0x000000FF) >> 0;
	*(out + 1) = (val & 0x0000FF00) >> 8;
	*(out + 2) = (val & 0x00FF0000) >> 16;
	*(out + 3) = (val & 0xFF000000) >> 24;
}

int QSSF_ImageEncode(const struct QSSF_Image* img, uint8_t** outData, uint32_t* outSize) {
	return 0;
}

int QSSF_ImageDecode(struct QSSF_Image* img, const uint8_t* outData, uint32_t outSize) {
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

