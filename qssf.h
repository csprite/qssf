#ifndef QSSF_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define QSSF_HEADER_SIZE 13
#define QSSF_MAGIC \
	(((unsigned int)'q') << 24 | ((unsigned int)'s') << 16 | ((unsigned int)'s') <<  8 | ((unsigned int)'f'))

enum QSSF_ImageComp {
	QSSF_IMAGE_COMP_RGB = 3,
	QSSF_IMAGE_COMP_RGBA = 4
};

enum QSSF_LayerBlend {
	QSSF_LAYER_BLEND_NORMAL = 0
};

struct QSSF_Layer {
	char* name;
	uint8_t opacity;
	enum QSSF_LayerBlend blend;
	uint8_t* pixels;
};

struct QSSF_Image {
	uint32_t width;
	uint32_t height;
	enum QSSF_ImageComp comp;
	uint32_t numLayers;
	struct QSSF_Layer* layers;
};

typedef uint8_t* (*QSSF_EncodePng_CB)(uint32_t width, uint32_t height, uint8_t numChannels, uint8_t* pixel_data);
typedef uint8_t* (*QSSF_DecodePng_CB)(uint32_t* width, uint32_t* height, uint8_t* numChannels, const uint8_t* png_data);

int QSSF_ImageDecode(struct QSSF_Image* img, const uint8_t* inData, uint32_t inSize);

int QSSF_ImageAddLayer(struct QSSF_Image* img, const char* name, uint8_t opacity, enum QSSF_LayerBlend blend);
void QSSF_ImageFreeLayer(struct QSSF_Layer* layer);

int QSSF_ImageEncode(const struct QSSF_Image* img, uint8_t** outData, uint32_t* outSize);

#ifdef __cplusplus
}
#endif

#endif // !QSSF_H_INCLUDED_

