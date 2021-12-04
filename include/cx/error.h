#pragma once

#include <cx/common.h>
#include <cx/idioms.h>

//TODO Add a `cleanup` chain to the error hierarchy

//Flags to configure error message bahaviour
#ifdef CX_ERROR_MSG
 //Configure default error message behaviour based on STL support
 #if !defined(CX_ERROR_MSG_ALLOC) && !defined(CX_ERROR_MSG_BUF)
  #ifdef CX_STL_SUPPORT
   //Enable message allocation if building with STL support
   #define CX_ERROR_MSG_ALLOC
  #else
   //Enable message buffers if building without STL support
   #define CX_ERROR_MSG_BUF 1024
  #endif //CX_STL_SUPPORT
 #endif //!defined(CX_ERROR_MSG_ALLOC) && !defined(CX_ERROR_MSG_BUF)

 //Sanitize error message behaviour flags
 #if defined(CX_ERROR_MSG_ALLOC) && defined(CX_ERROR_MSG_BUF)
  #error \
   Only one of `CX_ERROR_MSG_ALLOC` or `CX_ERROR_MSG_BUF` may be enabled\
   at one time.
 #endif //defined(CX_ERROR_MSG_ALLOC) && defined(CX_ERROR_MSG_BUF)

 //Sanitize `CX_ERROR_MSG_BUF` flag
 #ifdef CX_ERROR_MSG_BUF
  //Error if not a positive integer
  #if CX_ERROR_MSG_BUF < 0
   #error `CX_ERROR_MSG_BUF` must be >= 0
  #endif //CX_ERROR_MSG_BUF < 0

  //Warning if buffer uses excessive space
  #if CX_ERROR_MSG_BUF > 1024
   CX_DEBUG_MSG((
    "CX_ERROR_MSG_BUF" is recommended to be in the range
    "0 <= CX_ERROR_MSG_BUF <= 1024". Using values larger than 1024 may incur
    significant overhead, or even stack buffer overflows. Consider using a
    smaller value.
   ));
  #endif //CX_ERROR_MSG_BUF > 1024
 #endif //CX_ERROR_MSG_BUF
#endif //CX_ERROR_MSG

//Flags to configure error tracing behaviour
#ifdef CX_ERROR_TRACE
 //Sanitize `CX_ERROR_TRACE` flag
 #ifndef CX_STL_SUPPORT
  #error `CX_ERROR_TRACE` requires STL support.
 #endif //CX_STL_SUPPORT
#endif //CX_ERROR_TRACE

namespace CX {
 //Forward `CX::Error` for use with error meta-functions
 struct Error;

 //Supporting meta-functions
 namespace ErrorMetaFunctions {
  template<SizeType N>
  void expect(char const(&)[N]) noexcept;
 }

 //Error identity concept
 template<typename MaybeError>
 concept IsError =
  requires (char const * (MaybeError::*f)() const noexcept) {
   f = &MaybeError::describe;
  }
  || requires (MaybeError m) {
   ErrorMetaFunctions::expect(m.describe());
  };

 namespace Internal {
  //Constexpr-compatible error message container
  //TODO Optimize string copying to make it trivial for the optimizer
  struct ErrorMessage final {
  private:
   //Constexpr cstring copy
   static constexpr void cstrcpy(char * dst, char const * src, SizeType dstMax)
    noexcept
   {
    SizeType i = 0;
    for (; i < dstMax && i[src]; i++) {
     dst[i] = src[i];
    }
    dst[i] = '\00';
   }

   //Constexpr cstring length
   static constexpr SizeType cstrlen(char const * str) noexcept {
    SizeType len = 0;
    while (str[len]) len++;
    return len;
   }

   constexpr ErrorMessage& mut() const noexcept {
    return const_cast<ErrorMessage&>(*this);
   }

  public:
   SizeType size;
   #ifdef CX_ERROR_MSG_ALLOC
    //Constexpr-compatible allocating message impl
    char * message;

    //Default constructor
    constexpr ErrorMessage() noexcept :
     size{0},
     message{nullptr}
    {}

    //Constexpr cstring copy constructor
    template<SizeType N>
    constexpr ErrorMessage(char const(&message)[N]) noexcept :
     size{N},
     message{new char[N + 1]{0}}
    {
     //Copy message contents
     cstrcpy(this->message, &message[0], N);
    }

    //Constexpr copy constructor
    constexpr ErrorMessage(ErrorMessage const& other) noexcept :
     size{0},
     message{nullptr}
    {
     mut().operator=((ErrorMessage const&)other);
    }

    //Constexpr move constructor
    constexpr ErrorMessage(ErrorMessage&& other) noexcept :
     size{0},
     message{nullptr}
    {
     mut().operator=((ErrorMessage&&)other);
    }

