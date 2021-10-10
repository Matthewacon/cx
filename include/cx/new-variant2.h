#pragma once

#include <cx/idioms.h>
#include <cx/templates.h>
#include <cx/exit.h>

//Yields `expr` if `CX_CONSTEXPR_SEMANTICS` is enabled, otherwise nothing
#ifdef CX_CONSTEXPR_SEMANTICS
 #define CX_CONSTEXPR_EXPR(expr) expr
#else
 #define CX_CONSTEXPR_EXPR(...)
#endif

namespace CX {
 //Error for evaluating a variant with a non-present type
 struct VariantTypeError final {
  constexpr auto& describe() const noexcept {
   return "Variant type not present";
  }
 };
 static_assert(IsError<VariantTypeError>);

 //Supporting `CX::Variant` meta-functions
 namespace VariantMetaFunctions {
  namespace Internal {
   //Type meta-function that strips all array qualifiers from a type
   template<typename T>
   struct ArrayFullyDecayed final {
    using Type = T;
   };

   template<typename T>
   requires Array<T>
   struct ArrayFullyDecayed<T> final {
    using Type = typename ArrayFullyDecayed<ArrayDecayed<T>>::Type;
   };
  }

  //Removes all array qualifiers from a type
  //Note: Shim to `Internal::ArrayFullyDecayed`
  template<typename T>
  using StripArray = typename Internal
   ::ArrayFullyDecayed<T>
   ::Type;

  //Whether or not a given type is a valid candidate for copy-initialization
  //with a variant. Must satisfy any of the following conditions:
  // - `T` is copy-constructible
  // - `T` is default constructible and copy-assignable
  template<typename T>
  concept VariantElementCopyInitable = CopyConstructible<StripArray<T>>
   || (Constructible<StripArray<T>> && CopyAssignable<StripArray<T>>);

  //Whether or not a given type is a valid candidate for move-initialization
  //with a variant
  // - `T` is move-constructible
  // - `T` is default constructible and move-assignable
  template<typename T>
  concept VariantElementMoveInitable = MoveConstructible<StripArray<T>>
   || (Constructible<StripArray<T>> && MoveAssignable<StripArray<T>>);
 }

 //Variant element identity concept
 template<typename E>
 concept VariantElement = !MatchAnyType<E, void, Never>
  && !UnsizedArray<E>
  && !Reference<E>
  && (VariantMetaFunctions::VariantElementCopyInitable<E>
   || VariantMetaFunctions::VariantElementMoveInitable<E>
  );

 //Supporting `CX::Variant` meta-functions
 namespace VariantMetaFunctions {
  //Empty nop struct to differentiate between `VariantStorage<...>` element
  //copy/move construction and default construction + copy/move assignment
  struct Disambiguate final {
   Never _{};
   constexpr Disambiguate() noexcept = default;
   constexpr ~Disambiguate() noexcept = default;
  };

  //Array type alias
  template<auto N, typename T>
  using ArrayPrototype = T[N];

  //Variant union base
  template<VariantElement...>
  union VariantStorage;

  //TODO handle initialization of multi-dimensional array types
  //Union impl
  template<VariantElement E, VariantElement... Elements>
  union VariantStorage<E, Elements...> final {
  private:
   //Returns mutable reference to `*this`
   constexpr VariantStorage& mut() const noexcept {
    return const_cast<VariantStorage&>(*this);
   }

   //Copy or move assigns to default constructed array elements
   template<typename A>
   constexpr void initArrayValue(A arr) noexcept
    requires Array<E>
   {
    constexpr auto const size = ArraySize<E>;
    using ElementType = ArrayDecayed<E>;
    using PromotionType = SelectType<
     SameType<A, E const&>,
     ElementType const&,
     ElementType&&
    >;
    auto& mut = const_cast<VariantStorage&>(*this);
    for (SizeType i = 0; i < size; i++) {
     mut.value[i] = (PromotionType)arr[i];
    }
   }

  public:
   using ElementType = E;

   //Intermediate value
   Never _;
   //Value
   E value;
   //Next value
   VariantStorage<Elements...> next;

   //Intermediate state constructor
   constexpr VariantStorage(Never) noexcept :
    _{}
   {}

   //Value copy-constructor
   constexpr VariantStorage(Disambiguate, E const& value) noexcept
   requires (!Array<E>) :
    value{(E const&)value}
   {}

   //Value passthrough copy-constructor
   template<typename T>
   requires (!Array<T> && MatchAnyType<T, Elements...>)
   constexpr VariantStorage(Disambiguate d, T const& value) noexcept :
    next{d, (T const&)value}
   {}

