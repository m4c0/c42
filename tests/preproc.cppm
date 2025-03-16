module;
#include <stdio.h>

export module preproc;
export import a;
import b;

#ifdef _WIN32
#warning windows is just a hack
#define main \
  WinMain
#else
#endif

int main() { printf("ok\n"); }

module : private;

