#ifndef Included_Android_Info
#define Included_Android_Info

#include "../engine.hpp"

struct android_info : public engine::info_core {
	private:

	public:
	android_info();
	~android_info();
	long memory() override;
};

#endif // Included_Android_Info