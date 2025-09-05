#ifndef __CORE__
#define __CORE__

// this special for mobile device
extern void *core_stateSaved(void);
extern unsigned int core_stateLength(void);
extern void core_stateLoad(void *);
// common loop
extern void Main_update();
extern void Main_pause();
extern void Main_term();

#endif // __CORE__