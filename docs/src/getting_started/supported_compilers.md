# Supported Compilers
## Requesting Support
If you do not see your compiler in the following list and would like support
added, feel free to make an issue at the repository's
[issue page](https://github.com/Matthewacon/CX/issues) with the tag
`Feature Request`.

Better yet, if you would like to implement support for a new compiler
make an issue at the [issue page](https://github.com/Matthewacon/CX/issues)
with the `Feature Request` tag and leave a comment noting that you
are self-assigning the issue. When you're all ready to make your PR, be
sure to reference the issue in question so reviewers know what your PR
is for!

## List of Supported Compilers
| | |
| :- | :- |
| 🟢 | Fully supported |
| 🔵 | Minor discrepancies; all features functional with minor issues |
| 🟣 | Moderate discrepancies; most features functional, some with potentially non-standard behaviour |
| 🟠 | Major discrepancies; many features non-functional or unimplemented |
| 🔴 | Not supported yet |

| Status | Compiler | Versions |
| :- | :- | :- |
| 🔵 | `Clang` | **10.x.x**+ |
| 🔵 | `AppleClang` | **12.0.5**+ |
| 🔵 | `Intel LLVM (dpc++)` | **10.x.x**+ |
| 🟠 | `GCC (unpatched)` | **10.x.x** - **12.x.x** |
| 🔵 | `GCC (patched)` | <= **10.2.x** |
| 🟢 | `GCC (patched)` | **10.3.x** - **12.x.x** |
| 🔴 | `MSVC` | Not supported yet |
| 🔴 | `Intel (icc)` | Note supported yet |

## Known Issues
### GCC (unpatched)
🟠 **<= 10.2.x**:
<table id="numbered-description-table">
 <tr><td>

  There is a known bug that rejects valid code when invoking
  variadic concepts without any arguemnts,
  [see here](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98717).

  **Note**: *This bug is fixed in GCC *10.3.x*+.*

  ---
  This does not immediately impact any CX facilities, however, you may
  experience compiler errors on valid code when using purely
  variadic concepts without arguments.

 </td></tr>
</table>

🟠 **10.x.x** - **12.x.x**:
<table id="numbered-description-table">
 <tr><td>

  There is an arbitrary limit on the complexity of constraints as
  represented internally by the GCC equation solver,
  [see here](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100828).

  **Note**: *The bug report contains a patch that removes the
  constraint complexity limit. After applying the patch, GCC
  compiles CX with no issues. The patch should work on GCC
  **11.x.x**+.*

  ---
  **Affected Constructs**:

  $$
   \begin{CD}
    \overbrace{\text{CX::FunctionWithPrototype}}^{\text{<cx/idioms.h>}} @<<depends<
    \begin{cases}
     \begin{CD}
      \overbrace{\text{CX::Lambda}}^{\text{<cx/lambda.h>}}
       @<<depends<
      \overbrace{\text{CX::Defer}}^{\text{<cx/memory.h>}}
      &\\
      \underbrace{\text{CX::AllocLambda}}_{\text{<cx/lambda.h>}}
       @<<depends<
      \underbrace{\text{CX::AllocDefer}}_{\text{<cx/memory.h>}}
     \end{CD}\\
    \end{cases}
   \end{CD}
  $$

  All affected constructs are unusable as this compiler bug rejects
  valid code.

  </td></tr>
</table>

### GCC (patched)
🟠 **<= 10.2.x**:

GCC versions <= 10.2.x, with the patch to fix [the constraint complexity limit bug](#description-13-2) applied, only suffer from the [variadic concept bug](#description-13-1).

### Clang-like Compilers
#### Clang
🔵 **10.x.x - 12.x.x**:
<table id="numbered-description-table">
 <tr><td>

  There is a known issue with the selection of partially specialized
  templates for zero-length-array types,
  [see here](https://bugs.llvm.org/show_bug.cgi?id=49808).

  ---
  **Affected Constructs:**

  $$
   \begin{CD}
    \overbrace{
     \begin{gather}
      \text{CX::Array}\\
      \text{CX::SizedArray}\\
      \text{CX::ArrayDecayed}\\
      \text{CX::ArraySize}\\
     \end{gather}
    }^{
     \text{<cx/idioms.h>}
    }
    @<<depends<
    \begin{cases}
     \overbrace{
      \text{CX::Variant}
     }^{
      \text{<cx/variant.h>}
     }
    \end{cases}
   \end{CD}
  $$

  This compiler bug causes two behavioural changes:
  1. All non-concept constructs will yield a compiler error when invoked
  with a zero-length-array type; `CX::ArrayDecayed`, `CX::ArraySize` and
  `CX::Variant`.
  2. All concept constructs will be unsatisfied for zero-length-array
  types; `CX::Array` and `CX::SizedArray`.
 </td></tr>
 <tr><td>

  There is a known issue with nested auto lambda incovations that causes
  a compiler crash, [see here](https://bugs.llvm.org/show_bug.cgi?id=49743).

  ---
  This does not immediately impact any CX facilities, however, you may
  experience compiler crashes if you invoke nested auto lambdas inside
  of a lambda passed to constructs within CX.

 </td></tr>
</table>

#### AppleClang
🔵 **12.0.5**+:

AppleClang suffers from the same bugs as Clang:
 1. [Template specialization and deduction with zero-length-arrays](#description-17-1)
 2. [Nested auto lambda invocation limit](#description-17-2)

#### IntelLLVM (dpc++)
🔵 **10.x.x**+:

The Intel LLVM compiler collection suffers from the same bugs as Clang:
 1. [Template specialization and deduction with zero-length-arrays](#description-17-1)
 2. [Nested auto lambda invocation limit](#description-17-2)

### MSVC
TODO

### Intel (icc)
TODO
