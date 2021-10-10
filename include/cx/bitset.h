#pragma once

#include <cx/idioms.h>

namespace CX {
 //Supporting meta-functions for bitset
 namespace BitsetMetaFunctions {
  namespace Internal {
   //Utility type meta-function that yields a buffer type with at least `N`
   //bits
   //Note: Naive implementation for platforms/compilers that do not support
   //N-bit integral types
   template<auto N>
   requires (N > 0)
   struct BufferForNBits final {
   private:
    using IndexType = ConstDecayed<decltype(N)>;

    //The number of bytes required to store `N` bits
    static constexpr auto const bytes = []() constexpr noexcept {
     auto bytes = N / 8;
     if (bytes * 8 != N) {
      bytes++;
     }
     return bytes;
    }();

    //TODO Utility type to support subscript-like access to the underlying bits
    struct BitReference final {

     constexpr BitReference(BitReference const&) noexcept = delete;
     constexpr BitReference(BitReference&&) noexcept = delete;


     constexpr BitReference& operator=(BitReference const&) noexcept = delete;
     constexpr BitReference& operator=(BitReference&&) noexcept = delete;
    };

    //Returns the min of 2 values
    static constexpr auto min(auto v1, auto v2) noexcept {
     return v1 < v2 ? v1 : v2;
    }

    //Utility fop binary bitwise operations
    template<auto OtherN>
    constexpr void binOp(BufferForNBits<N> const& other, auto op) noexcept {
     for (IndexType i = 0; i < min(N, OtherN); i++) {
      op(buffer[i], other.buffer[i]);
     }
    }

    //Utility for unary bitwise operations
    constexpr void unOp(auto op) noexcept {
     for (IndexType i = 0; i < N; i++) {
      op(buffer[i]);
     }
    }

   public:
    unsigned char buffer[bytes];

    //Default constructor
    constexpr BufferForNBits() noexcept = default;

    //Default copy-constructor
    constexpr BufferForNBits(BufferForNBits const&) noexcept = default;

    //Default move-constructor
    constexpr BufferForNBits(BufferForNBits&&) noexcept = default;

    //Constructor to initialize buffer from an integral value
    constexpr BufferForNBits(Integral auto value) noexcept {
     const_cast<BufferForNBits&>(*this).operator=(value);
    }

    //Differently-sized `BufferForNBits` copy-constructor
    template<auto OtherN>
    constexpr BufferForNBits(BufferForNBits<OtherN> const& other) noexcept {
     operator=(move(other));
    }

    //Differently-sized `BufferForNBits` move-constructor
    template<auto OtherN>
    constexpr BufferForNBits(BufferForNBits<OtherN>&& other) noexcept {
     operator=(move(other));
    }

    //Default copy-assignment operator
    constexpr BufferForNBits& operator=(BufferForNBits const&) noexcept
     = default;

    //Default move-assignment operator
    constexpr BufferForNBits& operator=(BufferForNBits&&) noexcept = default;

    //Integral assignment operator
    constexpr BufferForNBits& operator=(Integral auto value) noexcept {
     using IntegralType = ConstDecayed<ReferenceDecayed<decltype(value)>>;
     constexpr unsigned char mask = ~0;
     for (SizeType i = 0; i < sizeof(IntegralType); i++) {
      buffer[i / 8] = mask & (value << i);
     }
     return *this;
    }

    //Differently-sized copy-assignment operator
    template<auto OtherN>
    constexpr BufferForNBits& operator=(BufferForNBits<OtherN> const& other)
     noexcept
    {
     binOp(
      other,
      [](auto& b1, auto& b2) {
       b1 = b2;
      }
     );
     return *this;
    }

    //Differently-sized move-assginment operator
    template<auto OtherN>
    constexpr BufferForNBits operator=(BufferForNBits<OtherN>&& other) noexcept
    {
     binOp(
      other,
      [](auto& b1, auto& b2) {
       b1 = b2;
      }
     );
     return *this;
    }

