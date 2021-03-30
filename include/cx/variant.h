#pragma once

#include <cx/idioms.h>
#include <cx/templates.h>

//Conditional logic if CX was built with stl support enabled
#ifdef CX_STL_SUPPORT
 #define CX_STL_SUPPORT_EXPR(expr) expr

 #include <exception>
#else
 #define CX_STL_SUPPORT_EXPR(expr)
#endif

#ifdef CX_LIBC_SUPPORT
 #include <cstring>
#endif

namespace CX {
 //Forward declare variant for use with supporting variant meta-functions
 template<typename... Elements>
 //Disallow duplicated elements, as well as void elements
 requires (UniqueTypes<void, Elements...>)
 struct Variant;

 //Supporting variant meta-functions and concepts
 namespace VariantMetaFunctions {
  template<typename V, typename = Unqualified<V>>
  struct IsVariant : FalseType {};

  template<typename Q, template<typename...> typename V, typename... Elements>
  struct IsVariant<Q, V<Elements...>> {
   static constexpr auto const Value = SameTemplateType<Variant, V>;
  };
 }

 //Simple variant identity concept
 template<typename MaybeVariant>
 concept IsVariant = VariantMetaFunctions::IsVariant<MaybeVariant>::Value;

 namespace VariantMetaFunctions {
  template<typename V, typename = Unqualified<V>>
  struct VariantCompatibilityBase;

  template<typename Q1, template<typename...> typename V1, typename... V1Elements>
  requires (CX::IsVariant<Q1>)
  struct VariantCompatibilityBase<Q1, V1<V1Elements...>> {
   //Meta-function to check whether V2 is compatible with V1.
   //Note: This function is directional; V2 may be compatible with V1,
   //but not the other way around.
   //eg:
   // Variant<int, float> v1{1234};
   // Variant<int, float, double> v2;
   // v2 = v1; //ok, `v2` is a superset Variant of `v1`
   // v1 = v2; //not ok, `v1` is not a superset Variant of `v2`, missing `double`
   template<typename V2, typename = Unqualified<V2>>
   struct IsCompatible : FalseType {};
  };

  template<typename Q1, template<typename...> typename V1, typename... V1Elements>
  requires (CX::IsVariant<Q1>)
  template<typename Q2, template<typename...> typename V2, typename... V2Elements>
  requires (CX::IsVariant<Q2>)
  struct VariantCompatibilityBase<Q1, V1<V1Elements...>>::IsCompatible<Q2, V2<V2Elements...>> {
   //For V2 to be compatible with V1, the element set for V2 must be a superset of V1's element
   //set.
   static constexpr auto const Value = sizeof...(V2Elements) >= sizeof...(V1Elements)
    && (MatchAnyType<Unqualified<V1Elements>, Unqualified<V2Elements>...> && ...);
  };
 }

 //Concept to check whether or not V2 is compatible with V1.
 //See line 34 and 53
 template<typename V1, typename V2>
 concept CompatibleVariant = VariantMetaFunctions
  ::VariantCompatibilityBase<V1>
  ::template IsCompatible<V2>
  ::Value;

 //Supporting exceptions
 struct VariantTypeError CX_STL_SUPPORT_EXPR(: std::exception) {
  char const * what() const noexcept {
   return "Variant type not present";
  }
 };

 struct IncompatibleVariantError CX_STL_SUPPORT_EXPR(: std::exception) {
  char const * what() const noexcept {
   return "Variant types are not convertible";
  }
 };

 //Variant impl
 template<typename... Elements>
 requires (UniqueTypes<void, Elements...>)
 struct Variant final {
  template<typename... Types>
  requires (UniqueTypes<void, Types...>)
  friend struct Variant;

  static constexpr auto const Size = MaxTypeSize<Elements...>;

 private:
  decltype(1 << sizeof...(Elements)) tag;
  char data[Size];

  //TODO this cannot be used yet due to a clang frontend bug.
  //See: https://bugs.llvm.org/show_bug.cgi?id=49743
  //
  //Note: Once this bug is fixed, make this a public member function
  void runtimeElementOp(auto op) const {
   //Do not attempt op if variant is empty
   if (tag) {
    TypeIterator<Elements...>::run([&]<typename E>() -> bool {
     if (has<E>()) {
      op.template operator()<E>();
      return false;
     }
     return true;
    });
   }
  }

  void destruct() const {
   auto &ref = const_cast<Variant&>(*this);
   //Clean up stored data
   /*TODO waiting for compiler bug to be fixed
   runtimElementOp([&]<typename E> {
    if constexpr(Destructible<E>) {
     (*(E *)&ref.data).~E();
    }
   });
   */
   TypeIterator<Elements...>::run([&]<typename E> {
    if (has<E>()) {
     if constexpr (Destructible<E>) {
      (*(E *)&ref.data).~E();
     }
     return false;
    }
    return true;
   });
   ref.tag = 0;
   #ifdef CX_VARIANT_HARD_CLEAR
    #ifdef CX_LIBC_SUPPORT
     memset(ref.data, 0, Size);
    #else
     for (auto &c : ref.data) {
      c = 0;
     }
    #endif
   #endif
  }

  /*
  template<typename EQualified, MatchAnyType<Elements...> E = Unqualified<EQualified>>
  void assign(EQualified e) const {
   auto &ref = const_cast<Variant&>(*this);
   destruct();
   ref.tag = 1 << IndexOfType<E, Elements...>;
   //Prevent l/r-value decay by re-promoting to `EQualified`
   *(E *)&ref.data = (EQualified)e;
  }
  */