   //Value copy-assignment constructor
   constexpr VariantStorage(E const& value) noexcept
   requires (!Array<E>):
    value{}
   {
    mut().value = (E const&)value;
   }

   //Value passthrough copy-assignment constructor
   template<typename T>
   requires (!Array<T> && MatchAnyType<T, Elements...>)
   constexpr VariantStorage(T const& value) noexcept :
    next{(T const&)value}
   {}

   //Value move-constructor
   constexpr VariantStorage(Disambiguate, E&& value) noexcept
   requires (!Array<E>) :
    value{(E&&)value}
   {}

   //Value passthrough move-constructor
   template<typename T>
   requires (!Array<T> && MatchAnyType<T, Elements...>)
   constexpr VariantStorage(Disambiguate d, T&& value) noexcept :
    next{d, (T&&)value}
   {}

   //Value move-assignment constructor
   constexpr VariantStorage(E&& value) noexcept
   requires (!Array<E>) :
    value{}
   {
    mut().value = (E&&)value;
   }

   //Value passthrough move-assignment constructor
   template<typename T>
   requires (!Array<T> && MatchAnyType<T, Elements...>)
   constexpr VariantStorage(T&& value) noexcept :
    next{(T&&)value}
   {}

   //Array element copy-assignment constructor (for non-copy-construcible array
   //element types)
   template<auto N, typename T>
   requires SameType<T[N], E>
   constexpr VariantStorage(T const (&arr)[N]) noexcept :
    value{}
   {
    using ArrayType = ArrayPrototype<N, T> const&;
    initArrayValue<ArrayType>((ArrayType)arr);
   }

   //Array passthrough copy-assignment constructor
   template<auto N, typename T>
   requires (SameType<T[N], Elements> || ...)
   constexpr VariantStorage(T const (&arr)[N]) noexcept :
    next{(ArrayPrototype<N, T> const&)arr}
   {}

   //Array element copy-constructor (for copy-constructible array element
   //types)
   template<typename... ArrayElements>
   requires (Array<E> && (sizeof...(ArrayElements) == ArraySize<E>))
   constexpr VariantStorage(Disambiguate, ArrayElements const&... elements)
   noexcept :
    value{(ArrayElements const&)elements...}
   {}

   //Array passthrough copy-constructor (for copy-constructible array element
   //types)
   template<typename... ArrayElements>
   requires (
    (Array<Elements> && (sizeof...(ArrayElements) == ArraySize<Elements>))
    || ...
   )
   constexpr VariantStorage(Disambiguate d, ArrayElements const&... elements)
   noexcept :
    next{d, (ArrayElements const&)elements...}
   {}

   //Array element move-assignment constructor (for non-move-constructible
   //array element types)
   template<auto N, typename T>
   requires (SameType<E, T[N]>)
   constexpr VariantStorage(T (&&arr)[N]) noexcept :
    value{}
   {
    using ArrayType = T[N];
    initArrayValue<ArrayType&&>((ArrayType&&)arr);
   }

   //Array passthrough move-assignment constructor
   template<auto N, typename T>
   requires (SameType<T[N], Elements> || ...)
   constexpr VariantStorage(T (&&arr)[N]) noexcept :
    next{(ArrayPrototype<N, T>&&)arr}
   {}

   //Array element move-constructor (for move-constructible array element
   //types)
   template<typename... ArrayElements>
   requires (Array<E> && (sizeof...(ArrayElements) == ArraySize<E>))
   constexpr VariantStorage(Disambiguate, ArrayElements&&... elements)
   noexcept :
    value{(ArrayDecayed<E>&&)elements...}
   {}

   //Array passthrough move-constructor (for move-constructible array element
   //types)
   template<typename... ArrayElements>
   requires (
    (Array<Elements> && (sizeof...(ArrayElements) == ArraySize<Elements>))
    || ...
   )
   constexpr VariantStorage(Disambiguate d, ArrayElements&&... elements)
   noexcept :
    next{d, (ArrayElements&&)elements...}
   {}

   //Constexpr nop destructor
   inline constexpr ~VariantStorage() noexcept {}
  };

  //Union base specialization
  template<>
  union VariantStorage<> final {
   using ElementType = void;

   Never _;

   //Constexpr default constructor
   constexpr VariantStorage() noexcept :
    _{}
   {}

   //Constexpr nop destructor
   constexpr ~VariantStorage() noexcept {}

