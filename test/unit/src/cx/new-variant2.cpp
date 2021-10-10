#include <cx/test/common/common.h>

#include <cx/new-variant2.h>

namespace CX {
 //Tests for `VariantStorage<...>`
 namespace VariantMetaFunctions {
  //Common test body for `createVariantStorage` for non-array types
  template<typename E, typename T>
  requires (!Array<E>)
  constexpr auto createVariantStorageTestProducer(auto verifier) noexcept {
   //Returns lambda that runs `body` for both constant-evaluated and runtime
   //paths
   return [=]<typename... VariantTypes> {
    //Test body
    constexpr auto body = [=]() constexpr {
     //Create element instance
     auto element = E{};
     //Create instance of variant storage
     auto storage = allocateVariantStorage<
      E,
      T,
      VariantTypes...
     >((T)element);
     //Invoke verifier
     auto& encapsulator = storageForElement<E>(*storage);
     verifier(encapsulator);
     //Destroy encapsulated element
     encapsulator.value.~E();
     //Destroy storage instance
     delete storage;
     return 0;
    };

    //Run test body
    //Constant-evaluated test path
    constexpr auto r = body();
    (void)r;
    //Runtime test path
    (void)body();
   };
  }

  //Common test body for `createVariantStorage` array types
  template<typename E, typename T>
  requires Array<E>
  constexpr auto createVariantStorageTestProducer(auto verifier) noexcept {
   return [=]<typename... VariantTypes> {
    using ElementType = ArrayDecayed<E>;
    //Test body
    constexpr auto body = [=]() constexpr {
     //Create elements
     E elements{};
     //Create instance of variant storage
     auto storage = allocateVariantStorage<
      E,
      T,
      VariantTypes...
     >((T)elements);
     //Invoke verifier
     auto& encapsulator = storageForElement<E>(*storage);
     verifier(encapsulator);
     //Destroy encapsulated elements
     for (auto& element : encapsulator.value) {
      element.~ElementType();
     }
     //Destroy storage instance
     delete storage;
     return 0;
    };

    //Run test body
    //Constant-evaluated test path
    constexpr auto r = body();
    (void)r;
    //Runtime test path
    (void)body();
   };
  }

  //TODO test multi-dimensional array types
  //`createVariantStorage` tests
  TEST(createVariantStorage, copy_constructible_element_is_copy_constructed) {
   using Type = CopyConstructibleType;

   constexpr auto test = createVariantStorageTestProducer<Type, Type const&>(
    [](auto& storage) constexpr {
     CX_GTEST_SHIM(EXPECT_TRUE, storage.value.copyConstructed);
    }
   );

   //Test when `Type` is the first encapsulated type
   test.template operator()<Type, float>();

   //Test when `Type` is not the first encapsulated type
   test.template operator()<float, Type>();
  }

  TEST(createVariantStorage, copy_assignable_element_is_default_constructed_and_copy_assigned) {
   using Type = CopyAssignableType;

   constexpr auto test = createVariantStorageTestProducer<Type, Type const&>(
    [](auto& storage) {
     CX_GTEST_SHIM(
      EXPECT_TRUE,
      (storage.value.defaultConstructed && storage.value.copyAssigned)
     );
    }
   );

   //Test when `Type` is the first encapsulated type
   test.template operator()<Type, char, double>();

   //Test when `Type` is not the first encapsulated type
   test.template operator()<char, Type, double>();
  }

  TEST(createVariantStorage, move_constructible_element_is_move_constructed) {
   using Type = MoveConstructibleType;

   constexpr auto test = createVariantStorageTestProducer<Type, Type&&>(
    [](auto& storage) {
     CX_GTEST_SHIM(EXPECT_TRUE, storage.value.moveConstructed);
    }
   );

   //Test when `Type` is the first encapsulated type
   test.template operator()<Type, wchar_t>();

   //Test when `Type` is not the first encapsulated type
   test.template operator()<wchar_t, Type>();
  }

