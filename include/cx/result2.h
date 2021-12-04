#include <cx/common.h>
#include <cx/utility.h>
#include <cx/idioms.h>
#include <cx/templates.h>
#include <cx/error.h>
#include <cx/exit.h>

namespace CX {
 //Supporting exceptions for `CX::Result`
 struct ResultValueNotPresentError final : Never {
  static constexpr auto& describe() noexcept {
   return "Result value not present!";
  }
 };
 static_assert(IsError<ResultValueNotPresentError>);

 struct ResultErrorNotPresentError final : Never {
  static constexpr auto& describe() noexcept {
   return "Result value not present!";
  }
 };
 static_assert(IsError<ResultErrorNotPresentError>);

 struct ResultValueInitializationError final : Never {
  static constexpr auto& describe() noexcept {
   return "Failed to copy or move initialize result value!";
  }
 };
 static_assert(IsError<ResultValueInitializationError>);

 struct ResultErrorInitializationError final : Never {
  static constexpr auto& describe() noexcept {
   return "Failed to copy or move initialize result error!";
  }
 };
 static_assert(IsError<ResultErrorInitializationError>);

 //Forward declare `CX::Result` for use with meta-functions
 template<typename...>
 struct Result;

 //Result error type identity concept
 template<typename E>
 concept ResultErrorParameter = IsError<E>;

 //Result value type identity concept
 template<typename V>
 concept ResultValueParameter = IsCopyableOrMovable<V>;
  /*
  (CopyConstructible<E> || (Constructible<E> && CopyAssignable<E>))
  || (MoveConstructible<E> || (Constructible<E> && MoveAssignable<E>));
  */

 //Supporting meta-functions
 namespace ResultMetaFunctions {
  template<typename>
  struct IsResult : FalseType {};

  template<template<typename...> typename R, typename V, typename E>
  requires (
   CX::SameTemplateType<R, Result>
   && ResultValueParameter<V>
   && ResultErrorParameter<E>
  )
  struct IsResult<R<V, E>> : TrueType {
   using ValueType = V;
   using ErrorType = E;
  };
 }

 //Result identity concept
 template<typename MaybeResult>
 concept IsResult = ResultMetaFunctions
  ::IsResult<MaybeResult>
  ::Value;

 //Result value type meta-function
 template<IsResult R>
 using ResultValueType = typename ResultMetaFunctions
  ::IsResult<R>
  ::ValueType;

 //Result error type meta-function
 template<IsResult R>
 using ResultErrorType = typename ResultMetaFunctions
  ::IsResult<R>
  ::ErrorType;

 //Partial specialization for either error or value results
 template<ResultValueParameter V, ResultErrorParameter E>
 struct Result<V, E> final {
 private:
  template<typename...>
  friend struct Result;

  enum struct State : unsigned char {
   ERR,
   OK,
   INVALID
  } state : 2;

  union U {
   V value;
   E error;

   constexpr U() noexcept {}
   constexpr ~U() noexcept {}
  } u;

  //Returns mutable reference to this
  constexpr auto& mut() const noexcept {
   return const_cast<Result&>(*this);
  }

  //Destructs currently active element and initializes as `State::INVALID`
  constexpr void reset() noexcept {
   auto const oldState = state;
   state = State::INVALID;
   switch (oldState) {
    case State::OK: {
     destroy(u.value);
     break;
    }
    case State::ERR: {
     destroy(u.error);
     break;
    }
    default: break;
   }
  }

  //Initialize Result from value
  template<typename T>
  constexpr void initValue(T v) noexcept {
   //Copy or move initalize value
   auto const op = copyOrMove(
    u.value,
    (T)v,
    state == State::OK,
    [&](V&) noexcept {
     reset();
    }
   );

   //Fail if value could not be initialized
   if (op == CopyOrMoveOperation::NONE) {
    exit(ResultValueInitializationError{});
   }

   //Update state to reflect value presence
   state = State::OK;
  }

  //Initialize Result from error
  template<typename T>
  constexpr void initError(T e) noexcept {
   //Copy or move initalize error
   auto const op = copyOrMove(
    u.error,
    (T)e,
    state == State::ERR,
    [&](E&) noexcept {
     reset();
    }
   );

   //Fail if error could not be initialized
   if (op == CopyOrMoveOperation::NONE) {
    exit(ResultErrorInitializationError{});
   }

   //Update state to reflect error presence
   state = State::ERR;
  }

  //Initializes from another error
  template<typename T>
  constexpr void initFromResult(T r) noexcept {
   constexpr auto
    copyable = IsCopyable<T>,
    movable = IsMovable<T>,
    preferCopy = SameType<T, Result const&>,
    copy = preferCopy ? copyable : movable;

   //Copy or move error or value from other result
   switch (r.state) {
    case State::INVALID: {
     reset();
     break;
    }
    case State::OK: {
     using ValueType = SelectType<copy, V const&, V&&>;
     initValue<ValueType>((ValueType)r.u.value);
     break;
    }
    case State::ERR: {
     using ErrorType = SelectType<copy, E const&, E&&>;
     initError<ErrorType>((ErrorType)r.u.error);
     break;
    }
   }

   //If operation was a move, reset other result
   if constexpr (!copy) {
    r.reset();
   }
  }