    //Equality operator
    constexpr bool operator==(BufferForNBits const& other) const noexcept {
     for (IndexType i = 0; i < N; i++) {
      if (buffer[i] != other.buffer[i]) {
       return false;
      }
     }
     return true;
    }

    //Differently-sized equality operator
    template<auto OtherN>
    constexpr bool operator==(BufferForNBits<OtherN> const&) const noexcept {
     return false;
    }

    //Bitwise AND operator
    template<auto OtherN>
    constexpr void operator&(BufferForNBits<OtherN> const& other)
     noexcept
    {
     binOp(
      other,
      [](auto& b1, auto& b2) {
       b1 &= b2;
      }
     );
    }

    //Bitwise OR
    template<auto OtherN>
    constexpr void operator|(BufferForNBits<OtherN> const& other) noexcept {
     binOp(
      other,
      [](auto& b1, auto& b2) {
       b1 |= b2;
      }
     );
    }

    //Bitwise XOR
    template<auto OtherN>
    constexpr void operator^(BufferForNBits<OtherN> const& other) noexcept {
     binOp(
      other,
      [](auto& b1, auto& b2) {
       b1 ^= b2;
      }
     );
    }

    //Bitwise compl
    constexpr void operator~() noexcept {
     unOp([](auto& b1) {
      b1 = ~b1;
     });
    }

    //Bitwise left-shift
    constexpr void operator<<(IndexType const shift) noexcept {
     //TODO test this code
     for (IndexType i = 0; i < N; i++) {
      buffer[i] <<= shift;
      if (i != N - 1) {
       buffer[i] |= (~(~(unsigned char)0 << shift) & buffer[i + 1]);
      }
     }
    }

    //Bitwise right-shift
    constexpr void operator>>(IndexType const shift) noexcept {
     //TODO test this code
     for (IndexType i = N; i >= 0; i--) {
      buffer[i] >>= shift;
      if (i > 0) {
       buffer[i] |= (~(~(unsigned char)0 >> shift) & buffer[i - 1]);
      }
     }
    }
   };
  }

  //Returns an instance of bit-buffer backend
  template<auto N>
  requires (N > 0)
  constexpr auto bufferForNBits() noexcept {
   //Used to prevent conflicting return type deductions
   constexpr auto const useExtInt = [] {
    #ifdef CX_COMPILER_CLANG_LIKE
     return N < ((1 << 24) - 1);
    #else
     return false;
    #endif
   }();
   #ifdef CX_COMPILER_CLANG_LIKE
    //If using a clang-like compiler with `_ExtInt` support, use that instead
    //of the naive byte-buffer backend
    if constexpr (useExtInt) {
     using Type = unsigned _ExtInt(N);
     return Type{};
    }
   #endif
   //Return naive byte-buffer backend
   if constexpr (!useExtInt) {
    return Internal::BufferForNBits<N>{};
   }
  }
 }

 //Optimized buffer type supporting basic bitwise operations
 template<auto N>
 using BufferTypeForNBits = decltype(BitsetMetaFunctions::bufferForNBits<N>());

 //Forward declare `CX::Bitset<N>`
 template<typename T = SizeType, auto = T{}>
 struct Bitset;

 //TODO `CX::Bitset` identity concept
 template<typename T>
 concept IsBitset = false;

 //TODO Bitset speciailzation for custom backends
 template<typename T, auto>
 struct Bitset final {

 };

 //TODO
 // - Iterator for each bit
 // - Support user-defined backends
 // - Create a runtime backend for bitsets of non-constexpr sizes
 template<auto N>
 struct Bitset<decltype(N), N> final : Never {
  //Bit index type alias
  using IndexType = ConstDecayed<decltype(N)>;

 private:
  //Bit buffer type
  using BufferType = BufferTypeForNBits<N>;

  //Bit buffer
  BufferType buffer;

  //Returns mutable reference `*this`
  constexpr auto& mut() const noexcept {
   return const_cast<Bitset&>(*this);
  }

 public:
  //Lazy default constructor, no initialization performed
  constexpr Bitset() noexcept = default;

  //Default copy-constructor
  constexpr Bitset(Bitset const&) noexcept = default;

