

enum {
  A = 1,
  B = 2,
  C = 3
};

static const volatile register aaa = 0x012345678ABCDEFL;

void f();

/*
 * main() - The entry point of the program
 */
// Note that declaration list followed by function header is not supported
int main(int argc, char **argv, ...) /* int x, y, z; */ {
  static const static volatile register int *(*xyz)(int(*)(), long *) = C;
  long x = 1 & xyz;
  void *c;
  // This struct is used to store data
  struct struct_type {
    int a;
    char b : 20;   // 20 bit field
    long c;
  } a, b, c;
  
  printf("Hello, world!\n");
  
  a.a = 20UL;
  a.b = 0x12345 >> (5 & 0xFFFFFFFF);
  a.c = 0777;
  b.b = '\n';
  
  return 0;
}