   //Constexpr nop copy-assignment operator
   constexpr VariantStorage& operator=(VariantStorage const&) noexcept {
    return *this;
   }
  };

  //Returns a reference to the union instance encapsulating `Element`
  template<typename Element, typename... Elements>
  requires MatchAnyType<Element, Elements...>
  constexpr auto& storageForElement(VariantStorage<Elements...>& root)
  noexcept {
   using StorageType = ConstDecayed<ReferenceDecayed<decltype(root)>>;
   if constexpr (SameType<Element, typename StorageType::ElementType>) {
    return root;
   } else {
    return storageForElement<Element>(root.next);
   }
  }

  namespace Internal {
   //Utility type for `createVariantStorage`
   //Constant-evaluated dispatcher for constant-evaluated allocation and
   //construction, and runtime in-place construction
   template<bool Allocate, typename... Elements>
   struct Creator final {
    VariantStorage<Elements...> * dst;

    template<typename... Args>
    constexpr auto operator()(Args... args) noexcept {
     if constexpr (Allocate) {
      //Allocate and construct
      return new VariantStorage<Elements...> {(Args)args...};
     } else {
      //Construct in-place
      return new (dst) VariantStorage<Elements...> {(Args)args...};
     }
    }
   };

   //Creates a `VariantStorage<...>` instance containing the given argument and
   //using the constructor lambda provided
   template<
    bool Allocate,
    typename E,
    typename T,
    typename... Elements
   >
   requires MatchAnyType<E, Elements...>
   constexpr auto createVariantStorage(
    T t,
    VariantStorage<Elements...> * dst = nullptr
   ) noexcept {
    using RDT = ConstDecayed<ReferenceDecayed<T>>;
    using ElementType = SelectType<
     Array<RDT>,
     StripArray<RDT>,
     RDT
    >;
    constexpr bool
     copy = SameType<T, E const&>,
     constructible = (copy
      ? CopyConstructible<ElementType>
      : MoveConstructible<ElementType>
     );

    //Note: Cannot use a lambda here due to a bug in clang related to nested
    //lambda invocations. See here: https://bugs.llvm.org/show_bug.cgi?id=49743
    Creator<Allocate, Elements...> creator{dst};
    if constexpr (Array<E>) {
     //Handle array types
     if constexpr (constructible) {
      //Copy or move construct array elements
      return unravelAndInvoke(
       (T)t,
       [&]<typename... ArrayElements>(ArrayElements... elements) {
        return creator.template operator()<Disambiguate, ArrayElements...>(
         Disambiguate{},
         (ArrayElements)elements...
        );
       }
      );
     } else {
      //Default construct and copy or move assign elements
      return creator.template operator()<T>((T)t);
     }
    } else {
     //Handle non-array types
     if constexpr (constructible) {
      //Copy or move construct element
      return creator.template operator()<Disambiguate, T>(
       Disambiguate{},
       (T)t
      );
     } else {
      //Default construct and copy or move assign element
      return creator.template operator()<T>((T)t);
     }
    }
   }

   //Walks `VariantStorage` hierarchy at runtime until halt condition is met
   //Note: `E = void` is an easy start condition since `void` it will never
   //occur as a `CX::Variant` member
   template<typename E = void>
   constexpr void walkVariantStorageImpl(
    auto& storage,
    auto& visitor
   ) noexcept {
    using StorageType = ConstVolatileDecayed<
     ReferenceDecayed<decltype(storage)>
    >;
    if constexpr (!SameType<E, typename StorageType::ElementType>) {
     //Execute `visitor` with current storage instance
     if constexpr (SameType<
      bool,
      decltype(visitor(declval<StorageType&>()))
     >) {
      //Walk storage until one of the following conditions is met:
      // - `SameType<E, typename StorageType::ElementType>`
      // - `!visitor(storage)`
      auto const next = [&]<typename...>() {
       return visitor.template operator()<StorageType>(storage);
      }();
      if (next) {
       walkVariantStorageImpl(storage.next, visitor);
      }
     } else {
      //Walk until `SameType<E, typename StorageType::ElementType>`
      visitor.template operator()<StorageType>(storage);
      walkVariantStorageImpl(storage.next, visitor);
     }
    }
   }
  }

  //Shim to `Internal::createVariantStorage<true, E, T, Elements...>`
  template<
   typename E,
   typename T,
   typename... Elements
  >
  constexpr auto allocateVariantStorage(T t) noexcept {
   return Internal::createVariantStorage<
    true,
    E,
    T,
    Elements...
   >((T)t);
  }