 public:
  //Default invalid state constructor
  constexpr Result() noexcept :
   state{State::INVALID}
  {}

  //Error copy constructor
  constexpr Result(E const& e) noexcept :
   state{State::INVALID}
  {
   mut().operator=((E const&)e);
  }

  //Error move constructor
  constexpr Result(E&& e) noexcept :
   state{State::INVALID}
  {
   mut().operator=((E&&)e);
  }

  //Value copy constructor
  constexpr Result(V const& v) noexcept :
   state{State::INVALID}
  {
   //TODO REMOVE
   if (!isConstexpr()) {
    printf("Result::Result(V const&)\n");
   }
   mut().operator=((V const&)v);
  }

  //Value move constructor
  constexpr Result(V&& v) noexcept :
   state{State::INVALID}
  {
   //TODO REMOVE
   if (!isConstexpr()) {
    printf("Result::Result(V&&)\n");
   }
   mut().operator=((V&&)v);
  }

  //Result copy constructor
  constexpr Result(Result const& r) noexcept :
   state{State::INVALID}
  {
   mut().operator=((Result const&)r);
  }

  //Result move constructor
  constexpr Result(Result&& r) noexcept :
   state{State::INVALID}
  {
   mut().operator=((Result&&)r);
  }

  //Constexpr destructor
  constexpr ~Result() noexcept {
   mut().reset();
  }

  //Error copy-assignment operator
  constexpr Result& operator=(E const& e) noexcept {
   initError<E const&>((E const&)e);
   return *this;
  }

  //Error move-assignment operator
  constexpr Result& operator=(E&& e) noexcept {
   initError<E&&>((E&&)e);
   return *this;
  }

  //Value copy-assignment operator
  constexpr Result& operator=(V const& v) noexcept {
   //TODO REMOVE
   if (!isConstexpr()) {
    printf("Result::operator=(V const&)\n");
   }
   initValue<V const&>((V const&)v);
   return *this;
  }

  //Value move-assignment operator
  constexpr Result& operator=(V&& v) noexcept {
   //TODO REMOVE
   if(!isConstexpr()) {
    printf("Result::operator=(V&&)\n");
   }
   initValue<V&&>((V&&)v);
   return *this;
  }

  //Result copy-assignment operator
  constexpr Result& operator=(Result const& r) noexcept {
   initFromResult<Result const&>((Result const&)r);
   return *this;
  }

  //Result move-assignment operator
  constexpr Result& operator=(Result&& r) noexcept {
   initFromResult<Result&&>((Result&&)r);
   return *this;
  }

  //Value presence check
  constexpr bool hasValue() const noexcept {
   return state == State::OK;
  }

  //Error presence check
  constexpr bool hasError() const noexcept {
   return state == State::ERR;
  }

  //Value presence implicit conversion operator
  constexpr operator bool() const noexcept {
   return hasValue();
  }

  //TODO
  // - Doc
  // - Error conjunction or disjunction?
  template<IsResult T>
  constexpr auto andThen(
   FunctionWithPrototype<T (V&) noexcept> auto f
  ) const
   noexcept
  {
   (void)f;
   //TODO
  }

  //TODO
  // - Doc
  // - Error conjunction or disjunction?
  template<IsResult T>
  constexpr auto orElse(
   FunctionWithPrototype<T (E&) noexcept> auto f
  ) const noexcept {
   (void)f;
   //TODO
  }

  //"Value or immediate" operator
  constexpr auto operator||(V other) const noexcept {
   return hasValue() ? u.value : other;
  }

  //"Value or lazily evaluated result" operator
  constexpr auto operator||(
   FunctionWithPrototype<V () noexcept> auto&& generator
  ) const noexcept {
   return hasValue() ? u.value : generator();
  }

  //Checked value decapsulation
  constexpr auto& getValue() const noexcept {
   if (hasValue()) {
    return u.value;
   }
   exit(ResultValueNotPresentError{});
  }

  //Checked value decapsulation
  constexpr auto& operator++() const noexcept {
   return getValue();
  }

  //Unchecked value decapsulation
  constexpr auto& operator+() const noexcept {
   return u.value;
  }

  //Checked error decapsulation
  constexpr auto& getError() const noexcept {
   if (hasError()) {
    return u.error;
   }
   exit(ResultErrorNotPresentError{});
  }

  //Checked error decapsulation
  constexpr auto& operator--() const noexcept {
   return getError();
  }

  //Unchecked error decapsulation
  constexpr auto& operator-() const noexcept {
   return u.error;
  }
 };

 //TODO Partial specialization for guaranteed value results
 template<ResultValueParameter V>
 requires (!ResultErrorParameter<V>)
 struct Result<V> final {
 private:
  template<typename...>
  friend struct Result;

  union U {
   V value;

   constexpr U() noexcept {}
   constexpr ~U() noexcept {}
  } u;
 };

 //Partial specialization for guaranteed error results
 template<ResultErrorParameter E>
 struct Result<E> final {
 private:
  template<typename...>
  friend struct Result;

  union U {
   E error;

   constexpr U() noexcept {}
   constexpr ~U() noexcept {}
  } u;
 };
}
