#ifndef Included_2DBatch
#define Included_2DBatch 1

struct 2DBatch {
private:
	int blendSrcFunc = TGF_SRC_ALPHA;
	int blendDstFunc = TGF_ONE_MINUS_SRC_ALPHA;
	int texUsed = 0;
	float *vertices;
	float colorPacked = 0xffffffff;
	void *lastTexture = nullptr;
  
public:
	2DBatch(float,float);
	~2DBatch();
	void resize(float, float);
	void begin();
	void end();

  void draw(void*, float, float);
  void draw(void*, float, float, float, float);
  void draw(void*, float, float, float, float, float, float, float, float);
}

#endif // Included_2DBatch