  //Shim to `Internal::createVariantStorage<false, E, T, Elements...>`
  template<
   typename E,
   typename T,
   typename... Elements
  >
  constexpr auto constructVariantStorage(
   T t,
   VariantStorage<Elements...> * dst
  ) noexcept {
   return Internal::createVariantStorage<
    false,
    E,
    T,
    Elements...
   >((T)t, dst);
  }

  //Shim to `Internal::walkVariantStorage`
  template<typename E = void>
  constexpr void walkVariantStorage(auto& storage, auto visitor) noexcept {
   Internal::walkVariantStorageImpl<E>(storage, visitor);
  }

  namespace Internal {
   //Destructs the principal elements of an array. Can handle multi-dimensional
   //array types.
   template<typename T>
   constexpr void destructArray(T& array) noexcept {
    using ElementType = ArrayDecayed<T>;
    for (auto& element : array) {
     if constexpr (Array<ElementType>) {
      destructArray(element);
     } else {
      element.~ElementType();
     }
    }
   }
  }

  //Destructs the element `E` and all preceeding `VariantStorage` instances
  template<typename E, typename... Elements>
  constexpr void destroyVariantStorage(VariantStorage<Elements...>& storage)
  noexcept {
   auto& encapsulator = storageForElement<E>(storage);
   if constexpr (Array<E>) {
    Internal::destructArray(encapsulator.value);
   } else {
    encapsulator.value.~E();
   }
  }
 }

 //Forward declare `CX::Variant` for use with supporting meta-functions
 template<VariantElement... Elements>
 struct Variant;

 //Supporting meta-functions for `CX::Variant<...>`
 namespace VariantMetaFunctions {
  //`CX::Variant<...>` identity meta-function
  template<typename>
  struct IsVariant : FalseType {};

  template<
   template<typename...> typename MaybeVariant,
   typename... Elements
  >
  requires CX::SameTemplateType<Variant, MaybeVariant>
  struct IsVariant<MaybeVariant<Elements...>> : TrueType {};

  //Meta-function to check whether V2 is compatible with V1.
  //Note: This function is directional; V2 may be compatible with V1,
  //but not the other way around.
  //eg:
  // Variant<int, float> v1{1234};
  // Variant<int, float, double> v2;
  // v2 = v1; //ok, `v2` is a superset Variant of `v1`
  // v1 = v2; //not ok, `v1` is not a superset Variant of `v2`, missing
  //          //`double`
  template<typename, typename>
  struct CompatibleVariant : FalseType {};

  template<
   template<typename...> typename V1,
   typename... V1Elements,
   template<typename...> typename V2,
   typename... V2Elements
  >
  requires (
   sizeof...(V1Elements) <= sizeof...(V2Elements)
   && (MatchAnyType<V1Elements, V2Elements...> && ...)
  )
  struct CompatibleVariant<V1<V1Elements...>, V2<V2Elements...>> : TrueType {};
 }

 //`CX::Variant<...>` identity concept
 template<typename MaybeVariant>
 concept IsVariant = VariantMetaFunctions
  ::IsVariant<MaybeVariant>
  ::Value;

 //`CX::Variant<...> <-> CX::Variant<...>` compatibility concept
 template<typename V2, typename V1>
 concept CompatibleVariant = IsVariant<V1>
  && IsVariant<V2>
  && VariantMetaFunctions
   ::CompatibleVariant<V2, V1>
   ::Value;

 //`CX::Variant<...>` implementation
 template<VariantElement... Elements>
 struct Variant final {
  static_assert(!(IsVariant<Elements> || ...));

 private:
  template<VariantElement...>
  friend struct Variant;

  //Alias for the underlying storage union
  using StorageType = VariantMetaFunctions::VariantStorage<Elements...>;

  //Number of bits required to store an index into `Elements...`
  //Note: `+1` state for errored state and `+1` bit for constexpr backend flag
  static constexpr auto const StateBits =
   bitsForNDistinct(sizeof...(Elements) + 1) CX_CONSTEXPR_EXPR(+ 1);

  //Constexpr bit
  static constexpr auto const ConstexprBit = StateBits - 1;

  //TODO Use `CX::BitSet` when ready
  //Alias for the smallest unsigned integral type able to store `StateBits`
  //unique states
  using StateType = UnsignedIntegral<StateBits>;