  //Default move-constructor
  constexpr Bitset(Bitset&&) noexcept = default;

  //Constructor to initialize bitset from an integral value
  constexpr Bitset(Integral auto value) noexcept {
   operator=(value);
  }

  //Differnetly-sized Bitset copy-constructor
  template<auto OtherN>
  constexpr Bitset(Bitset<OtherN> const& other) noexcept {
   //TODO
   (void)other;
  }

  //Differently-sized Bitset move-constructor
  template<auto OtherN>
  constexpr Bitset(Bitset<OtherN>&& other) noexcept {
   //TODO
   (void)other;
  }

  //Constexpr default destructor
  constexpr ~Bitset() noexcept = default;

  //TODO returns value of bit at `index`
  constexpr bool get(IndexType const index) const noexcept {
   (void)index;
   return false;
  }

  //TODO sets a bit to `value` at `index`
  constexpr bool set(IndexType const index, bool value) noexcept {
   (void)index;
   (void)value;
   return false;
  }

  //TODO reset all bits to `false`
  constexpr void reset(bool const value = false) noexcept {
   (void)value;
  }

  //TODO equality
  template<auto OtherN>
  constexpr bool operator==(Bitset<OtherN> const& other) noexcept {
   (void)other;
   return false;
  }

  //TODO Bitwise AND
  template<auto OtherN>
  constexpr Bitset operator&(Bitset<OtherN> const& other) const noexcept {
   (void)other;
   return {};
  }

  //TODO R-value bitwise AND
  template<auto OtherN>
  constexpr Bitset operator&(Bitset<OtherN>&& other) const noexcept {
   (void)other;
   return {};
  }

  //TODO Integral bitwise AND
  constexpr Bitset operator&(Integral auto other) const noexcept {
   (void)other;
   return {};
  }

  //TODO Bitwise OR
  template<auto OtherN>
  constexpr Bitset operator|(Bitset<OtherN> const& other) const noexcept {
   (void)other;
   return {};
  }

  //TODO Bitwise XOR
  template<auto OtherN>
  constexpr Bitset operator^(Bitset<OtherN> const& other) const noexcept {
   (void)other;
   return {};
  }

  //TODO Bitwise NOT
  constexpr Bitset operator~() const noexcept {
   return {};
  }

  //TODO Bitwise left-shift
  constexpr Bitset operator<<(IndexType const shift) const noexcept {
   (void)shift;
   return {};
  }

  //TODO Bitwise right-shift
  constexpr Bitset operator>>(IndexType const shift) const noexcept {
   (void)shift;
   return {};
  }

  //Default copy-assignment operator
  constexpr Bitset& operator=(Bitset const&) noexcept = default;

  //Default move-assignment operator
  constexpr Bitset& operator=(Bitset&&) noexcept = default;

  //Interal assignment operator
  constexpr Bitset& operator=(Integral auto value) noexcept {
   buffer = value;
   return *this;
  }

  //Differently-sized Bitset copy-assignment operator
  template<auto OtherN>
  constexpr Bitset& operator=(Bitset<OtherN> const& other) noexcept {
   buffer = other.buffer;
   return *this;
  }

  //Differently-sized Bitset move-assignment operator
  template<auto OtherN>
  constexpr Bitset& operator=(Bitset<OtherN>&& other) noexcept {
   return operator=(copy(other));
  }

  //Bit index operator
  constexpr BitReference operator[](IndexType bitIndex) noexcept {
   (void)bitIndex;

  }

  //Bit index operator
  constexpr BitReference const operator[](IndexType bitIndex) const noexcept {
   return mut()[bitIndex];
  }

  //Zero-check implicit bool conversion
  constexpr operator bool() const noexcept {
   return (bool)buffer;
  }

  //TODO all bitwise assignment operators
 };

 //TODO Empty Bitset specialization
 template<>
 struct Bitset<SizeType, 0> final : Never {
  //TODO all member functions
 };

 //Deduction guides for `Bitset<N>`
 template<Integral I>
 Bitset(I) -> Bitset<I, sizeof(I) * 8>;
};