  template<MatchAnyType<Elements...> E>
  requires (CopyConstructible<E> || CopyAssignable<E>)
  void assign(E const &e) const {
   auto &ref = const_cast<Variant&>(*this);
   destruct();
   ref.tag = 1 << IndexOfType<E, Elements...>;
   //Prevent l-value decay by re-promoting
   if constexpr (CopyConstructible<E>) {
    //[&]<typename...> {
    new (&ref.data) E{(E const&)e};
    //}();
   } else if constexpr (CopyAssignable<E>) {
    //[&]<typename...> {
    *(E *)&ref.data = (E const&)e;
    //}();
   }
  }

  template<MatchAnyType<Elements...> E>
  requires (!Const<E> && (MoveConstructible<E> || MoveAssignable<E>))
  void assign(E &&e) const {
   auto &ref = const_cast<Variant&>(*this);
   destruct();
   ref.tag = 1 << IndexOfType<E, Elements...>;
   //Prevent r-value reference -> l-value reference decay by re-promoting
   if constexpr (MoveConstructible<E>) {
    new (&ref.data) E{(E&&)e};
   } else if constexpr (MoveAssignable<E>) {
    *(E *)&ref.data = (E&&)e;
   }
  }

  void convert(auto &otherVariant) {
   /*TODO use when compiler bug has been fixed
   runtimeElementOp([&]<typename E> {
    otherVariant = get<E>();
   });
   */
   TypeIterator<Elements...>::run([&]<typename E> {
    if (has<E>()) {
     otherVariant = get<E>();
     return false;
    }
    return true;
   });
  }

 public:
  Variant() :
   tag(0),
   data{}
  {}

  //Element copy constructor
  template<MatchAnyType<Elements...> E>
  Variant(E const &e) :
   tag(0),
   data{}
  {
   assign((E const&)e);
  }

  //Element move constructor
  template<MatchAnyType<Elements...> E>
  Variant(E &&e) :
   tag(0),
   data{}
  {
   assign((E&&)e);
  }

  //Copy constructor
  Variant(CompatibleVariant<Variant> auto const &v) :
   tag(0),
   data{}
  {
   operator=((decltype(v)&)v);
  }

  //Move constructor
  Variant(CompatibleVariant<Variant> auto &&v) :
   tag(0),
   data{}
  {
   operator=((decltype(v)&)v);
   v.destruct();
  }

  ~Variant() {
   destruct();
  }

  //Checks the type of the encapsulated element
  template<MatchAnyType<Elements...> E>
  bool has() const noexcept {
   return tag & (1 << IndexOfType<E, Elements...>);
  }

  //Returns reference to the encapsulated element
  template<MatchAnyType<Elements...> E>
  E& get() const {
   if (has<E>()) {
    return *(E *)&data;
   }
   throw VariantTypeError{};
  }

  //Return encapsulated element and clear variant
  template<MatchAnyType<Elements...> E>
  E drain() {
   if (has<E>()) {
    struct GC {
     Variant const &v;

     ~GC() {
      v.destruct();
     }
    } gc{*this};
    return *(E *)&data;
   }
   throw VariantTypeError{};
  }

  //Similar to `drain()`, however, instead of returning a value,
  //accept a reference to the destination and move the
  //encapsulated element
  template<MatchAnyType<Elements...> E>
  void rdrain(E &e) {
   if (has<E>()) {
    e = (E&&)*(E *)&data;
    destruct();
   }
   throw VariantTypeError{};
  }

  //Copy assignment operator
  Variant& operator=(CompatibleVariant<Variant> auto const &v) {
   destruct();
   /*TODO use when compiler bug has been fixed
   v.runtimeElementOp([&]<typename E> {
    destruct();
    assign(*(E *)&v.data);
   });
   */
   TypeParameterDeducer<TypeIterator, Unqualified<decltype(v)>>::run([&]<typename E> {
    if (v.template has<E>()) {
     destruct();
     assign(*(E *)&v.data);
     return false;
    }
    return true;
   });
   return *this;
  }

  //Move assignment operator
  Variant& operator=(CompatibleVariant<Variant> auto &&v) {
   operator=((Unqualified<decltype(v)> const &)v);
   v.destruct();
   return *this;
  }

  //Element copy assignment operator
  Variant& operator=(MatchAnyType<Elements...> auto const &e) {
   assign((decltype(e)&)e);
   return *this;
  }

  //Element move assignment operator
  Variant& operator=(MatchAnyType<Elements...> auto &&e) {
   assign((decltype(e)&&)e);
   return *this;
  }

  //Implicit conversion to compatible variant types
  template<typename OtherVariant>
  requires CompatibleVariant<OtherVariant, Variant>
  operator OtherVariant() const {
   Unqualified<OtherVariant> v;
   if (tag) {
    convert(v);
   }
   return v;
  }
 };

 //Empty variant
 template<>
 struct Variant<> final {
  static constexpr decltype(sizeof(0)) const Size = 0;

 private:
  template<typename... Elements>
  requires (UniqueTypes<void, Elements...>)
  friend struct Variant;

  char data[Size];

  void runtimeElementOp(auto) {}

  void destruct() {}

  template<typename E>
  void assign(E) {
   throw IncompatibleVariantError{};
  }

 public:
  template<typename E>
  bool has() const noexcept {
   return false;
  }

  template<typename E>
  E& get() {
   throw VariantTypeError{};
  }

  template<typename E>
  E drain() {
   throw VariantTypeError{};
  }

  template<typename E>
  void rdrain(E&) {
   throw VariantTypeError{};
  }
 };
}

//Clean up internal macros
#undef CX_STL_SUPPORT_EXPR
