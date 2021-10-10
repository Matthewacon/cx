#pragma once

#include <cx/common.h>
#include <cx/idioms.h>
#include <cx/templates.h>
#include <cx/error.h>
#include <cx/exit.h>

//Conditional dependency if CX was built with libc support enabled
#ifdef CX_LIBC_SUPPORT
 #include <cstring>
#endif

namespace CX {
 //Forward declare variant for use with supporting variant meta-functions
 template<typename... Elements>
 //Disallow duplicated elements, as well as void elements
 requires (UniqueTypes<void, Elements...> && !(UnsizedArray<Elements> || ...))
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
 struct VariantTypeError final {
  constexpr char const * describe() const noexcept {
   return "Variant type not present";
  }
 };
 static_assert(IsError<VariantTypeError>);

 struct IncompatibleVariantError final {
  constexpr char const * describe() const noexcept {
   return "Variant types are not convertible";
  }
 };
 static_assert(IsError<IncompatibleVariantError>);

 //Variant impl
 template<typename... Elements>
 requires (UniqueTypes<void, Elements...> && !(UnsizedArray<Elements> || ...))
 struct Variant final {
  template<typename... Types>
  requires (UniqueTypes<void, Types...> && !(UnsizedArray<Types> || ...))
  friend struct Variant;

  static constexpr auto const Size = MaxTypeSize<Elements...>;
  static constexpr auto const Alignment = MaxTypeAlignment<Elements...>;

 private:
  decltype(1 << sizeof...(Elements)) tag;
  alignas(Alignment) unsigned char data[Size];

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

  template<MatchAnyType<Elements...> E>
  requires (Trivial<ConstVolatileDecayed<E>>)
  void trivialCopy(E const &e) const {
   static constexpr auto const Length = sizeof(E);
   auto &ref = const_cast<Variant&>(*this);
   #ifdef CX_LIBC_SUPPORT
    memcpy(&ref.data, &e, Length);
   #else
    using LengthType = ConstDecayed<decltype(Length)>;
    auto &array = *(decltype(ref.data)*)&e;
    for (LengthType i = 0; i < Length; i++) {
     ref.data[i] = array[i];
    }
   #endif
  }

  void destruct() const {
   auto &ref = const_cast<Variant&>(*this);

   //Clean up stored data
   if (tag) {
    //TODO use runtimeElementOp once the clang frontend
    //bug has been fixed
    TypeIterator<Elements...>::run([&]<typename E> {
     if (has<E>()) {
      if constexpr (Destructible<E>) {
       //Destruct encapsulated element
       (*(E *)&ref.data).~E();
      } else if constexpr (Array<E> && Destructible<ArrayDecayed<E>>) {
       //Destruct all elements of encapsulated array
       using ElementType = ArrayDecayed<E>;
       static constexpr auto const Length = ArraySize<E>;
       using LengthType = ConstVolatileDecayed<decltype(Length)>;
       auto &array = *(ElementType(*)[Length])&ref.data;

       //Invoke destructor for every element type
       for (LengthType i = 0; i < Length; i++) {
        array[i].~ElementType();
       }
      }
      return false;
     }
     return true;
    });
    ref.tag = 0;
    #ifdef CX_VARIANT_HARD_CLEAR
     #ifdef CX_LIBC_SUPPORT
      memset(&ref.data, 0, Size);
     #else
      for (auto &c : ref.data) {
       c = 0;
      }
     #endif
    #endif
   }
  }

  template<MatchAnyType<Elements...> E>
  requires (
   !Array<ConstDecayed<E>>
    && (CopyConstructible<ConstDecayed<E>>
     || (Constructible<ConstDecayed<E>> && CopyAssignable<ConstDecayed<E>>)
    )
  )
  void assign(E const &e) const {
   using EDecayed = ConstDecayed<E>;
   auto &ref = const_cast<Variant&>(*this);

   //Clean up current state
   destruct();

   //Assign new tag for corresponding element type
   ref.tag = 1 << IndexOfType<E, Elements...>;

   //Prevent l-value decay by re-promoting
   if constexpr (Trivial<EDecayed>) {
    trivialCopy<E>((E const&)e);
   } if constexpr (CopyConstructible<EDecayed>) {
    //If `EDecayed` is copy-constructible, try constructing it
    new (&ref.data) E{(E const&)e};
   } else if constexpr (Constructible<EDecayed> && CopyAssignable<EDecayed>) {
    //If `EDecayed` is default-constructible and copy-assignable
    //try constructing and assigning to it
    *new (&ref.data) E{} = (E const&)e;
   }
  }