  //Note:
  // - when `state & (1 << ConstexprBit) != 0`, `constexprStorage` is being
  //   used
  // - the active index is `~(~state | (1 << ConstexprBit))`, wipes the
  //   `constexprStorage` bit
  // - the errored state is if the active index is equal to the number of
  //   elements in the variant, ie. `activeIndex() == sizeof...(Elements)`
  StateType state : StateBits;

  //Note: For constant-evaluated contexts, dynamic allocation of the storage
  //union is used due to limitations imposed by [expr.const]p5.
  union {
   //Runtime variant backend
   StorageType storage;

   //Constant-evaluated variant backend
   CX_CONSTEXPR_EXPR(StorageType *constexprStorage;)
  };

  //Returns the state for a given active element
  template<MatchAnyType<Elements...> E>
  static constexpr StateType calculateState(
   bool setConstexprBit = isConstexpr()
  ) noexcept {
   (void)setConstexprBit;
   constexpr auto const state = IndexOfType<E, Elements...>;
   #ifdef CX_CONSTEXPR_SEMANTICS
    //Handle setting constexpr backend bit
    return setConstexprBit ? state | (1 << ConstexprBit) : state;
   #else
    //No constexpr support, no need to handle constexpr backend bit
    return state;
   #endif
  }

  //Returns the index of the active type
  constexpr StateType activeIndex() const noexcept {
   #ifdef CX_CONSTEXPR_SEMANTICS
    //Remove constexpr bit
    return ~(~state | (1 << ConstexprBit));
   #else
    //No constexpr support, no need to remove bit
    return state;
   #endif
  }

  //Returns whether or not this variant instance is using the constexpr backend
  constexpr bool usingConstexprBackend() const noexcept {
   #ifdef CX_CONSTEXPR_SEMANTICS
    return state & (1 << ConstexprBit);
   #else
    return false;
   #endif
  }

  //Returns whether or not this variant instance is in an errored state
  constexpr bool isErrored() const noexcept {
   return activeIndex() == (StateType)sizeof...(Elements);
  }

  //Returns mutable reference to `*this`
  constexpr auto& mut() const noexcept {
   return const_cast<Variant&>(*this);
  }

  //Returns reference to active storage backend
  constexpr auto& storageBackend() noexcept {
   #ifdef CX_CONSTEXPR_SEMANTICS
    return usingConstexprBackend() ? *constexprStorage : storage;
   #else
    return storage;
   #endif
  }

  //Shim to non-const version
  constexpr auto& storageBackend() const noexcept {
   return mut().storageBackend();
  }

  //Returns a reference to the storage instance for a given element type
  template<MatchAnyType<Elements...> E>
  constexpr auto& storageForElement() const noexcept {
   return VariantMetaFunctions::storageForElement<E>(storageBackend());
  }

  //Runs `op` on the active varaint element
  constexpr void elementOp(auto op) const noexcept {
   //Do nothing if in errored state
   if (!isErrored()) {
    VariantMetaFunctions::walkVariantStorage(
     storageBackend(),
     [&](auto& storageInst) constexpr noexcept {
      using StorageType = ConstDecayed<ReferenceDecayed<decltype(storageInst)>>;
      using ElementType = typename StorageType::ElementType;
      if (has<ElementType>()) {
       op(storageInst);
       return false;
      } else {
       return true;
      }
     }
    );
   }
  }

  //Destroys encapsulated element of type `E` and storage
  template<MatchAnyType<Elements...> E>
  constexpr void destroyStorageForElement() noexcept {
   //Do nothing if varaint is in an errored state
   if (!isErrored()) {
    auto const constexprBackend = usingConstexprBackend();
    (void)constexprBackend;

    //Destruct encapsulated element
    VariantMetaFunctions::destroyVariantStorage<E>(storageBackend());

    //Set state to errored, so no further operations are applied to the
    //uninitialized storage backend
    state = sizeof...(Elements);

    //Destruct and deallocate `VariantStorage` instance if using
    //constant-evaluated backend
    #ifdef CX_CONSTEXPR_SEMANTICS
     //Free constant-evaluated allocated variant storage
     if (constexprBackend) {
      delete constexprStorage;
     }
    #endif
   }
  }

  //Destroys encapsulated element and storage
  constexpr void destroyStorage() noexcept {
   elementOp([&](auto& storageInst) {
    using E = typename ConstVolatileDecayed<
     ReferenceDecayed<decltype(storageInst)>
    >::ElementType;
    destroyStorageForElement<E>();
   });
  }

