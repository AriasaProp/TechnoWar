#ifndef Included_MATRIX4
#define Included_MATRIX4 1

namespace matrix4 {
	void idt(float *);
	void mul(float *, float*);
	void rotate(float *, float, float, float);
	void toOrtho(float *, float, float, float, float, float, float);
}

#endif //Included_MATRIX4