  TEST(createVariantStorage, move_assignable_element_is_default_constructed_and_move_assigned) {
   using Type = MoveAssignableType;

   constexpr auto test = createVariantStorageTestProducer<Type, Type&&>(
    [](auto& storage) constexpr {
     CX_GTEST_SHIM(
      EXPECT_TRUE,
      (storage.value.defaultConstructed && storage.value.moveAssigned)
     );
    }
   );

   //Test when `Type` is the first encapsulated type
   test.template operator()<Type, long long>();

   //Test when `Type` is not the first encapsulated type
   test.template operator()<long long, Type>();
  }

  TEST(createVariantStorage, array_of_copy_constructible_elements_are_copy_constructed) {
   using Type = CopyConstructibleType[5];

   constexpr auto const test = createVariantStorageTestProducer<
    Type,
    Type const&
   >([](auto& storage) {
    for (auto& element : storage.value) {
     CX_GTEST_SHIM(EXPECT_TRUE, element.copyConstructed);
    }
   });

   //Test when `Type` is the first encapsulated type
   test.template operator()<Type, float, long long>();
   //Test when `Type` is not the first encapsulated type
   test.template operator()<long long, float, Type>();
  }

  TEST(createVariantStorage, array_of_copy_assignable_elements_are_default_constructed_and_copy_assigned) {
   using Type = CopyAssignableType[1];

   constexpr auto const test = createVariantStorageTestProducer<
    Type,
    Type const&
   >([](auto& storage) {
    for (auto& element : storage.value) {
     CX_GTEST_SHIM(
      EXPECT_TRUE,
      (element.defaultConstructed && element.copyAssigned)
     );
    }
   });

   //Test when `Type` is the first encapsulated type
   test.template operator()<Type, wchar_t, float, int>();
   //Test when `Type` is not the first encapsulated type
   test.template operator()<wchar_t, Type, float, int>();
  }

  TEST(createVariantStorage, array_of_move_constructible_elements_are_move_constructed) {
   using Type = MoveConstructibleType[25];

   constexpr auto const test = createVariantStorageTestProducer<Type, Type&&>(
    [](auto& storage) {
     for (auto& element : storage.value) {
      CX_GTEST_SHIM(EXPECT_TRUE, element.moveConstructed);
     }
    }
   );

   //Test when `Type` is the first encapsulated type
   test.template operator()<Type, short[1], char>();
   //Test when `Type` is not the first encapsulated type
   test.template operator()<short[1], Type, char>();
  }

  TEST(createVariantStorage, array_of_move_assignable_elements_are_default_constructed_and_move_assigned) {
   using Type = MoveAssignableType[200];

   constexpr auto const test = createVariantStorageTestProducer<Type, Type&&>(
    [](auto& storage) {
     for (auto& element : storage.value) {
      CX_GTEST_SHIM(
       EXPECT_TRUE,
       (element.defaultConstructed && element.moveAssigned)
      );
     }
    }
   );

   //Test when `Type` is the first encapsulated type
   test.template operator()<Type, double[123]>();
   //Test when `Type` is not the first encapsulated type
   test.template operator()<double[123], Type>();
  }
 }

 //IsVariant concept tests
 TEST(IsVariant, variant_satisfies_constraint) {
  EXPECT_TRUE((IsVariant<Variant<>>));
  EXPECT_TRUE((IsVariant<Variant<int, char, float>>));
 }

 TEST(IsVariant, non_variant_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((IsVariant<char>));
  EXPECT_FALSE((IsVariant<Dummy<>>));
 }

 //CompatibleVariant concept tests
 TEST(CompatibleVariant, superset_and_matched_set_variant_types_satisfy_constraint) {
  //Matched type set
  EXPECT_TRUE((CompatibleVariant<Variant<>, Variant<>>));
  EXPECT_TRUE((CompatibleVariant<Variant<char>, Variant<char>>));

  //Superset
  EXPECT_TRUE((CompatibleVariant<Variant<int, float>, Variant<int, float, char>>));
  EXPECT_TRUE((CompatibleVariant<Variant<double, char, float>, Variant<float, double, char, int[1234]>>));
 }