  //Handles element copy or move construction
  template<MatchAnyType<Elements...> E, typename T>
  constexpr void constructStorageElement(T t) noexcept {
   if (isConstexpr()) {
    //Constant-evaluated variant backend
    #ifdef CX_CONSTEXPR_SEMANTICS
     //Deactivate placeholder runtime backend
     //storage.~StorageType();
     //Allocate storage instance
     constexprStorage = VariantMetaFunctions::allocateVariantStorage<
      E,
      T,
      Elements...
     >((T)t);
     state = calculateState<E>();
    #else
     //Exit if constant-evaluated variant backend is used without
     //`CX_CONSTEXPR_SEMANTICS`
     exit(NoConstexprSemanticsError{});
    #endif
   } else {
    //Runtime varaint backend
    VariantMetaFunctions::constructVariantStorage<
     E,
     T,
     Elements...
    >(
     (T)t,
     &storage
    );
    state = calculateState<E>();
   }
  }

  //Handles construction of an element from a placeholder errored state
  //Note: Only meant to be invoked by element constructors. Only `const`
  // qualified to reduce duplicated code in element constructors.
  template<MatchAnyType<Elements...> E, typename T>
  constexpr void handleElementConstruction(T t) const noexcept {
   auto& ref = mut();
   //Destroy placeholder union member
   ref.storage._.~Never();
   //Deactivate placeholder runtime backend
   ref.storage.~StorageType();
   //Construct element storage
   ref.template constructStorageElement<E, T>((T)t);
  }

  //Handles assignment of compatible variant types
  template<CompatibleVariant<Variant> V, typename T>
  constexpr void handleVariantAssignment(T t) noexcept {
   constexpr auto const copy = SameType<T, V const&>;
   if constexpr (SameType<V, Variant<>>) {
    //Handle assignment of empty variant specialization
    destroyStorage();
   } else {
    //Handle assignment of a compatible variant type
    if (t.isErrored()) {
     //If assigned variant is errored, reset to errored state
     destroyStorage();
    } else {
     //Copy or move initialize element
     t.elementOp([&](auto& storageInst) constexpr noexcept {
      using E = typename ConstVolatileDecayed<
       ReferenceDecayed<decltype(storageInst)>
      >::ElementType;
      using PromotionType = SelectType<
       copy,
       E const&,
       E&&
      >;
      handleElementAssignment<E, PromotionType>(
       (PromotionType)storageInst.value
      );
     });
     //If elemnt value is moved from assigned variant, reset assigned variant
     if constexpr (!copy) {
      t.destroyStorage();
     }
    }
   }
   (void)t;
   (void)copy;
  }

  //Handles initialization of an element from an assigned compatible variant
  //type when in a placeholder errored state
  //Note: Only meant to be invoked by element constructors. Only `const`
  // qualified to reduce duplicated code in element constructors.
  template<CompatibleVariant<Variant> V, typename T>
  constexpr void handleVariantConstruction(T t) const noexcept {
   auto& ref = mut();
   //Destroy placeholder union member
   ref.storage._.~Never();
   //Deactivate placeholder runtime backend
   ref.storage.~StorageType();
   //Handle variant assignment
   ref.template handleVariantAssignment<V, T>((T)t);
  }

  //TODO move to `VariantMetaFunctions`
  //Recursively descends to the lowest nested array depth to copy or move
  //assign component elements
  template<
   bool Copy,
   typename A,
   typename B
  >
  constexpr void assignStorageArrayElement(A& dst, B& src) noexcept {
   static_assert(Array<A>
    && SameType<A, ConstVolatileDecayed<B>>
    && (ArraySize<A> == ArraySize<B>)
   );
   if constexpr (Array<ArrayDecayed<A>>) {
    //Recursively descend
    for (SizeType i = 0; i < ArraySize<A>; i++) {
     assignStorageArrayElement<Copy>(dst[i], src[i]);
    }
   } else {
    //Copy or move assign all component elements
    using E = ArrayDecayed<A>;
    using PromotionType = SelectType<Copy, E const&, E&&>;
    for (SizeType i = 0; i < ArraySize<A>; i++) {
     dst[i] = (PromotionType)src[i];
    }
   }
  }

  //Handles updating an existing element
  template<MatchAnyType<Elements...> E, typename T>
  constexpr void assignStorageElement(T t) noexcept {
   constexpr auto const copy = SameType<T, E const&>;
   auto& storageInst = storageForElement<E>();
   if constexpr (Array<E>) {
    //Handle array element types
    assignStorageArrayElement<copy>(storageInst.value, t);
   } else {
    //Handle non-array element types
    storageInst.value = (T)t;
   }
  }

