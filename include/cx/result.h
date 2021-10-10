#pragma once

#include <cx/common.h>
#include <cx/idioms.h>
#include <cx/error.h>
#include <cx/exit.h>

namespace CX {
 struct ResultValueNotPresentError final {
  constexpr auto& describe() const noexcept {
   return "Result value not present!";
  }
 };
 static_assert(IsError<ResultValueNotPresentError>);

 struct ResultErrorNotPresentError final {
  constexpr auto& describe() const noexcept {
   return "Result error not present!";
  }
 };
 static_assert(IsError<ResultErrorNotPresentError>);

 struct ResultMovedError final {
  constexpr auto& describe() const noexcept {
   return "Result moved!";
  }
 };
 static_assert(IsError<ResultMovedError>);

 //Result error type identity concept
 //Note: Supports `CX::Never` for guaranteed value results to not waste space
 template<typename E>
 concept ResultErrorParameter = IsError<E> || SameType<E, Never>;

 //Forward declare `CX::Result` for use with result meta-functions
 template<typename R, ResultErrorParameter E = Never>
 struct Result;

 namespace ResultMetaFunctions {
  //Result identity meta-function
  template<typename MaybeResult, typename = Unqualified<MaybeResult>>
  struct IsResult : FalseType {};

  template<
   typename MaybeResult,
   template<typename...> typename R,
   typename RType,
   typename EType
  >
  struct IsResult<MaybeResult, R<RType, EType>> :
   MetaFunctions::MatchAnyTemplateType<R, Result>
  {
   using ResultType = RType;
   using ErrorType  = EType;
  };

  //Result state enum
  enum struct ResultState : unsigned char {
   OK,
   ERR,
   MOVED
  };
 }

 //Result identity concept
 template<typename MaybeResult>
 concept IsResult = ResultMetaFunctions
  ::IsResult<MaybeResult>
  ::Value;

 //Result value type meta-function
 template<IsResult R>
 using ResultType = typename ResultMetaFunctions
  ::IsResult<R>
  ::ResultType;

 //Result error type meta-function
 template<IsResult R>
 using ResultError = typename ResultMetaFunctions
  ::IsResult<R>
  ::ErrorType;

 //`CX::Result` impl
 //TODO
 // - Result error disjunction operator. ie.
 //   Result<int, E1> r1;
 //   Result<int, E2> r2;
 //   auto r3 = r1 | r2; //-> Result<int, Variant<E1, E2>> OR Result<int, E1, E2>
 // - Result error conjunction operator. ie.
 //   Result<int, E1> r1;
 //   Result<int, E2> r2;
 //   auto r3 = r1 & r2; //-> Result<int, Error>
 // - Replace internal `Storage` union with `CX::Variant` once it has been made
 //   constexpr-compatible
 template<typename R, ResultErrorParameter E>
 struct Result final {
 private:
  using ResultState = ResultMetaFunctions::ResultState;

  ResultState state : 2;
  union Storage {
   //Intermediate state
   Never _;
   //Value state
   R value;
   //Error state
   E error;

   //Intermediate state constructor
   constexpr Storage() noexcept :
    _{}
   {}

   //Value copy constructor
   constexpr Storage(R const& value) noexcept
    requires (CopyConstructible<R>)
   :
    value{(R const&)value}
   {}

   //Value move constructor
   constexpr Storage(R&& value) noexcept
    requires (MoveConstructible<R>)
   :
    value{(R&&)value}
   {}

   //Error copy constructor
   constexpr Storage(E const& error) noexcept
    requires (CopyConstructible<E>)
   :
    error{(E const&)error}
   {}

   //Error move constructor
   constexpr Storage(E&& error) noexcept
    requires (MoveConstructible<E>)
   :
    error{(E&&)error}
   {}

   //Default constexpr union copy constructor
   constexpr Storage(Storage const&) noexcept = default;

   //Default constexpr union destructor
   constexpr ~Storage() noexcept {}

   //Default union copy-assignment operator
   constexpr Storage& operator=(Storage const&) = default;
  } storage;

  //Resets state to `ResultState::MOVED`
  constexpr void reset() noexcept {
   switch (state) {
    case ResultState::OK: {
     //Destruct encapsulated value
     if constexpr (Destructible<R> && !TriviallyDestructible<R>) {
      storage.value.~R();
     }
     break;
    }
    case ResultState::ERR: {
     //Destruct encapsulated error
     if constexpr (Destructible<E> && !TriviallyDestructible<E>) {
      storage.error.~E();
     }
     break;
    }
    default: break;
   }

   //Re-initialize `storage` and `state`
   storage._ = {};
   state = ResultState::MOVED;
  }

  //Returns mutable reference
  constexpr Result& mut() const noexcept {
   return const_cast<Result&>(*this);
  }

  //Whether or not the result has been moved
  constexpr bool hasMoved() const noexcept {
   return state == ResultState::MOVED;
  }

 public:
  //Value copy constructor
  constexpr Result(R const& value) noexcept :
   state{ResultState::OK},
   storage{(R const&)value}
  {}

  //Value move constructor
  constexpr Result(R&& value) noexcept :
   state{ResultState::OK},
   storage{(R&&)value}
  {}

  //Error copy constructor
  constexpr Result(E const& error) noexcept :
   state{ResultState::ERR},
   storage{(E const&)error}
  {}

  //Error move constructor
  constexpr Result(E&& error) noexcept :
   state{ResultState::ERR},
   storage{(E&&)error}
  {}

  //Result copy constructor
  constexpr Result(Result const& other) noexcept :
   state{ResultState::ERR},
   storage{}
  {
   mut().operator=((Result const&)other);
  }

