#pragma once

#include <cx/common.h>
#include <cx/idioms.h>
#include <cx/exit.h>

namespace CX {
 template<typename T>
 concept OptionParameter = !(SameType<T, void> || UnsizedArray<T>);

 //Forward declare `CX::Option` for use with option meta-functions
 template<OptionParameter T>
 struct Option;

 //Forward declare `CX::OptionTuple` for use with option meta-functions
 template<OptionParameter... Types>
 struct OptionTuple;

 namespace OptionMetaFunctions {
  template<typename MaybeOption, typename = Unqualified<MaybeOption>>
  struct IsOption : FalseType {};

  template<
   typename MaybeOption,
   template<typename...> typename O,
   typename... Types
  >
  struct IsOption<MaybeOption, O<Types...>> :
   MetaFunctions::MatchAnyTemplateType<O, Option, OptionTuple>
  {};
 }

 //Option identity concept
 template<typename MaybeOption>
 concept IsOption = OptionMetaFunctions
  ::IsOption<MaybeOption>
  ::Value;

 //`CX::Option` impl
 template<OptionParameter T>
 struct Option final {
  //Prevent users from embedding options inside options
  static_assert(
   !IsOption<T>,
   "Cascading options is an antipattern. Consider consolidating your optional "
   "values."
  );

  template<OptionParameter>
  friend struct Option;

 private:
  bool populated;
  union Storage {
   unsigned char _[0];
   T value;

   constexpr Storage() noexcept :
    _{}
   {}

   //Value copy constructor
   constexpr Storage(T const& t)
    noexcept(noexcept(T{(T const&)t}))
    requires (CopyConstructible<T>)
   :
    value{(T const&)t}
   {}

   //Value move constructor
   constexpr Storage(T&& t)
    noexcept(noexcept(T{(T&&)t}))
    requires (MoveConstructible<T>)
   :
    value{(T&&)t}
   {}

   //Default copy constructor
   constexpr Storage(Storage const&) noexcept = default;

   //Default move constructor
   constexpr Storage(Storage&&) noexcept = default;

   constexpr ~Storage() noexcept(noexcept(value.~T())) = default;
  } storage;

  template<typename F>
  static constexpr bool const SupportedGeneratorPrototype = false;

 public:
  constexpr Option() noexcept :
   populated{false},
   storage{}
  {}

  //Value copy constructor
  constexpr Option(T const& t) noexcept(noexcept(Storage{(T const&)t})) :
   populated{true},
   storage{(T const&)t}
  {}

  //Value move constructor
  constexpr Option(T&& t) noexcept(noexcept(Storage{(T&&)t})) :
   populated{true},
   storage{(T&&)t}
  {}

  //Copy constructor
  constexpr Option(Option const& other) noexcept(
   declval<Option&>().operator=(declval<Option const&>())
  ) :
   populated{false},
   storage{}
  {
   const_cast<Option&>(*this).operator=((Option const&)other);
  }

  //Move constructor
  constexpr Option(Option&& other) noexcept(
   declval<Option&>().operator=(declval<Option&&>())
  ) :
   populated{false},
   storage{}
  {
   const_cast<Option&>(*this).operator=((Option&&)other);
  }

  //Constexpr destructor
  constexpr ~Option() noexcept(noexcept(reset())) {
   reset();
  }

  //Option copy-assignment operator
  constexpr Option& operator=(Option const& other) noexcept(
   (TriviallyCopyable<T> && noexcept(storage = other.storage))
   || (!TriviallyCopyable<T>
       && noexcept(Storage{} = Storage{(T const&)other.storage.value})
      )
  ) {
   if (other.populated) {
    if constexpr (TriviallyCopyable<T>) {
     storage = other.storage;
    } else {
     if (populated) {
      storage.value = (T const&)other.storage.value;
     } else {
      storage = {(T const&)other.storage.value};
     }
    }
    populated = true;
   } else {
    reset();
   }
  }

  //Option move-assignment operator
  //Note: Constexpr moves are not permitted since they require modification
  //of a remote object
  Option& operator=(Option&& other) noexcept(
   (TriviallyCopyable<T> && noexcept(storage = other.storage))
   || (!TriviallyCopyable<T>
       && noexcept(Storage{} = Storage{(T const&)other.storage.value})
      )
  ) {
   reset();
   populated = other.populated;
   if constexpr (TriviallyCopyable<T>) {
    storage = other.storage;
   } else {
    storage = {(T const&)other.storage.value};
   }
   other.reset();
   return *this;
  }

  //Value copy-assignment operator
  constexpr Option& operator=(T const& other)
   noexcept(
    (CopyAssignable<T>
     && noexcept(storage.value = (T const&)other)
     && noexcept(Storage{} = Storage{(T const&)other})
    )
    || (!CopyAssignable<T> && noexcept(Storage{} = Storage{(T const&)other}))
   )
   requires (CopyConstructible<T>)
  {
   if constexpr (CopyAssignable<T>) {
    if (populated) {
     storage.value = (T const&)other;
    } else {
     storage = {(T const&)other};
    }
   } else {
    reset();
    storage = {(T const&)other};
   }
   return *this;
  }

  //Value move-assignment operator
  constexpr Option& operator=(T&& other)
   noexcept(
    (MoveAssignable<T>
     && noexcept(storage.value = (T&&)other)
     && noexcept(Storage{} = Storage{(T&&)other})
    )
    || (!MoveAssignable<T> && noexcept(Storage{} = Storage{(T&&)other}))
   )
   requires (MoveConstructible<T>)
  {
   if constexpr (MoveAssignable<T>) {
    if (populated) {
     storage.value = (T&&)other;
    } else {
     storage = {(T&&)other};
    }
   } else {
    reset();
    storage = {(T&&)other};
   }
   return *this;
  }

  //Destructs storage value, if populated, and resets option
  constexpr void reset() noexcept(noexcept(storage.value.~T())) {
   if constexpr (!TriviallyDestructible<T>) {
    if (populated) {
     storage.value.~T();
    }
   }
   populated = false;
   storage = {};
  }

  //Value presence implicit conversion
  constexpr operator bool() const noexcept {
   return populated;
  }

  //"Value or immediate" operator
  constexpr auto& operator|(T other) const noexcept {
   return populated ? storage.value : other;
  }

  //"Value or lazyily evaluated result" operator
  constexpr auto operator|(FunctionOperator<T ()> auto generator) const
   noexcept/*(generator())*/
  {
   return populated ? storage.value : generator();
  }

  //Checked value decapsulation
  auto& operator+() const noexcept {
   if (populated) {
    return storage.value();
   }
   //TODO appropriate error
   exit();
  }

  //Unchecked value decapsulation
  constexpr auto& operator!() const noexcept {
   return storage.value;
  }
 };

 //`OptionTuple` impl
 template<OptionParameter... Types>
 struct OptionTuple final {
  //TODO
  static_assert(
   sizeof...(Types) != sizeof...(Types),
   "Unimplemented"
  );
 };
}