  //Handles element assignment operations for non-array element types
  template<MatchAnyType<Elements...> E, typename T>
  constexpr void handleElementAssignment(T t) noexcept {
   //Yields the array component type if `E` is an array, otherwise `E`
   using ComponentType = SelectType<
    Array<E>,
    VariantMetaFunctions::StripArray<E>,
    E
   >;
   //Whether or not to update the encapsulated element, if present, through
   //either copy or move assignment operators
   constexpr auto const assignable = SameType<T, E const&>
    ? CopyAssignable<ComponentType>
    : MoveAssignable<ComponentType>;

   auto const sameType = has<E>();
   if (sameType) {
    //Handle potential optimizations for assigning to the same element type
    if constexpr (assignable) {
     //Same element type and copy/move assignable, update existing encapsulated
     //element
     assignStorageElement<E, T>((T)t);
    } else {
     //Same element type but not copy/move assignable, construct new
     //encapsulated element storage

     //Destroy encapsulated element and storage instance
     destroyStorageForElement<E>();
     //Create new encapsulated element storage
     constructStorageElement<E, T>((T)t);
    }
   } else {
    //Destroy any existing encapsulated elemnt and storage, and construct
    //new encapsulated element storage

    //Destroy encapsulated element and storage instance
    destroyStorage();
    //Create new encapsulated element storage
    constructStorageElement<E, T>((T)t);
   }
  }

 public:
  //Default constructor
  //Note: Initializes variant to errored state
  constexpr Variant() noexcept :
   state{sizeof...(Elements)},
   storage{Never{}}
  {
   auto& ref = mut();
   //Destroy placeholder union member
   ref.storage._.~Never();
   //Deactivate placeholder runtime backend
   ref.storage.~StorageType();
  }

  //Element copy-constructor
  //Note: Default initializes to errored state in-case of error during
  //construction
  template<MatchAnyType<Elements...> E>
  constexpr Variant(E const& e) noexcept :
   state{sizeof...(Elements)},
   storage{Never{}}
  {
   handleElementConstruction<E, E const&>((E const&)e);
  }

  //Element move-constructor
  //Note: Default initializes to errored state in-case of error during
  //construction
  template<MatchAnyType<Elements...> E>
  constexpr Variant(E&& e) noexcept :
   state{sizeof...(Elements)},
   storage{Never{}}
  {
   handleElementConstruction<E, E&&>((E&&)e);
  }

  //Variant copy-constructor
  //Note: Default initializes to errored state in-case of error during
  //construction
  constexpr Variant(Variant const& other) noexcept :
   state{sizeof...(Elements)},
   storage{Never{}}
  {
   handleVariantConstruction<Variant, Variant const&>((Variant const&)other);
  }

  //Variant move-constructor
  //Note: Default initializes to errored state in-case of error during
  //construction
  constexpr Variant(Variant&& other) noexcept :
   state{sizeof...(Elements)},
   storage{Never{}}
  {
   handleVariantConstruction<Variant, Variant&&>((Variant&&)other);
  }

  //CompatibleVariant copy-constructor
  template<CompatibleVariant<Variant> V>
  constexpr Variant(V const& other) noexcept :
   state{sizeof...(Elements)},
   storage{Never{}}
  {
   handleVariantConstruction<V, V const&>((V const&)other);
  }

  //CompatibleVariant move-constructor
  template<CompatibleVariant<Variant> V>
  constexpr Variant(V&& other) noexcept :
   state{sizeof...(Elements)},
   storage{Never{}}
  {
   handleVariantConstruction<V, V&&>((V&&)other);
  }

  //Destroys encapsulated element and variant storage
  constexpr ~Variant() noexcept {
   destroyStorage();
  }

  //Element copy-assignment operator
  template<MatchAnyType<Elements...> E>
  //requires (!Array<E>)
  constexpr Variant& operator=(E const& e) noexcept {
   handleElementAssignment<E, E const&>((E const&)e);
   return *this;
  }

  //Element move-assignment operator
  template<MatchAnyType<Elements...> E>
  constexpr Variant& operator=(E&& e) noexcept {
   handleElementAssignment<E, E&&>((E&&)e);
   return *this;
  }

  //Variant copy-assignment operator
  constexpr Variant& operator=(Variant const& other) noexcept {
   handleVariantAssignment<Variant, Variant const&>((Variant const&)other);
   return *this;
  }

