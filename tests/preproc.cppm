module;
#include <stdio.h>

export module preproc;
export import a;
import b;

#ifdef _WIN32
#define main \
  WinMain
#endif

int main() { printf("ok\n"); }

module : private;

