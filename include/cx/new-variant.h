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
 namespace Internal {
  //Yields the element type for array types, `ArrayDecayed<T>`,  and `T` for
  //all other types
  //Note: Short for `VariantElementType`
  template<typename T>
  using VET = SelectType<Array<T>, ArrayDecayed<T>, T>;
 }

 //Varient element type identity concept
 //Does not allow:
 // - `void`
 // - unsized array types
 // - types that are not at least one of:
 //   - copy-constructible OR default-constructible and copy-assignable
 //   - move-constructible OR default-constructible and move-assignable
 template<typename E>
 concept VariantElement = !(SameType<E, void> || UnsizedArray<E>)
  && (Constructible<Internal::VET<E>>
   ? (CopyAssignable<Internal::VET<E>> || MoveAssignable<Internal::VET<E>>)
   : (CopyConstructible<Internal::VET<E>> || MoveAssignable<Internal::VET<E>>)
  );

 //Forward declare variant for use with supporting variant meta-functions
 //Note: Disallows duplicated elements
 template<VariantElement... Elements>
 requires UniqueTypes<Elements...>
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

  template<
   typename Q1,
   template<typename...> typename V1,
   typename... V1Elements
  >
  requires (CX::IsVariant<Q1>)
  struct VariantCompatibilityBase<Q1, V1<V1Elements...>> {
   //Meta-function to check whether V2 is compatible with V1.
   //Note: This function is directional; V2 may be compatible with V1,
   //but not the other way around.
   //eg:
   // Variant<int, float> v1{1234};
   // Variant<int, float, double> v2;
   // v2 = v1; //ok, `v2` is a superset Variant of `v1`
   // v1 = v2; //not ok, `v1` is not a superset Variant of `v2`, missing
   //          //`double`
   template<typename V2, typename = Unqualified<V2>>
   struct IsCompatible : FalseType {};
  };

  template<
   typename Q1,
   template<typename...> typename V1,
   typename... V1Elements
  >
  requires (CX::IsVariant<Q1>)
  template<
   typename Q2,
   template<typename...> typename V2,
   typename... V2Elements
  >
  requires (CX::IsVariant<Q2>)
  struct VariantCompatibilityBase<
   Q1,
   V1<V1Elements...>>::IsCompatible<Q2, V2<V2Elements...>
  > {
   //For V2 to be compatible with V1, the element set for V2 must be a superset
   //of V1's element set.
   static constexpr auto const Value =
    sizeof...(V2Elements) >= sizeof...(V1Elements)
     && (
      MatchAnyType<Unqualified<V1Elements>, Unqualified<V2Elements>...> && ...
     );
  };
 }

 //Concept to check whether or not `V2` is compatible with `V1`.
 template<typename V1, typename V2>
 concept CompatibleVariant = VariantMetaFunctions
  ::VariantCompatibilityBase<V1>
  ::template IsCompatible<V2>
  ::Value;

 namespace VariantMetaFunctions {
  namespace Internal {
   //Utilities for `VariantElementMatcher`
   template<typename T, typename Match>
   struct Matches : CX
    ::MetaFunctions
    ::SameType<Match, T>
   {};

   template<typename T, typename Match>
   requires Array<T>
   struct Matches<T, Match> : CX
    ::MetaFunctions
    ::MatchAnyType<Match, T, T const&, T&&>
   {};
  }

  //Matches `Match` against any of `Elements...`. For any element of an array
  //type, matching is performed explicitly against array l/r-value references
  //to avoid automatic array-to-pointer demotions.
  template<typename Match, typename... Elements>
  concept VariantElementMatcher = (Internal
   ::Matches<Match, Elements>
   ::Value || ...
  );

  //TODO
  //Companion type meta-function to `VariantElementMatcher`; given an array
  //type matching `T const(&)[N]` or `T(&&)[N]`, yield the original array type
  //specified in the variant specialization.
  template<typename T, typename... Elements>
  using VariantElementResolver = void;

  //Variant union base
  template<typename... Elements>
  union VariantStorage;

  namespace Internal {
   template<typename Maybe, typename = Unqualified<Maybe>>
   struct IsVariantStorage : CX::FalseType {};

   template<
    typename Maybe,
    template<typename...> typename U,
    typename... Elements
   >
   struct IsVariantStorage<Maybe, U<Elements...>> :
    MetaFunctions::SameTemplateType<U, VariantStorage>
   {};
  }

  //`VariantStorage` identity concept
  template<typename Maybe>
  concept IsVariantStorage = Internal
   ::IsVariantStorage<Maybe>
   ::Value;

  //Union impl
  template<typename E, typename... Elements>
  union VariantStorage<E, Elements...> final {
   using ElementType = E;

   //Intermediate state
   Never _;
   //Value
   E value;
   //Next value
   VariantStorage<Elements...> next;

   //Constexpr intermediate state constructor
   constexpr VariantStorage() noexcept :
    _{}
   {}

   //Constexpr nop destructor
   constexpr ~VariantStorage() noexcept {}

   //Returns reference to value
   constexpr auto& getValue() noexcept {
    return value;
   }
  };

  template<typename E, typename... Elements>
  requires Array<E>
  union VariantStorage<E, Elements...> final {
   using ElementType = E;

   //Intermediate state
   Never _;
   //Value
   struct {
    Never _;
    E value;
   } wrapper;
   //Next
   VariantStorage<Elements...> next;

   //Constexpr intermediate state constructor
   constexpr VariantStorage() noexcept :
    _{}
   {}

   //Constexpr nop destructor
   constexpr ~VariantStorage() noexcept {}

   //Returns reference to value
   constexpr auto& getValue() noexcept {
    return wrapper.value;
   }
  };

  //Base union specialization
  template<>
  union VariantStorage<> final {
   using ElementType = void;

   Never _{};

   //Constexpr nop default constructor
   constexpr VariantStorage() noexcept {}

   //Constexpr nop destructor
   constexpr ~VariantStorage() noexcept {}
  };

  namespace Internal {
   //Utility type for `UnionDefaultDestructible` (see below)
   template<typename...>
   union U;

   //Union impl
   template<typename E, typename... Elements>
   union U<E, Elements...> final {
   private:
    E e;
    U<Elements...> next;

    constexpr void unused() noexcept {
     (void)e;
     (void)next;
    }
   };

   //Base union specialiation
   template<>
   union U<> final {
   private:
   };
  }

  //Concept to determine whether or not `VariantStorage<...>` members need
  //to be destructed manually
  template<typename... Elements>
  concept UnionDefaultDestructible = Destructible<Internal::U<Elements...>>;

  //Walks union hierarchy at runtime until halt condition is met
  //Note: `E = void` is an easy start condition since `void` it will never
  //occur as a `CX::Variant` member
  template<typename E = void>
  constexpr void walkStorage(IsVariantStorage auto& storage, auto& visitor)
   noexcept
  {
   using StorageType = ReferenceDecayed<decltype(storage)>;
   if constexpr (!SameType<E, typename StorageType::ElementType>) {
    //Execute `visitor` with current storage instance
    if constexpr (SameType<
     bool,
     decltype(visitor(declval<StorageType&>()))
    >) {
     //Walk until one of the following conditions is met:
     // - `SameType<E, typename StorageType::ElementType>`
     // - `!visitor(storage)`
     auto const next = [&]<typename...>() {
      return visitor.template operator()<StorageType>(storage);
     }();
     if (next) {
      walkStorage(storage.next, visitor);
     }
    } else {
     //Walk until `SameType<E, typename StorageType::ElementType>`
     visitor.template operator()<StorageType>(storage);
     walkStorage(storage.next, visitor);
    }
   }
  }

  //Returns the storage with a matching member type
  template<typename E>
  constexpr auto& storageForElement(IsVariantStorage auto& storage) noexcept {
   using ElementType = typename ReferenceDecayed<decltype(storage)>
    ::ElementType;
   static_assert(
    !SameType<ElementType, void>,
    "Storage does not contain an element of type E"
   );
   if constexpr (SameType<E, ElementType>) {
    return storage;
   } else {
    return storageForElement<E>(storage.next);
   }
  }

  //Concept for variant element copy constructor / copy-assignment operators
  template<typename T>
  concept VariantElementCopyInitable = (Array<T>
   ? (CopyConstructible<ArrayDecayed<T>> || (
    Constructible<ArrayDecayed<T>> && CopyAssignable<ArrayDecayed<T>>
   ))
   : (CopyConstructible<T> || (Constructible<T> && CopyAssignable<T>))
  );

  //Concept for variant element move constructor / move-assignment operators
  template<typename T>
  concept VariantElementMoveInitable = (Array<T>
   ? (MoveConstructible<ArrayDecayed<T>> || (
    Constructible<ArrayDecayed<T>> && MoveAssignable<ArrayDecayed<T>>
   ))
   : (MoveConstructible<T> || (Constructible<T> && MoveAssignable<T>))
  );
 }

 //Supporting exceptions

 //Error for evaluating a variant with a non-present type
 struct VariantTypeError final {
  constexpr auto& describe() const noexcept {
   return "Variant type not present";
  }
 };
 static_assert(IsError<VariantTypeError>);

 //Error for illegal variant operations in constant-evaluated contexts
 struct ConstexprVariantTypeError final {
  constexpr auto& describe() const noexcept {
   return "To use `CX::Variant<...>` in a constant-evaluated context, all"
    "elements must be default constructible and copy or move assignable";
  }
 };
 static_assert(IsError<ConstexprVariantTypeError>);

 //Error for invalid runtime variant conversions
 struct IncompatibleVariantError final {
  constexpr auto& describe() const noexcept {
   return "Variant types are not convertible";
  }
 };
 static_assert(IsError<IncompatibleVariantError>);

 //Variant impl
 template<VariantElement... Elements>
 requires UniqueTypes<Elements...>
 struct Variant final {
  template<VariantElement... Types>
  requires UniqueTypes<Types...>
  friend struct Variant;

 private:
  static constexpr SizeType TagBits = sizeof...(Elements);
  using TagType = SignDecayed<decltype(1 << TagBits)>;

  template<typename E>
  static constexpr SizeType IndexOfElement = IndexOfType<E, Elements...>;

  //Bitfield union tag
  TagType tag : TagBits{};
  //Element storage union
  VariantMetaFunctions::VariantStorage<Elements...> storage{};

  //Returns mutable reference to instance
  constexpr Variant& mut() const noexcept {
   return const_cast<Variant&>(*this);
  }

  //Walks the storage until a stop condition is met
  template<typename E = void>
  constexpr void walkStorage(auto visitor) noexcept {
   VariantMetaFunctions::walkStorage<E>(storage, visitor);
  }

  //Returns the storage instance containing the element of type `E`
  template<typename E>
  constexpr auto& storageForElement() noexcept {
   return VariantMetaFunctions::storageForElement<E>(storage);
  }

  template<typename E>
  constexpr auto& storageForElement() const noexcept {
   return VariantMetaFunctions::storageForElement<E>(storage);
  }

  //Destructs currently encapsulated element and re-initializes storage in
  //intermediate state
  constexpr void reset() noexcept {
   //Do not invoke active member destructor if a union of `Elements` is already
   //default destructible
   if constexpr (!VariantMetaFunctions::UnionDefaultDestructible<Elements...>)
   {
    //Do invoke any destructor if there is no active member
    if (tag) {
     //Walk the storage until the active member is reached
     walkStorage<>([&]<typename S>(S& s) {
      using E = typename S::ElementType;
      if (has<E>()) {
       //Destroy encapsulated element
       if constexpr (Destructible<E> && !TriviallyDestructible<E>) {
        //Invoke element destructor if it is not an array and is not trivially
        //destructible
        s.value.~E();
       } else if constexpr (Array<E>) {
        using ArrayE = ArrayDecayed<E>;
        if constexpr (Destructible<ArrayE> && !TriviallyDestructible<ArrayE>) {
         //Invoke array element destructors if they are not trivially
         //destructible
         for (auto& e : s.wrapper.value) {
          e.~ArrayE();
         }
        }
       }
       return false;
      }
      return true;
     });
    }
   }
   //Reset tag
   tag = 0;
   //Re-initialize storage in intermediate state
   storage._ = {};
  }

  //Updates the member `E` in the appropriate storage union
  //Note: Does not check for presence of value
  template<MatchAnyType<Elements...> E, typename T>
  requires (MatchAnyType<T, E const&, E&&>)
  constexpr void update(T e) noexcept {
   constexpr bool const array = Array<E>;
   using ElementType = SelectType<array, ArrayDecayed<E>, E>;

   constexpr bool const
    copy = SameType<T, E const&>,
    assignable = copy
     ? CopyAssignable<ElementType>
     : MoveAssignable<ElementType>;

   static_assert(
    assignable,
    "Cannot update element because it is either not move or copy assignable"
   );

   //Update active member
   if constexpr (array) {
    constexpr auto const size = ArraySize<E>;
    auto& s = storageForElement<E>();
    //Initalize wrapper
    s.wrapper = {};
    //Handle array element copy or move
    for (SizeType i = 0; i < size; i++) {
     if constexpr (copy) {
      //Handle array element copy assignment
      s.wrapper.value[i] = (ElementType const&)e[i];
     } else {
      //Handle array element move assignment
      s.wrapper.value[i] = (ElementType&&)e[i];
     }
    }
   } else {
    //Handle non-array element copy or move
    storageForElement<E>().value = (T)e;
   }
  }

  //Activates the member `E` in the appropriate storage union
  template<MatchAnyType<Elements...> E, typename T>
  requires (MatchAnyType<T, E const&, E&&>)
  constexpr void init(T e) noexcept {
   constexpr bool const array = Array<E>;
   using ElementType = SelectType<array, ArrayDecayed<E>, E>;

   constexpr bool const
    copy = SameType<T, E const&>,
    constructible = copy
     ? CopyConstructible<ElementType>
     : MoveConstructible<ElementType>,
    assignable = Constructible<ElementType> && (copy
     ? CopyAssignable<ElementType>
     : MoveAssignable<ElementType>
    );

   using PromotionType = SelectType<copy, ElementType const&, ElementType&&>;

   //Update active member
   if (!isConstexpr()) {
    auto& s = storageForElement<E>();
    auto const handleElementInit = [&](ElementType& toActivate) {
     //Placement new can be used when not being constant-evaluated
     if constexpr (constructible) {
      //Copy/move construct element
      new (&toActivate) ElementType{(PromotionType)e};
     } else if constexpr (assignable) {
      //Default construct and copy/move-assign element
      *new (&toActivate) ElementType{} = (PromotionType)e;
     }
    };

    //Initialize encapsulated element
    if constexpr (array) {
     //Initialize array
     constexpr auto const size = ArraySize<E>;
     s.wrapper._ = {};
     for (SizeType i = 0; i < size; i++) {
      handleElementInit(s.wrapper.value[i]);
     }
    } else {
     //Initialize element
     handleElementInit(s.value);
    }
   } else {
    //Placement new cannot be used when being constant-evaluated
    if constexpr (assignable) {
     //Walk storage and initialize next union until desired union is reached
     walkStorage([&]<typename S>(S& s) {
      if constexpr (SameType<E, typename S::ElementType>) {
       if constexpr (array) {
        //Default construct and copy/move-assign each array element
        //s.wrapper = {};
        s = {.wrapper = {}};
        constexpr auto size = ArraySize<E>;
        for (SizeType i = 0; i < size; i++) {
         s.wrapper.value[i] = (PromotionType)e[i];
        }
       } else {
        //Default construct (implicitly) and copy/move-assign element
        s.value = (PromotionType)e;
       }
       return false;
      } else {
       //Initialize next storage union
       s.next = {};
       return true;
      }
     });
    } else {
     exit(ConstexprVariantTypeError{});
    }
   }
   //Update tag
   tag = 1 << IndexOfElement<E>;
  }

  //Handle copy or move from a compatible variant type
  template<CompatibleVariant<Variant> V, typename T>
  requires (MatchAnyType<T, V const&, V&&>)
  constexpr void initFromVariant(T other) noexcept {
   bool updated = false;
   //Only initialize storage if other variant has an active member
   if (other.tag) {
    other.walkStorage([&]<typename S>(S& s) {
     using E = typename S::ElementType;
     if (other.template has<E>()) {
      if constexpr (SameType<T, V const&>) {
       //Handle copy
       operator=((E const&)s.getValue());
      } else if constexpr (SameType<T, V&&>) {
       //Handle move
       operator=((E&&)s.getValue());
      }
      /*
      if (has<E>() && CopyAssignable<E>) {
       //Copy-assign active element
       update<E, E const&>(s.value);
      } else {
       //Reset current state and initialize storage with new member
       reset();
       init<E, E const&>(s.value);
      }
      */
      updated = true;
      return false;
     }
     return true;
    });
    //Reset other variant if it was moved
    if constexpr (SameType<T, V&&>) {
     other.reset();
    }
   }
   //Reset state if it has not been updated
   if (!updated) {
    reset();
   }
  }

 public:
  //Empty variant constructor
  constexpr Variant() noexcept = default;

  //Variant element forwarding construction constructor
  //Note: Constructs the first element type that accepts `Args`
  template<typename... Args>
  requires (
   /*(Constructible<Elements, Args...>) || ...*/
   (Constructible<Elements, Args...> && !Array<Elements>) || ...
  )
  constexpr Variant(Args&&... args) noexcept {
   TypeIterator<Elements...>::run([&]<typename Element>() {
    if constexpr (Constructible<Element, Args...>) {
     //Prefer moving over copying when possible
     if constexpr (
      MoveConstructible<Element>
      || (Constructible<Element> && MoveAssignable<Element>)
     ) {
      //Move init element
      init<Element, Element&&>(Element{args...});
     } else if constexpr (
      CopyConstructible<Element>
      || (Constructible<Element> && CopyAssignable<Element>)
     ) {
      //Copy init element
      auto e = Element{args...};
      init<Element, Element const&>(e);
     }
     return false;
    }
    return true;
   });
  }

  //Variant element copy constructor
  template<VariantMetaFunctions::VariantElementMatcher<Elements...> E>
  requires VariantMetaFunctions::VariantElementCopyInitable<E>
  constexpr Variant(E const& e) noexcept {
   mut().operator=((E const&)e);
  }

  //Variant element move constructor
  template<VariantMetaFunctions::VariantElementMatcher<Elements...> E>
  requires VariantMetaFunctions::VariantElementMoveInitable<E>
  constexpr Variant(E&& e) noexcept {
   mut().operator=((E&&)e);
  }

  //Variant array element move constructor
  //Note: Only a viable overload when `Elements...` contains an array type
  template<typename T, auto N>
  requires (VariantMetaFunctions::VariantElementMatcher<T[N], Elements...>
   && VariantMetaFunctions::VariantElementMoveInitable<T>
  )
  constexpr Variant(T (&&array)[N]) noexcept {
   using PromotionType = T(&&)[N];
   mut().operator=((PromotionType)array);
  }

  //Variant copy constructor
  constexpr Variant(Variant const& other) noexcept {
   mut().opreator=((Variant const&)other);
  }

  //Variant move constructor
  constexpr Variant(Variant&& other) noexcept {
   mut().opreator=((Variant&&)other);
  }

  //CompatibleVariant copy constructor
  template<CompatibleVariant<Variant> V>
  constexpr Variant(V const& other) noexcept {
   mut().operator=((V const&)other);
  }

  //CompatibleVariant move constructor
  template<CompatibleVariant<Variant> V>
  constexpr Variant(V&& other) noexcept {
   mut().operator=((V&&)other);
  }

  //Constexpr destructor
  constexpr ~Variant() noexcept {
   //Reset current state, if any
   mut().reset();
  }

  //Element copy-assignment operator
  template<MatchAnyType<Elements...> E>
  constexpr Variant& operator=(E const& e) noexcept {
   if (has<E>() && CopyAssignable<E>) {
    //Copy-assign active member rather than performing an expensive reset and
    //initialization
    update<E, E const&>(e);
   } else {
    //Reset current state and initialize storage with new member
    reset();
    init<E, E const&>(e);
   }
   return *this;
  }

  //Element move-assignment operator
  template<MatchAnyType<Elements...> E>
  constexpr Variant& operator=(E&& e) noexcept {
   if (has<E>() && MoveAssignable<E>) {
    //Move-assign active member rather than performing an expensive reset and
    //initialization
    update<E, E&&>((E&&)e);
   } else {
    //Reset current state and initialize storage with new member
    reset();
    init<E, E&&>((E&&)e);
   }
   return *this;
  }

  //Variant copy-assignment operator
  constexpr Variant& operator=(Variant const& other) noexcept {
   initFromVariant<Variant, Variant const&>(other);
   return *this;
  }

  //Variant move-assignment operator
  constexpr Variant& operator=(Variant&& other) noexcept {
   initFromVariant<Variant, Variant&&>((Variant&&)other);
   return *this;
  }

  //CompatibleVariant copy-assignment operator
  template<CompatibleVariant<Variant> V>
  constexpr Variant& operator=(V const& other) noexcept {
   initFromVariant<V, V const&>(other);
   return *this;
  }

  //CompatibleVariant move-assignment operator
  template<CompatibleVariant<Variant> V>
  constexpr Variant& operator=(V&& other) noexcept {
   initFromVariant<V, V&&>((V&&)other);
   return *this;
  }

  //Implcit conversion to compatible variant types
  template<typename V>
  requires CompatibleVariant<V, Variant>
  explicit operator V() const noexcept {
   return V{(Variant const&)*this};
  }

  //Element presence check
  template<MatchAnyType<Elements...> E>
  constexpr bool has() const noexcept {
   return tag & (1 << IndexOfElement<E>);
  }

  //Returns a reference to the currently active member or fails
  template<MatchAnyType<Elements...> E>
  constexpr auto& get() const noexcept {
   if (has<E>()) {
    auto& s = storageForElement<E>();
    if constexpr (Array<E>) {
     return s.wrapper.value;
    } else {
     return s.value;
    }
   }
   exit(VariantTypeError{});
  }

  //Returns a reference to the currently active member or fails
  template<SizeType I>
  requires (I < sizeof...(Elements))
  constexpr auto& get() const noexcept {
   using Type = TypeAtIndex<I, Elements...>;
   return get<Type>();
  }
 };

 /*
 //TODO Invariant variant impl
 template<VariantElement T>
 struct Variant<T> final {
  template<VariantElement... Elements>
  requires UniqueTypes<Elements...>
  friend struct Variant;

 private:
 public:

 };
 */

 //TODO Empty variant impl
 template<>
 struct Variant<> final {
  template<VariantElement... Elements>
  requires UniqueTypes<Elements...>
  friend struct Variant;

 private:

 public:

 };

 //Variant deduction guides
 Variant() -> Variant<>;
 template<typename... Elements>
 Variant(Elements...) -> Variant<Elements...>;
}
