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

int QSSF_ImageEncode(const struct QSSF_Image* img, uint8_t** outData, uint32_t* outSize, QSSF_EncodePng_CB encode) {
	if (img->width < 1 || img->height < 1 || img->numLayers < 1 || encode == NULL) {
		return 1;
	}

	uint32_t maxSize = (img->width * img->height * img->comp) + QSSF_HEADER_SIZE;
	uint32_t pos = 0;

	*outData = malloc(maxSize);

	Write_32(*outData, &pos, QSSF_MAGIC);
	Write_32(*outData, &pos, img->width);
	Write_32(*outData, &pos, img->height);
	Write_8(*outData, &pos, img->comp);
	Write_32(*outData, &pos, img->numLayers);

	for (uint32_t i = 0; i < img->numLayers; i++) {
		struct QSSF_Layer* layer = &img->layers[i];
		uint32_t nameLen = strlen(layer->name);

		uint8_t* pngRaw = NULL;
		uint32_t pngRawSize = 0;
		if (
			encode(img->width, img->height, img->comp, layer->pixels, &pngRaw, &pngRawSize) != 0 ||
			pngRawSize < 1 || pngRaw == NULL
		) {
			free(*outData);
			*outData = NULL;
			return 1;
		}

		if (maxSize < (pos + (pngRawSize + 1) + (nameLen + 1) + 2 + 4)) {
			maxSize += (pngRawSize + 1) + nameLen + 2 + 4;
			*outData = realloc(*outData, maxSize);
		}

		for (uint32_t j = 0; j < nameLen + 1; j++) {
			Write_8(*outData, &pos, (uint8_t)layer->name[j]);
		}
		Write_8(*outData, &pos, '\0'); // NULL Terminate

		Write_8(*outData, &pos, layer->blend);
		Write_8(*outData, &pos, layer->opacity);
		Write_32(*outData, &pos, pngRawSize);

		for (uint32_t j = 0; j < pngRawSize; j++) {
			Write_8(*outData, &pos, pngRaw[j]);
		}
		free(pngRaw);
	}

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
	outImg->numLayers = 0;

	for (uint32_t i = 0; i < numLayers; i++) {
		uint32_t len;
		for (len = 0; pos + len < inSize; len++) {
			if (*(inData + pos + len) == 0) {
				break;
			}
		}
		if (pos + len + 1 >= inSize) {
			return 1;
		}

		char* name = malloc(len + 1);
		strncpy(name, (char*)inData + pos + len, len + 1);
		pos += len + 1;

		uint8_t blend = Read_8(inData, &pos);
		uint8_t opacity = Read_8(inData, &pos);
		uint32_t pngRawSize = Read_32(inData, &pos);

		pos += pngRawSize;

		QSSF_ImageAddLayer(outImg, name, opacity, blend);
		free(name);
	}

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
	strncpy(layer->name, name, strlen(name) + 1);

	return 0;
}

void QSSF_ImageFreeLayer(struct QSSF_Layer* layer) {
	if (layer->name != NULL) {
		free(layer->name);
		layer->name = NULL;
	}

	if (layer->pixels != NULL) {
		free(layer->pixels);
		layer->pixels = NULL;
	}
}

void QSSF_ImageDestroy(struct QSSF_Image* img) {
	if (img->layers != NULL) {
		for (uint32_t i = 0; i < img->numLayers; i++) {
			QSSF_ImageFreeLayer(&img->layers[i]);
		}
		free(img->layers);
	}
	memset(img, 0, sizeof(*img));
}

