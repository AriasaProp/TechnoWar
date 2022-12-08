#ifndef Included_UI_Batch
#define Included_UI_Batch 1

struct UI_Batch {
private:
	int blendSrcFunc = TGF_SRC_ALPHA;
	int blendDstFunc = TGF_ONE_MINUS_SRC_ALPHA;
	int texUsed = 0;
	float *vertices;
	float colorPacked = 0xffffffff;
	void *lastTexture = nullptr;
  
public:
	UI_Batch(float,float);
	~UI_Batch();
	void resize(float, float);
	void begin();
	void end();

  void draw(void*, float, float);
  void draw(void*, float, float, float, float);
  void draw(void*, float, float, float, float, float, float, float, float);
}

#endif // Included_UI_Batch
