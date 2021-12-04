#pragma once

#include <cx/common.h>
#include <cx/idioms.h>
#include <cx/templates.h>

namespace CX {
 //Constant-evaluated placement new
 template<typename T, typename... Args>
 constexpr T& newInPlace(T& t, Args... args) noexcept {
  return *std::construct_at(&t, (Args)args...);
 }

 //Delegate function to invoke the destructor of a given type
 template<typename T>
 constexpr void destroy(T& t) noexcept {
  using Type = ConstVolatileDecayed<ReferenceDecayed<T>>;
  if constexpr (Destructible<T> && !TriviallyDestructible<T>) {
   t.~Type();
  }
 };

 //Actions performed by `copyOrMove`
 enum struct CopyOrMoveOperation {
  ASSIGN,
  CONSTRUCT,
  DEFAULT_CONSTRUCT_AND_ASSIGN,
  NONE
 };

 //Identity concept for types that can be copy-constructed, copy-assigned or
 //default constructed and copy-assigned
 template<typename T>
 concept IsCopyable = CopyConstructible<T>
  || CopyAssignable<T>
  || (Constructible<T> && CopyAssignable<T>);

 //Identity concept for types that can be move-constructed, move-assigned or
 //default constructed and move-assigned
 template<typename T>
 concept IsMovable = MoveConstructible<T>
  || MoveAssignable<T>
  || (Constructible<T> && MoveAssignable<T>);

 //Identity concept for types that can be handled by `copyOrMove`
 template<typename T>
 concept IsCopyableOrMovable = IsCopyable<T> || IsMovable<T>;

 namespace Internal {
  //Implementation of `copyOrMove`
  //Copy or move value, does the following:
  // 1.   If copy/move assignable AND value IS present, copy/move assign.
  //      Yields `CopyOrMoveOperation::ASSIGN`.
  // 2.   If copy/move constructible AND value is NOT present, copy/move
  //      construct in-place. Yields `CopyOrMoveOperation::CONSTRUCT`.
  // 2.1. If copy/move constructible AND value IS present, destroy and
  //      copy/move construct in-place. Yields `CopyOrMoveOperation::CONSTRUCT`
  // 3.   If default constructible, copy/move assignable, AND value is NOT
  //      present, construct in-place and copy/move assign. Yields
  //      `CopyOrMoveOperation::DEFAULT_CONSTRUCT_AND_ASSIGN`.
  // 3.1  If default constructible, copy/move assignable AND value IS present
  //      destroy, default construct in-place and copy/move assign. Yields
  //      `CopyOrMoveOperation::DEFAULT_CONSTRUCT_AND_ASSIGN`.
  // 4.   Otherwise, yield `CopyOrMoveOperation::NONE`
  template<IsCopyableOrMovable T, typename V>
  requires MatchAnyType<V, T const&, T&&>
  constexpr CopyOrMoveOperation copyOrMove(
   T& dst,
   V src,
   bool present,
   FunctionWithPrototype<void (T&) noexcept> auto& destroyer
  ) noexcept {
   constexpr auto const
    copy = SameType<V, T const&>,
    assignable = (copy ? CopyAssignable<T> : MoveAssignable<T>),
    constructible = (copy ? CopyConstructible<T> : MoveConstructible<T>),
    defaultConstructible = Constructible<T>;

   constexpr auto const viableType = assignable
    || constructible
    || (defaultConstructible && assignable);

   static_assert(
    viableType,
    R"e(copyOrMove requires types that are any of:
1. Copy or move assignable and present
2. Copy or move constructible (present or not)
3. Default constructible and copy or move assignable (present or not)

Note: If supplied with a type that is copy or move assignable but not present,
will yield `CopyOrMoveOperation::NONE`.
)e"
   );

   using PromotionType = SelectType<copy, T const&, T&&>;

   //1
   if constexpr (assignable) {
    if (present) {
     dst = (PromotionType)src;
     return CopyOrMoveOperation::ASSIGN;
    }
   }

   //2
   if constexpr (constructible) {
    if (present) {
     destroyer(dst);
    }
    newInPlace<T, PromotionType>(dst, (PromotionType)src);
    return CopyOrMoveOperation::CONSTRUCT;
   }

   //3
   if constexpr (defaultConstructible && assignable) {
    if (present) {
     destroyer(dst);
    }
    newInPlace<T>(dst) = (PromotionType)src;
    return CopyOrMoveOperation::DEFAULT_CONSTRUCT_AND_ASSIGN;
   }

   //Handle error case for types that are assignable but not constructible or
   //defaultConstructible, and absent
   return CopyOrMoveOperation::NONE;
  }
 }

 //Copy shim to `Internal::copyOrMove`
 template<IsCopyableOrMovable T>
 requires (!Const<T>)
 constexpr CopyOrMoveOperation copyOrMove(
  T& dst,
  T const& src,
  bool present,
  FunctionWithPrototype<void (T&) noexcept> auto&& destroyer
 ) noexcept {
  return Internal::copyOrMove<T, T const&>(
   dst,
   src,
   present,
   destroyer
  );
 }

 //Copy shim to `Internal::copyOrMove`
 template<IsCopyableOrMovable T>
 requires (!Const<T>)
 constexpr CopyOrMoveOperation copyOrMove(T& dst, T const& src, bool present)
  noexcept
 {
  return Internal::copyOrMove<T, T const&>(
   dst,
   src,
   present,
   destroy<T>
  );
 }

 //Move shim to `Internal::copyOrMove`
 template<IsCopyableOrMovable T>
 requires (!Const<T>)
 constexpr CopyOrMoveOperation copyOrMove(
  T& dst,
  T&& src,
  bool present,
  FunctionWithPrototype<void (T&) noexcept> auto&& destroyer
 ) noexcept {
  return Internal::copyOrMove<T, T&&>(
   dst,
   move(src),
   present,
   destroyer
  );
 }

 //Move shim to `Internal::copyOrMove`
 template<IsCopyableOrMovable T>
 requires (!Const<T>)
 constexpr CopyOrMoveOperation copyOrMove(T& dst, T&& src, bool present)
  noexcept
 {
  return Internal::copyOrMove<T, T&&>(
   dst,
   move(src),
   present,
   destroy<T>
  );
 }
}
