#ifndef QSSF_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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
};

struct QSSF_Image {
	uint32_t width;
	uint32_t height;
	enum QSSF_ImageComp comp;
	uint32_t numLayers;
	struct QSSF_Layer* layers;
};

#ifdef __cplusplus
}
#endif

#endif // !QSSF_H_INCLUDED_