  //Variant move-assignment operator
  constexpr Variant& operator=(Variant&& other) noexcept {
   handleVariantAssignment<Variant, Variant&&>((Variant&&)other);
   return *this;
  }

  //CompatibleVariant copy-assignment operator
  template<CompatibleVariant<Variant> V>
  constexpr Variant& operator=(V const& v) noexcept {
   handleVariantAssignment<V, V const&>((V const&)v);
   return *this;
  }

  //CompatibleVariant move-assignment operator
  template<CompatibleVariant<Variant> V>
  constexpr Variant& operator=(V&& v) noexcept {
   handleVariantAssignment<V, V&&>((V&&)v);
   return *this;
  }

  //Element presence check by element type
  template<MatchAnyType<Elements...> E>
  constexpr bool has() const noexcept {
   return activeIndex() == (StateType)IndexOfType<E, Elements...>;
  }

  //Element presence check by element type index
  template<StateType I>
  requires (I < (StateType)sizeof...(Elements))
  constexpr bool has() const noexcept {
   return activeIndex() == I;
  }

  //Unchecked element decapsulation by element type
  template<MatchAnyType<Elements...> E>
  constexpr E& get() const noexcept {
   return storageForElement<E>().value;
  }

  //Unchecked element decapsulation by element type index
  template<StateType I>
  requires (I < (StateType)sizeof...(Elements))
  constexpr auto& get() const noexcept {
   using E = TypeAtIndex<I, Elements...>;
   return storageForElement<E>().value;
  }

  //Copy-insertion by element type index
  template<StateType I>
  requires (I < (StateType)sizeof...(Elements))
  constexpr void set(TypeAtIndex<I, Elements...> const& e) noexcept {
   operator=(copy(e));
  }

  //Move-insertion by element type index
  template<StateType I>
  requires (I < (StateType)sizeof...(Elements))
  constexpr void set(TypeAtIndex<I, Elements...>&& e) noexcept {
   operator=(move(e));
  }

  //TODO Checked element decapsulation (by element type and by element type
  // index)
 };

 //Empty variant specialization
 template<>
 struct Variant<> final : Never {
  template<VariantElement...>
  friend struct Variant;

  //Default no-element constructor
  constexpr Variant() noexcept = default;

  //Illegal element copy-constructor
  template<typename E>
  constexpr Variant(E const&) noexcept = delete;

  //Illegal element move-constructor
  template<typename E>
  constexpr Variant(E&&) noexcept = delete;

  //Default variant copy-constructor
  constexpr Variant(Variant const&) noexcept = default;

  //Default variant move-constructor
  constexpr Variant(Variant&&) noexcept = default;

  //CompatibleVariant copy-constructor
  template<CompatibleVariant<Variant> V>
  constexpr Variant(V const&) noexcept {}

  //CompatibleVariant move-constructor
  template<CompatibleVariant<Variant> V>
  constexpr Variant(V&&) noexcept {}

  //Default destructor
  constexpr ~Variant() noexcept = default;

  //Illegal element copy-assignment operator
  template<typename E>
  constexpr Variant& operator=(E const&) noexcept {
   exit(VariantTypeError{});
   return *this;
  }

  //Illegal element move-assignment operator
  template<typename E>
  constexpr Variant& operator=(E&&) noexcept {
   exit(VariantTypeError{});
   return *this;
  }

  //Default varaint copy-assignment operator
  constexpr Variant& operator=(Variant const&) noexcept = default;

  //Default variant move-assignment operator
  constexpr Variant& operator=(Variant&&) noexcept = default;

  //CompatibleVariant copy-assignment operator
  template<CompatibleVariant<Variant> V>
  constexpr Variant& operator=(V const&) noexcept {
   return *this;
  }

  //CompatibleVariant move-assignment operator
  template<CompatibleVariant<Variant> V>
  constexpr Variant& operator=(V&&) noexcept {
   return *this;
  }

  //Element presence check by type
  template<typename>
  constexpr bool has() const noexcept {
   return false;
  }

  //Element presence check by index
  template<auto>
  constexpr bool has() const noexcept {
   return false;
  }

  //Illegal unchecked element decapsulation by element type
  template<typename T>
  constexpr T& get() const noexcept {
   exit(VariantTypeError{});
  }

  //Illegal unchecked element decapsulation by element type index
  template<auto>
  constexpr auto& get() const noexcept {
   exit(VariantTypeError{});
  }
 };

 //CX::Variant deduction guides
 Variant() -> Variant<>;

 /*TODO fix guide
 template<typename T>
 Variant(T) -> Variant<T>;
 */
}