    //Constexpr destructor
    constexpr ~ErrorMessage() noexcept {
     delete[] message;
    }

    //Constexpr copy-assignment operator
    constexpr ErrorMessage& operator=(ErrorMessage const& other) noexcept {
     //Reallocate buffer if it is not large enough
     if (other.size > size || !message) {
      delete[] message;
      size = other.size;
      //Note: `+1` for null terminator
      message = new char[size + 1];
     }
     //Copy message contents
     cstrcpy(message, other.message, size);
     return *this;
    }

    //Constexpr move-assignment operator
    constexpr ErrorMessage& operator=(ErrorMessage&& other) noexcept {
     size = other.size;
     message = other.message;
     other.size = 0;
     other.message = nullptr;
     return *this;
    }

    //Resets stored message
    constexpr void reset() noexcept {
     //Note: All messages will allocate at least enough for a null terminator
     //so this will always be in-bounds
     if (message) {
      message[0] = '\00';
     }
    }

    //Copies cstring
    constexpr ErrorMessage& from(char const * str) noexcept {
     auto len = cstrlen(str);
     //Reallocate buffer if it is not large enough
     if (len > size || !message) {
      delete[] message;
      message = new char[len + 1];
      size = len;
     }
     //Copy message contents
     cstrcpy(message, str, len);
     return *this;
    }
   #elif defined(CX_ERROR_MSG_BUF)
    //Constexpr-compatible non-allocating message impl
    char message[CX_ERROR_MSG_BUF];

    //Default constructor
    constexpr ErrorMessage() noexcept :
     size{0},
     message{0}
    {}

    //Constexpr cstring copy constructor
    template<SizeType N>
    constexpr ErrorMessage(char const(&message)[N]) noexcept :
     size{N},
     message{0}
    {
     //Copy message contents
     cstrcpy(this->message, message, CX_ERROR_MSG_BUF);
    }

    //Constexpr copy constructor
    constexpr ErrorMessage(ErrorMessage const& other) noexcept :
     size{0},
     message{0}
    {
     mut().operator=((ErrorMessage const&)other);
    }

    //Constexpr move constructor
    constexpr ErrorMessage(ErrorMessage&& other) noexcept :
     size{0},
     message{0}
    {
     mut().operator=((ErrorMessage&&)other);
    }

    //Constexpr default destructor
    constexpr ~ErrorMessage() noexcept = default;

    //Constexpr copy-assignment operator
    constexpr ErrorMessage& operator=(ErrorMessage const& other) noexcept {
     size = other.size;
     cstrcpy(message, other.message, CX_ERROR_MSG_BUF - 1);
     return *this;
    }

    //Constexpr move-assignment operator
    constexpr ErrorMessage& operator=(ErrorMessage&& other) noexcept {
     operator=((ErrorMessage const&)other);
     other.reset();
     return *this;
    }

    //Resets stored message
    constexpr void reset() noexcept {
     if constexpr (CX_ERROR_MSG_BUF > 0) {
      message[0] = '\00';
     }
    }

    //Copies cstring
    constexpr ErrorMessage& from(char const * str) noexcept {
     auto len = cstrlen(str);
     if (len > CX_ERROR_MSG_BUF - 1) {
      len = CX_ERROR_MSG_BUF - 1;
     }
     cstrcpy(message, str, len);
     return *this;
    }
   #else
    //Catch-all nop constructor
    constexpr ErrorMessage(auto) noexcept {}

    //Default nop constructor
    constexpr ErrorMessage() noexcept = default;

    //nop stub
    constexpr ErrorMessage& from(char const *) noexcept {
     return *this;
    }
   #endif //CX_ERROR_MSG_ALLOC

   //Returns cstring message
   constexpr char const * get() const noexcept {
    #if defined(CX_ERROR_MSG_BUF)
     if (CX_ERROR_MSG_BUF > 0 && message[0]) {
      return message;
     }
    #elif defined(CX_ERROR_MSG_ALLOC)
     return message;
    #endif //defined(CX_ERROR_MSG_BUF)
    return nullptr;
   }
  };
 }

 //Flag for `CX::Error` constructor initializer lists
 #if defined(CX_ERROR_MSG) || defined(CX_ERROR_TRACE)
  #define CX_ERROR_CTOR_INIT_LIST
 #endif //defined(CX_ERROR_MSG) || defined(CX_ERROR_TRACE)

 //CX error base
 struct Error final {
 private:
  using Message = Internal::ErrorMessage;

