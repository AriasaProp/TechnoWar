#ifndef ENGINE_INCLUDED_
#define ENGINE_INCLUDED_

struct engine_graphics {
	void (*funct1)();
	void (*funct2)();
};
struct engine_asset {
	void (*funct1)();
	void (*funct2)();
};
struct engine_input {
	void (*funct1)();
	void (*funct2)();
};
struct engine_extras {
	void (*funct1)();
	void (*funct2)();
};

struct engine {
	struct engine_graphics g;
	struct engine_asset a;
	struct engine_input i;
	struct engine_extras e;
};

struct engine *get_engine();

#endif // ENGINE_INCLUDED_