  //Result move constructor
  constexpr Result(Result&& other) noexcept :
   state{ResultState::ERR},
   storage{}
  {
   mut().operator=((Result&&)other);
  }

  //Constexpr destructor
  constexpr ~Result() noexcept {
   mut().reset();
  }

  //Value copy assignment operator
  constexpr Result& operator=(R const &value) noexcept {
   //Initialize storage
   if (state == ResultState::OK && CopyAssignable<R>) {
    //Copy-assign value
    storage.value = (R const&)value;
   } else {
    //Clean up current state and copy-construct value
    reset();
    storage = {(R const&)value};
   }
   state = ResultState::OK;

   return *this;
  }

  //Value move assignment operator
  constexpr Result& operator=(R&& value) noexcept {
   //Initialize storage
   if (state == ResultState::OK && MoveAssignable<R>) {
    //Move-assign value
    storage.value = (R&&)value;
   } else {
    //Clean up current state and move-construct value
    reset();
    storage = {(R&&)value};
   }
   state = ResultState::OK;

   return *this;
  }

  //Error copy assignment operators
  constexpr Result& operator=(E const &error) noexcept {
   //Initialize storage
   if (state == ResultState::ERR && CopyAssignable<E>) {
    //Copy-assign error
    storage.error = (E const&)error;
   } else {
    //Clean up current state and copy-construct error
    reset();
    storage.error = {(E const&)error};
   }
   state = ResultState::ERR;

   return *this;
  }

  //Error move assignment operator
  constexpr Result& operator=(E&& error) noexcept {
   //Initialize storage
   if (state == ResultState::ERR && MoveAssignable<E>) {
    //Move-assign error
    storage.error = (E&&)error;
   } else {
    //Clean up current state and move-construct error
    reset();
    storage = {(E&&)error};
   }
   state = ResultState::ERR;

   return *this;
  }

  //Result copy-assignment operator
  constexpr Result& operator=(Result const& other) noexcept {
   auto const initStorage = [&]<typename T>(T const& t) constexpr noexcept {
    if constexpr (TriviallyCopyable<T>) {
     storage = other.storage;
    } else {
     storage = this->operator=((T const&)t);
    }
   };

   //Clean up current state
   reset();

   //Initialize storage
   switch (other.state) {
    case ResultState::OK: {
     //Copy value
     initStorage(other.storage.value);
     break;
    }
    case ResultState::ERR: {
     //Copy error
     initStorage(other.storage.error);
     break;
    }
    case ResultState::MOVED: {
     storage = {};
     break;
    }
   }
   state = other.state;
   return *this;
  }

  //Result move-assignment operator
  constexpr Result& operator=(Result&& other) noexcept {
   auto const initStorage = [&]<typename T>(T&& t) constexpr noexcept {
    if constexpr (TriviallyCopyable<T>) {
     storage = other.storage;
    } else {
     storage = this->operator=((T&&)t);
    }
   };

   //Clean up current state
   reset();

   //Initialize storage and state
   switch (other.state) {
    case ResultState::OK: {
     //Copy value
     initStorage(other.storage.value);
     break;
    }
    case ResultState::ERR: {
     //Copy error
     initStorage(other.storage.error);
     break;
    }
    case ResultState::MOVED: {
     storage = {};
     break;
    }
   }
   state = other.state;

   //Reset moved result
   other.reset();
   return *this;
  }

  //Value presence check
  constexpr bool hasValue() const noexcept {
   return state == ResultState::OK;
  }

  //Error presence check
  constexpr bool hasError() const noexcept {
   return state == ResultState::ERR;
  }

  //Value presence implicit conversion
  constexpr operator bool() const noexcept {
   return hasValue();
  }

  //"Value or immediate" operator
  constexpr auto& operator||(R other) const noexcept {
   return hasValue() ? storage.value : other;
  }

  //"Value or lazily evaluated result" operator
  constexpr auto operator||(FunctionOperator<R ()> auto generator) const
   noexcept
  {
   return hasValue() ? storage.value : generator();
  }

  //Checked value decapsulation
  constexpr auto& operator+() const noexcept {
   if (hasValue()) {
    return storage.value;
   }
   if (hasMoved()) {
    exit(ResultMovedError{});
   } else {
    exit(ResultValueNotPresentError{});
   }
  }

  //Checked value decapsulation
  constexpr auto& getValue() const noexcept {
   return +*this;
  }

  //Checked error decapsulation
  constexpr auto& operator-() const noexcept {
   if (hasError()) {
    return storage.error;
   }
   if (hasMoved()) {
    exit(ResultMovedError{});
   } else {
    exit(ResultErrorNotPresentError{});
   }
  }

  //Checked error decapsulation
  constexpr auto& getError() const noexcept {
   return -*this;
  }

  //Unchecked value decapsulation
  constexpr auto& operator!() const noexcept {
   return storage.value;
  }

  //Unchecked value decapsulation
  constexpr auto& value() const noexcept {
   return !*this;
  }

  //Unchecked error decapsulation
  constexpr auto& operator~() const noexcept {
   return storage.error;
  }

  //Unchecked error decapsulation
  constexpr auto& error() const noexcept {
   return ~*this;
  }
 };

 //Result deduction guides
 //Guide for results that always hold a value
 template<typename T>
 requires (!IsError<T>)
 Result(T) -> Result<T, Never>;

 //Guide for results that always hold an error
 template<IsError E>
 Result(E) -> Result<Never, E>;

 /*
 //TODO add overloads for functions that return `auto`

 //Shim to `Result<R, ...>::Result{R}`
 template<IsResult R>
 constexpr R ok(ResultType<R> result) {
  return {result};
 }

 //Shim to `Result<..., E>::Result{E}`
 template<IsResult R>
 constexpr R err(ResultError<R> error) {
  return {error};
 }
 */
}