  template<MatchAnyType<Elements...> E>
  requires (
   !Array<ConstDecayed<E>>
    && (MoveConstructible<ConstDecayed<E>>
     || (Constructible<ConstDecayed<E>> && MoveAssignable<ConstDecayed<E>>)
    )
  )
  //r-value assignments cannot have const operands
  void assign(ConstDecayed<E> &&e) const {
   using EDecayed = ConstDecayed<E>;
   auto &ref = const_cast<Variant&>(*this);

   //Clean up current state
   destruct();

   //Assign new tag for corresponding element type
   ref.tag = 1 << IndexOfType<E, Elements...>;

   //Prevent r-value reference -> l-value reference decay by
   //re-promoting
   if constexpr (Trivial<EDecayed>) {
    //If `EDecayed` is a trivial type, no need to move construct
    //or assign
    trivialCopy<E>((EDecayed const&)e);
   } if constexpr (MoveConstructible<EDecayed>) {
    //If `EDecayed` is move constructible, try constructing it
    new (&ref.data) E{(EDecayed&&)e};
   } else if constexpr (Constructible<EDecayed> && MoveAssignable<EDecayed>) {
    //If `EDecayed` is default constructible and move assignable,
    //try constructing and assigning to it
    *new (&ref.data) E{} = (EDecayed&&)e;
   }
  }

  //Array type copy assignment
  template<MatchAnyType<Elements...> E>
  requires (
   SizedArray<E>
    && (Trivial<ArrayDecayed<E>>
     || CopyConstructible<ArrayDecayed<E>>
     || (Constructible<ArrayDecayed<E>> && CopyAssignable<ArrayDecayed<E>>)
    )
  )
  void assign(E const &src) const {
   using ElementType = ArrayDecayed<E>;
   static constexpr auto const Length = ArraySize<E>;
   using LengthType = ConstVolatileDecayed<decltype(Length)>;
   auto &variantRef = const_cast<Variant&>(*this);

   //Clean up current state
   destruct();

   //Assign new tag for corresponding element type
   variantRef.tag = 1 << IndexOfType<E, Elements...>;

   //Initialize encapsulated array
   auto &dst = *(ElementType(*)[Length])&variantRef.data;
   if constexpr (Trivial<ElementType>) {
    //If `ElementType` is a trivial type, no need to copy construct
    //or assign
    trivialCopy<E>((E const&)src);
   } else {
    //Initialize every array element
    for (LengthType i = 0; i < Length; i++) {
     if constexpr (CopyConstructible<ElementType>) {
      //Copy construct array element
      new (&dst[i]) ElementType{(ElementType const&)src[i]};
     } else if constexpr (Constructible<ElementType> && CopyAssignable<ElementType>) {
      //Default construct array element and copy assign new value
      *new (&dst[i]) ElementType{} = (ElementType const&)src[i];
     }
    }
   }
  }

  //Array type move assignment
  template<MatchAnyType<Elements...> E>
  requires (
   SizedArray<E>
    && (Trivial<ArrayDecayed<E>>
     || MoveConstructible<ArrayDecayed<E>>
     || (Constructible<ArrayDecayed<E>> && MoveAssignable<ArrayDecayed<E>>)
    )
  )
  void assign(ConstDecayed<E> &&src) {
   using ElementType = ArrayDecayed<E>;
   static constexpr auto const Length = ArraySize<E>;
   using LengthType = ConstVolatileDecayed<decltype(Length)>;
   auto &variantRef = const_cast<Variant&>(*this);

   //Clean up current state
   destruct();

   //Assign new tag for corresponding element type
   variantRef.tag = 1 << IndexOfType<E, Elements...>;

   //Initialize encapsulated array
   auto &dst = *(ElementType(*)[Length])&variantRef.data;
   if constexpr (Trivial<ElementType>) {
    //If `ElementType` is a trivial type, no need to copy construct
    //or assign
    trivialCopy<E>((E const&)src);
   } else {
    for (LengthType i = 0; i < Length; i++) {
     if constexpr (MoveConstructible<ElementType>) {
      //Move construct array element
      new (&dst[i]) ElementType{(ElementType&&)src[i]};
     } else if constexpr (Constructible<ElementType> && MoveAssignable<ElementType>) {
      //Default construct array element and move assign new value
      *new (&dst[i]) ElementType{} = (ElementType&&)src[i];
     }
    }
   }
  }

