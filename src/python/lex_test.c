
enum {
  A = 1;
  B = 2;
  C = 3; 
};

/*
 * main() - The entry point of the program
 */
int main(int argc, char **argv, ...) {
  printf("Hello, world!\n");
  
  enum xyz = C;
  
  // This struct is used to store data
  struct struct_type {
    int a;
    char b : 20;   // 20 bit field
    long c;
  } a, b, c;
  
  a.a = 20UL;
  a.b = 0x12345 >> 5 & 0xFFFFFFFF;
  a.c = 0777;
  b.b = '\n';
  
  return 0;
}
