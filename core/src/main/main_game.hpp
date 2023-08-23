#ifndef Included_MainListener
#define Included_MainListener 1

struct Main {
private:
  struct mMainData *mdata;

public:
  Main ();
  void resume ();
  void render ();
  void pause ();
  ~Main ();
};

#endif // Included_MainListener