  void convert(auto &otherVariant) {
   //TODO use runtimeElementOp once the clang frontend
   //bug has been fixed
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
   assign<E>(e);
  }

  //Element move constructor
  template<MatchAnyType<Elements...> E>
  Variant(E &&e) :
   tag(0),
   data{}
  {
   assign<E>((E&&)e);
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
   exit(VariantTypeError{});
  }

  //Return encapsulated element and clear variant
  template<MatchAnyType<Elements...> E>
  requires (!Array<Unqualified<E>>)
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
   exit(VariantTypeError{});
  }

  //Similar to `drain()`, however, instead of returning a value,
  //accept a reference to the destination and move the
  //encapsulated element
  template<MatchAnyType<Elements...> E>
  requires (MoveAssignable<Unqualified<E>>)
  void rdrain(E &e) {
   if (has<E>()) {
    struct GC {
     Variant const &v;

     ~GC() {
      v.destruct();
     }
    } gc{*this};
    e = (E&&)*(E *)&data;
   } else {
    exit(VariantTypeError{});
   }
  }

  //Copy assignment operator
  Variant& operator=(CompatibleVariant<Variant> auto const &v) {
   //TODO use runtimeElementOp when clang frontend bug has been fixed
   TypeParameterDeducer<TypeIterator, Unqualified<decltype(v)>>::run([&]<typename E> {
    if (v.template has<E>()) {
     destruct();
     assign<E>(*(E *)&v.data);
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
  template<MatchAnyType<Elements...> E>
  Variant& operator=(E const &e) {
   assign<E>((E const&)e);
   return *this;
  }

  //Element move assignment operator
  template<MatchAnyType<Elements...> E>
  Variant& operator=(E &&e) {
   assign<E>((E&&)e);
   return *this;
  }

  //Implicit conversion to compatible variant types
  template<typename OtherVariant>
  requires CompatibleVariant<OtherVariant, Variant>
  explicit operator OtherVariant() const {
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
  static constexpr SizeType const Size = 0;
  static constexpr AlignType const Alignment = 1;

 private:
  template<typename... Types>
  requires (UniqueTypes<void, Types...> && !(UnsizedArray<Types> || ...))
  friend struct Variant;

  decltype(1 << 0) const tag = 0;
  alignas(Alignment) char const data[Size];

  void runtimeElementOp(auto) {}

  void destruct() {}

  template<typename E>
  void assign(E) {
   exit(IncompatibleVariantError{});
  }

 public:
  Variant() :
   tag(0),
   data{}
  {};

  //Variant copy constructor
  Variant(CompatibleVariant<Variant> auto const&) :
   tag(0),
   data{}
  {};

  //Variant move constructor
  Variant(CompatibleVariant<Variant> auto&&) :
   tag(0),
   data{}
  {};

  //Element copy constructor
  template<typename E>
  requires false
  Variant(E const&) :
   tag(0),
   data{}
  {
   exit(VariantTypeError{});
  }

  //Element move constructor
  template<typename E>
  requires false
  Variant(E&&) :
   tag(0),
   data{}
  {
   exit(VariantTypeError{});
  };

  template<typename E>
  bool has() const noexcept {
   return false;
  }

  template<typename E>
  requires false
  E& get() {
   exit(VariantTypeError{});
  }

  template<typename E>
  requires false
  E drain() {
   exit(VariantTypeError{});
  }

  template<typename E>
  requires false
  void rdrain(E&) {
   exit(VariantTypeError{});
  }

  //Copy assignment operator
  Variant& operator=(CompatibleVariant<Variant> auto const&) {
   return *this;
  }

  //Move assignment operator
  Variant& operator=(CompatibleVariant<Variant> auto&&) {
   return *this;
  }

  //Element copy assignment operator
  template<typename E>
  requires false
  Variant& operator=(E const&) {
   exit(VariantTypeError{});
  }

  //Element move assignment operator
  template<typename E>
  requires false
  Variant& operator=(E&&) {
   exit(VariantTypeError{});
  }
 };

 //Deduction guides for Variant
 Variant() -> Variant<>;
 template<typename E>
 Variant(E) -> Variant<E>;
}