 TEST(CompatibleVariant, subset_variant_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((CompatibleVariant<Variant<float[123]>, Variant<>>));
  EXPECT_FALSE((CompatibleVariant<Variant<double, char>, Variant<int, double>>));
  EXPECT_FALSE((CompatibleVariant<Variant<int, float, short>, Variant<int, short>>));
 }

 TEST(CompatibleVariant, non_variant_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((CompatibleVariant<Dummy<>, ImpossibleType<>>));
  EXPECT_FALSE((CompatibleVariant<Variant<>, Dummy<>>));
  EXPECT_FALSE((CompatibleVariant<MetaFunctions::MatchAnyType<>, Variant<int>>));
 }

 //Tests for `Variant<...>`

 //Generates runtime and constant-evaluated codepaths for a given test body
 constexpr auto variantTestProducer(auto testBody) noexcept {
  return [=]() constexpr noexcept {
   #ifdef CX_CONSTEXPR_SEMANTICS
    //Enable constant-evaluted test paths
    constexpr auto r = [=]() constexpr noexcept {
     testBody();
     return 0;
    }();
    (void)r;
   #endif

   //Runtime test path
   testBody();
  };
 }

 TEST(Constructor, element_copy_constructor_with_copy_constructible_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = CopyConstructibleType;
   Type toCopy;
   //Invoke variant element copy-constructor
   Variant<Type> v{copy(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure element was copy-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, v.get<Type>().copyConstructed);
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, element_copy_constructor_with_copy_assignable_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = CopyAssignableType;
   Type toCopy;
   //Invoke variant element copy-constructor
   Variant<Type> v{copy(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure element was default-constructed and copy-assigned
   auto& elem = v.get<Type>();
   CX_GTEST_SHIM(EXPECT_TRUE, elem.defaultConstructed && elem.copyAssigned);
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, element_move_constructor_with_move_constructible_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = MoveConstructibleType;
   Type toCopy;
   //Invoke variant element copy-constructor
   Variant<Type> v{move(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure element was move-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, v.get<Type>().moveConstructed);
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, element_move_constructor_with_move_assignable_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = MoveAssignableType;
   Type toCopy;
   //Invoke variant element copy-constructor
   Variant<Type> v{move(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure element was default-constructed and move-assigned
   auto& elem = v.get<Type>();
   CX_GTEST_SHIM(EXPECT_TRUE, elem.defaultConstructed && elem.moveAssigned);
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, element_copy_constructor_with_trivial_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   float toCopy = 3.1415f;
   //Invoke variant element copy-constructor
   Variant<float> v{copy(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<float>());
   //Ensure encapsulated element is the same value
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<float>() == toCopy));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, element_move_constructor_with_trivial_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   float toMove = 3.1415f;
   //Invoke variant element move-constructor
   Variant<float> v{move(toMove)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<float>());
   //Ensure encapsulated element is the same value
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<float>() == toMove));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, variant_copy_constructor_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   //Test with copy-constructible type
   using TypeA = CopyConstructibleType;
   TypeA toCopyConstruct;
   //Invoke variant element copy-constructor
   Variant<TypeA> v1{copy(toCopyConstruct)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v1.has<TypeA>());
   //Ensure element was copy-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v1.get<TypeA>().copyConstructed));

   //Invoke variant copy-constructor
   Variant<TypeA> v1Copy{copy(v1)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v1Copy.has<TypeA>());
   //Ensure element was copy-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v1Copy.get<TypeA>().copyConstructed));

   //Test with copy-assignable type
   using TypeB = CopyAssignableType;
   TypeB toCopyAssign;
   //Invoke variant element copy-constructor
   Variant<TypeB> v2{copy(toCopyAssign)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v2.has<TypeB>());
   //Ensure element was default-constructed and copy-assigned
   auto& e1 = v2.get<TypeB>();
   CX_GTEST_SHIM(EXPECT_TRUE, (e1.defaultConstructed && e1.copyAssigned));

   //Invoke variant copy-constructor
   Variant<TypeB> v2Copy{copy(v2)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v2Copy.has<TypeB>());
   //Ensure element was default-constructed and copy-assigned
   auto& e2 = v2Copy.get<TypeB>();
   CX_GTEST_SHIM(EXPECT_TRUE, (e2.defaultConstructed && e2.copyAssigned));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, variant_move_constructor_properly_initializes_variant_and_destructs_moved_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   //Test with move-constructible type
   using TypeA = MoveConstructibleType;
   TypeA toMoveConstruct;
   //Invoke variant element move-constructor
   Variant<TypeA> v1{move(toMoveConstruct)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v1.has<TypeA>());
   //Ensure element was move-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v1.get<TypeA>().moveConstructed));

   //Invoke variant move-constructor
   Variant<TypeA> v1Moved{move(v1)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v1Moved.has<TypeA>());
   //Ensure element was move-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v1Moved.get<TypeA>().moveConstructed));
   //Ensure v1 does not contain a value anymore
   CX_GTEST_SHIM(EXPECT_FALSE, v1.has<TypeA>());

   //Test with move-assignable type
   using TypeB = MoveAssignableType;
   TypeB toMoveAssign;
   //Invoke variant element copy-constructor
   Variant<TypeB> v2{move(toMoveAssign)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v2.has<TypeB>());
   //Ensure element was default-constructed and move-assigned
   auto& e1 = v2.get<TypeB>();
   CX_GTEST_SHIM(EXPECT_TRUE, (e1.defaultConstructed && e1.moveAssigned));

   //Invoke variant move-constructor
   Variant<TypeB> v2Moved{move(v2)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v2Moved.has<TypeB>());
   //Ensure element was default-constructed and move-assigned
   auto& e2 = v2Moved.get<TypeB>();
   CX_GTEST_SHIM(EXPECT_TRUE, (e2.defaultConstructed && e2.moveAssigned));
   //Ensure v1 does not contain a value anymore
   CX_GTEST_SHIM(EXPECT_FALSE, v2.has<TypeB>());
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, compatible_variant_copy_constructor_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   //Variant<float, char, int> = copy(Variant<int, float>)
   int toCopy = 1234;
   Variant<int, float> cv{copy(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, cv.has<int>());
   CX_GTEST_SHIM(EXPECT_TRUE, (cv.get<int>() == toCopy));

   //Invoke CompatibleVariant copy-constructor
   Variant<float, char, int> v{copy(cv)};
   //Ensure variant was properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<int>());
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<int>() == toCopy));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, compatible_variant_move_constructor_properly_initializes_variant_and_destructs_moved_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   //Variant<float, char, int> = copy(Variant<int, float>)
   int toCopy = 1234;
   Variant<int, float> cv{copy(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, cv.has<int>());
   CX_GTEST_SHIM(EXPECT_TRUE, (cv.get<int>() == toCopy));

   //Invoke CompatibleVariant copy-constructor
   Variant<float, char, int> v{copy(cv)};
   //Ensure variant was properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<int>());
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<int>() == toCopy));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, array_element_copy_constructor_with_copy_constructible_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = CopyConstructibleType[123];
   Type toCopy{};
   //Invoke variant element copy-constructor
   Variant<Type> v{copy(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were copy-constructed
   for (auto& e : v.get<Type>()) {
    CX_GTEST_SHIM(EXPECT_TRUE, e.copyConstructed);
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, array_element_copy_constructor_with_copy_assignabe_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = CopyAssignableType[123];
   Type toCopy{};
   //Invoke variant element copy-constructor
   Variant<Type> v{copy(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were copy-assigned
   for (auto& e : v.get<Type>()) {
    CX_GTEST_SHIM(EXPECT_TRUE, e.copyAssigned);
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, array_element_move_constructor_with_move_constructible_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = MoveConstructibleType[200];
   Type toMove{};
   //Invoke variant element move-constructor
   Variant<Type> v{move(toMove)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were move-constructed
   for (auto& e : v.get<Type>()) {
    CX_GTEST_SHIM(EXPECT_TRUE, e.moveConstructed);
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, array_element_move_constructor_with_move_assignable_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = MoveAssignableType[2];
   Type toMove{};
   //Invoke variant element move-constructor
   Variant<Type> v{move(toMove)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were move-assigned
   for (auto& e : v.get<Type>()) {
    CX_GTEST_SHIM(EXPECT_TRUE, e.moveAssigned);
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, array_element_of_trivial_type_copy_constructor_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = wchar_t[243];
   Type toCopy{};
   //Invoke variant element move-constructor
   Variant<Type> v{copy(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure encapsulated elements are the same value
   auto& e = v.get<Type>();
   for (SizeType i = 0; i < ArraySize<Type>; i++) {
    CX_GTEST_SHIM(EXPECT_TRUE, (e[i] == toCopy[i]));
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Constructor, array_element_of_trivial_type_move_constructor_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = double[32];
   Type toMove{};
   //Invoke variant element move-constructor
   Variant<Type> v{move(toMove)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure encapsulated elements are the same value
   auto& e = v.get<Type>();
   for (SizeType i = 0; i < ArraySize<Type>; i++) {
    CX_GTEST_SHIM(EXPECT_TRUE, (e[i] == toMove[i]));
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, element_copy_assignment_with_copy_constructible_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = CopyConstructibleType;
   Type toCopy{};
   //Invoke variant element copy-assignment operator
   Variant<Type> v;
   v = copy(toCopy);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were copy-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<Type>().copyConstructed));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, element_copy_assignment_with_copy_assignable_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = CopyAssignableType;
   Type toCopy{};
   //Invoke variant element copy-assignment operator
   Variant<Type> v;
   v = copy(toCopy);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were copy-assigned
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<Type>().copyAssigned));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, element_move_assignment_with_move_constructible_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = MoveConstructibleType;
   Type toMove{};
   //Invoke variant element move-assignment operator
   Variant<Type> v;
   v = move(toMove);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were move-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<Type>().moveConstructed));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, element_move_assignment_with_move_assignable_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = MoveConstructibleType;
   Type toMove{};
   //Invoke variant element move-assignment operator
   Variant<Type> v;
   v = move(toMove);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were move-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<Type>().moveConstructed));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, element_copy_assignment_with_trivial_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = char8_t;
   Type toMove{};
   //Invoke variant element move-assignment operator
   Variant<Type> v;
   v = move(toMove);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure encapsulated element is the same value
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<Type>() == toMove));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, element_move_assignment_with_trivial_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = long;
   Type toMove{};
   //Invoke variant element move-assignment operator
   Variant<Type> v;
   v = move(toMove);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure encapsulated element is the same value
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<Type>() == toMove));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, variant_copy_assignment_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   //Test with copy-constructible type
   using TypeA = CopyConstructibleType;
   TypeA toCopyConstruct;
   //Invoke variant element copy-constructor
   Variant<TypeA> v1{copy(toCopyConstruct)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v1.has<TypeA>());
   //Ensure element was copy-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v1.get<TypeA>().copyConstructed));

   //Invoke variant copy-assignment operator
   Variant<TypeA> v1Copy;
   v1Copy = copy(v1);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v1Copy.has<TypeA>());
   //Ensure element was copy-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v1Copy.get<TypeA>().copyConstructed));

   //Test with copy-assignable type
   using TypeB = CopyAssignableType;
   TypeB toCopyAssign;
   //Invoke variant element copy-constructor
   Variant<TypeB> v2{copy(toCopyAssign)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v2.has<TypeB>());
   //Ensure element was default-constructed and copy-assigned
   auto& e1 = v2.get<TypeB>();
   CX_GTEST_SHIM(EXPECT_TRUE, (e1.defaultConstructed && e1.copyAssigned));

   //Invoke variant copy-assignment operator
   Variant<TypeB> v2Copy;
   v2Copy = copy(v2);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v2Copy.has<TypeB>());
   //Ensure element was default-constructed and copy-assigned
   auto& e2 = v2Copy.get<TypeB>();
   CX_GTEST_SHIM(EXPECT_TRUE, (e2.defaultConstructed && e2.copyAssigned));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, variant_move_assignment_properly_initializes_variant_and_destructs_moved_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   //Test with copy-constructible type
   using TypeA = MoveConstructibleType;
   TypeA toMoveConstruct;
   //Invoke variant element move-constructor
   Variant<TypeA> v1{move(toMoveConstruct)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v1.has<TypeA>());
   //Ensure element was move-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v1.get<TypeA>().moveConstructed));

   //Invoke variant move-assignment operator
   Variant<TypeA> v1Moved;
   v1Moved = move(v1);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v1Moved.has<TypeA>());
   //Ensure element was copy-constructed
   CX_GTEST_SHIM(EXPECT_TRUE, (v1Moved.get<TypeA>().moveConstructed));

   //Test with copy-assignable type
   using TypeB = MoveAssignableType;
   TypeB toMoveAssign;
   //Invoke variant element move-constructor
   Variant<TypeB> v2{move(toMoveAssign)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v2.has<TypeB>());
   //Ensure element was default-constructed and move-assigned
   auto& e1 = v2.get<TypeB>();
   CX_GTEST_SHIM(EXPECT_TRUE, (e1.defaultConstructed && e1.moveAssigned));

   //Invoke variant move-assignment operator
   Variant<TypeB> v2Moved;
   v2Moved = move(v2);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v2Moved.has<TypeB>());
   //Ensure element was default-constructed and move-assigned
   auto& e2 = v2Moved.get<TypeB>();
   CX_GTEST_SHIM(EXPECT_TRUE, (e2.defaultConstructed && e2.moveAssigned));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, compatible_variant_copy_assignment_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   //Variant<int[32], float, short, char> = copy(Variant<int[32]>)
   int toCopy[32]{};
   Variant<int[32]> cv{copy(toCopy)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, cv.has<int[32]>());
   auto& e1 = cv.get<int[32]>();
   for (SizeType i = 0; i < 32; i++) {
    CX_GTEST_SHIM(EXPECT_TRUE, (e1[i] == toCopy[i]));
   }

   //Invoke CompatibleVariant copy-constructor
   Variant<int[32], float, short, char> v{copy(cv)};
   //Ensure variant was properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<int[32]>());
   auto& e2 = v.get<int[32]>();
   for (SizeType i = 0; i < 32; i++) {
    CX_GTEST_SHIM(EXPECT_TRUE, (e2[i] == toCopy[i]));
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, compatible_variant_move_assignment_properly_initializes_variant_and_destructs_moved_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   //Variant<Type, MoveAssignableType> = copy(Variant<Type, Type[123], float>)
   using Type = MoveConstructibleType;
   Type toMove;
   Variant<Type> cv{move(toMove)};
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, cv.has<Type>());
   CX_GTEST_SHIM(EXPECT_TRUE, (cv.get<Type>().moveConstructed));

   //Invoke CompatibleVariant copy-constructor
   Variant<Type, Type[123], MoveAssignableType> v{move(cv)};
   //Ensure variant was properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<Type>().moveConstructed));

   //Ensure `cv` was reset
   CX_GTEST_SHIM(EXPECT_FALSE, (cv.has<Type>()));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, array_element_copy_assignment_with_copy_constructible_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = CopyConstructibleType[23];
   Type toCopy{};
   //Invoke variant element copy-assignment operator
   Variant<Type> v;
   v = copy(toCopy);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were copy-constructed
   for (auto& e : v.get<Type>()) {
    CX_GTEST_SHIM(EXPECT_TRUE, e.copyConstructed);
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, array_element_copy_assignment_with_copy_assignabe_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = CopyAssignableType[23];
   Type toCopy{};
   //Invoke variant element copy-assignment operator
   Variant<Type> v;
   v = copy(toCopy);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were default-constructed and copy-assigned
   for (auto& e : v.get<Type>()) {
    CX_GTEST_SHIM(EXPECT_TRUE, (e.defaultConstructed && e.copyAssigned));
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, array_element_move_assignment_with_move_constructible_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = MoveConstructibleType[22];
   Type toCopy{};
   //Invoke variant element copy-assignment operator
   Variant<Type> v;
   v = move(toCopy);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were move-constructed
   for (auto& e : v.get<Type>()) {
    CX_GTEST_SHIM(EXPECT_TRUE, e.moveConstructed);
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, array_element_move_assignment_with_move_assignable_type_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = MoveAssignableType[33];
   Type toCopy{};
   //Invoke variant element copy-assignment operator
   Variant<Type> v;
   v = move(toCopy);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure elements were default-constructed and move-assigned
   for (auto& e : v.get<Type>()) {
    CX_GTEST_SHIM(EXPECT_TRUE, (e.defaultConstructed && e.moveAssigned));
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, array_element_of_trivial_type_copy_assignment_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = char16_t[19];
   Type toCopy{};
   //Invoke variant element move-assignment operator
   Variant<Type> v;
   v = copy(toCopy);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure encapsulated elements are the same value
   auto& e = v.get<Type>();
   for (SizeType i = 0; i < ArraySize<Type>; i++) {
    CX_GTEST_SHIM(EXPECT_TRUE, (e[i] == toCopy[i]));
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Assignment, array_element_of_trivial_type_move_assignment_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = short[2];
   Type toMove{};
   //Invoke variant element move-assignment operator
   Variant<Type> v;
   v = move(toMove);
   //Ensure variant is properly initialized
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   //Ensure encapsulated elements are the same value
   auto& e = v.get<Type>();
   for (SizeType i = 0; i < ArraySize<Type>; i++) {
    CX_GTEST_SHIM(EXPECT_TRUE, (e[i] == toMove[i]));
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Variant, has_by_element_type_yields_expected_value) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   Variant<float, char, int> v{(int)31415};
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<int>());
   CX_GTEST_SHIM(EXPECT_FALSE, v.has<float>());
   CX_GTEST_SHIM(EXPECT_FALSE, v.has<char>());
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Variant, has_by_element_type_index_yields_expected_value) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   Variant<char8_t, char16_t, char32_t, wchar_t> v{(char8_t)' '};
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<0>());
   CX_GTEST_SHIM(EXPECT_FALSE, v.has<1>());
   CX_GTEST_SHIM(EXPECT_FALSE, v.has<2>());
   CX_GTEST_SHIM(EXPECT_FALSE, v.has<3>());
  });
  EXPECT_NO_EXIT((body()));
 }