  #ifdef CX_ERROR_MSG
   Message message{};
  #endif //CX_ERROR_MSG
  #ifdef CX_ERROR_TRACE
   Error * prev = nullptr;
  #endif //CX_ERROR_TRACE

  //Causes `sizeof(Error) == 0` when compiling without error messages or
  //tracing
  unsigned char unused[0]{};

  constexpr Error& mut() const noexcept {
   return const_cast<Error&>(*this);
  }

  //Constructor to create an error from a user-defined error-like object
  constexpr Error(Message&& message) noexcept {
   (void)message;
   #ifdef CX_ERROR_MSG
    mut().message = message;
   #endif
  }

 public:
  //Default constructor
  constexpr Error() noexcept = default;

  //User supplied message constructor
  template<SizeType N>
  constexpr Error(char const(&message)[N]) noexcept {
   (void)message;
   #ifdef CX_ERROR_MSG
    mut().message = message;
   #endif
  }

  //User supplied message and cause constructor
  template<SizeType N>
  constexpr Error(char const(&message)[N], IsError auto const& error) noexcept
  {
   auto& ref = mut();
   //Silence `unused` errors when compiling without messages or tracing
   (void)message;
   (void)error;
   (void)ref;
   #ifdef CX_ERROR_MSG
    ref.message = message;
   #endif //CX_ERROR_MSG
   #ifdef CX_ERROR_TRACE
    using EType = Unqualified<decltype(error)>;
    if constexpr (SameType<Error, EType>) {
     //Copy construct error
     ref.prev = new Error{(EType const&)error};
    } else {
     //Create error from error-like object
     //TODO check for `error.describe()` prototype and prefer
     //`ErrorMessage::ErrorMessage(char const(&)[N])` constructor over
     //`ErrorMessage::from(char const *)` to optimize error message
     //initialization
     ref.prev = new Error{Message{}.from(error.describe())};
    }
   #endif //CX_ERROR_TRACE
  }

  //User defined error copy constructor
  constexpr Error(IsError auto const& error) noexcept {
   mut().operator=((decltype(error) const&)error);
  }

  //Error copy constructor
  //Note: Does not support user defined errors
  constexpr Error(Error const& error) noexcept {
   mut().operator=((Error const&)error);
  }

  //Error move constructor
  //Note: Does not support user defined errors
  constexpr Error(Error&& error) noexcept {
   mut().operator=((Error&&)error);
  }

  constexpr ~Error() noexcept {
   //Suppress "private member is unused" errors
   (void)unused;
   #ifdef CX_ERROR_TRACE
    delete prev;
   #endif //CX_ERROR_TRACE
  }

  //Error copy-assignment operator
  //Note: Supports user defined errors
  constexpr Error& operator=(IsError auto const& error) noexcept {
   using EType = Unqualified<decltype(error)>;
   if constexpr (SameType<Error, EType>) {
    operator=((Error const&)error);
   } else {
    //Copy user error message
    //TODO check for `error.describe()` prototype and prefer
    //`ErrorMessage::ErrorMessage(char const(&)[N])` constructor over
    //`ErrorMessage::from(char const *)` to optimize error message
    //initialization
    #ifdef CX_ERROR_MSG
     message.from(error.describe());
    #endif //CX_ERROR_MSG
   }
   return *this;
  }

  //Error copy-assignment operator
  constexpr Error& operator=(Error const& error) noexcept {
   //Silence `unused` errors when compiling without messages or tracing
   (void)error;
   //Copy error message
   #ifdef CX_ERROR_MSG
    message = (Message const&)error.message;
   #endif //CX_ERROR_MSG
   //Copy error cause, if present
   #ifdef CX_ERROR_TRACE
    if (error.prev) {
     prev = new Error{*error.prev};
    }
   #endif //CX_ERROR_TRACE
   return *this;
  }

  //Error move-assignment operator
  //Note: Does not support user defined errors
  constexpr Error& operator=(Error&& error) noexcept {
   //Silence `unused` errors when compiling without messages or tracing
   (void)error;
   //Move error message
   #ifdef CX_ERROR_MSG
    message = (Message&&)error.message;
   #endif
   //Move error cause
   #ifdef CX_ERROR_TRACE
    prev = error.prev;
    error.prev = nullptr;
   #endif
   return *this;
  }

  //Returns the error message
  //Note: When building without `CX_ERROR_MSG`, returns nullptr
  constexpr char const * describe() const noexcept {
   #ifdef CX_ERROR_MSG
    return message.get();
   #else
    return nullptr;
   #endif //CX_ERROR_MSG
  }

  constexpr Error * cause() const noexcept {
   #ifdef CX_ERROR_TRACE
    return prev;
   #else
    return nullptr;
   #endif //CX_ERROR_TRACE
  }
 };
}
