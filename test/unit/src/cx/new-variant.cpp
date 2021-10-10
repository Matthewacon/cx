#include <cx/test/common/common.h>

#include <cx/new-variant.h>

struct Z {
 constexpr Z& operator=(Z const&) noexcept = default;

 constexpr ~Z() noexcept {
  if (!CX::isConstexpr()) {
   printf("Z::~Z()\n");
  }
 }
};

template<typename T, auto N>
using AAA = T[N];

namespace CX {
 /*
 constexpr int testing() noexcept {
  CX::Variant<int, Z> v{1234};
  v = 4567;
  //v = Z{};

  //int d[2] {1, 2};
  CX::Variant<int[2]> v1{{1, 2}};
  auto toReturn = v1.get<int[2]>()[1];

  CX::Variant<Z[4]> v2{{Z{}, {}, {}, {}}};
  return toReturn;
 }
 */

 struct A {
  ~A() noexcept {
   printf("A::~A()\n");
  }
 };

 TEST(Variant, a) {
  /*
  using TypeA = int;
  static constexpr TypeA const expectedValue = 1234;
  constexpr CX::Variant<TypeA> v{expectedValue};
  EXPECT_TRUE(v.has<TypeA>());
  //EXPECT_EQ(v.get<TypeA>(), expectedValue);
  (void)v;
  */

  //CX::Variant<Z[4]> vvvv{{Z{}, {}, {}, {}}};

  /*
  constexpr auto r = testing();
  (void)r;
  printf("testing(): %d\n", r);
  */

  CX::Variant<A> v2{A{}};
  (void)v2;

  CX::Variant<std::string> v3{std::string{}};
  (void)v3;

  Variant<int> vv1{1234};
  Variant<int> vv2{};
  vv2 = vv1;
 }
}