 /*
 TEST(Variant, set_copy_by_element_type_index_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   auto toCopy = (long double)3.333333333333333333333333;
   Variant<short, long double, int> v;
   v.set<1>(copy(toCopy));
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<long double>());
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<long double>() == toCopy));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Variant, set_move_by_element_type_index_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   int toMove = 1234;
   Variant<int> v;
   v.set<0>(move(toMove));
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<int>());
   CX_GTEST_SHIM(EXPECT_TRUE, (v.get<int>() == toMove));
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Variant, set_copy_by_array_element_type_index_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = CopyAssignableType[12];
   Type toCopy{};
   Variant<float, float, float, float, float, float, float, float, Type> v;
   v.set<8>(copy(toCopy));
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   for (auto& e : v.get<8>()) {
    CX_GTEST_SHIM(EXPECT_TRUE, e.copyAssigned);
   }
  });
  EXPECT_NO_EXIT((body()));
 }

 TEST(Variant, set_move_by_array_element_type_index_properly_initializes_variant) {
  constexpr auto const body = variantTestProducer([]() constexpr noexcept {
   using Type = MoveConstructibleType[2];
   Type toMove{};
   Variant<int, float, Type, char, double> v;
   v.set<2>(move(toMove));
   CX_GTEST_SHIM(EXPECT_TRUE, v.has<Type>());
   auto& e = v.get<Type>();
   for (SizeType i = 0; i < ArraySize<Type>; i++) {
    CX_GTEST_SHIM(EXPECT_TRUE, (e[i].moveConstructed));
   }
  });
  EXPECT_NO_EXIT((body()));
 }
 */

 //TODO Tests for checked decapsulation
 //TODO Tests for active element
 /*
 TEST(Variant, ) {

 }
 */

 //TODO Tests for `Variant<>`
}
