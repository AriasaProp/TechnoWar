#ifndef Included_UI_Batch
#define Included_UI_Batch 1

#include "../translatedGraphicsFunction.h"

struct UI_Batch {
private:
	//int blendSrcFunc = TGF_SRC_ALPHA;
	//int blendDstFunc = TGF_ONE_MINUS_SRC_ALPHA;
	unsigned int texUsed = 0;
	void *vertices;
	float *projection;
	float colorPacked = (float)0xffffffff;
	texture_core *lastTexture = nullptr;
public:
	UI_Batch(float,float);
	~UI_Batch();
	void resize(float, float);
	void begin();
	void end();

  void draw(texture_core*, float, float);
  void draw(texture_core*, float, float, float, float);
  void draw(texture_core*, float, float, float, float, float, float, float, float);
};

#endif // Included_UI_Batch
