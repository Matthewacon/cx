#include <cx/test/common/common.h>

#include <cx/lambda.h>

void abcd() {
 printf("abcd\n");
}

namespace CX {
 TEST(a, a) {
  char data[56]{'f'};
  (void)data;
  CX::Lambda<void ()> test {
   /*
   [=]() {
    printf("Hello from encapsulated lambda: %c\n", data[0]);
   }
   */
   abcd
  };
  test();
 }
}
