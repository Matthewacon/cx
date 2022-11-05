# CX Documentation

## Example use from CMake

Assuming you are using Clang or a Clang-like compiler, and want to integrate `CX` into an existing codebase, the following should get you started:

```cmake
project(example LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(cx
        GIT_REPOSITORY https://github.com/Matthewacon/cx
        GIT_TAG ff30870f700f36cb3fb4766b17b562b7c02bc5d5
)
FetchContent_MakeAvailable(cx)

# Add everything under ./src/*.cpp to the project
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(example ${SOURCES})

target_include_directories(example
        PRIVATE
                src
                ${cx_SOURCE_DIR}/include
)
target_compile_definitions(example
        PRIVATE
                CX_COMPILER_CLANG CX_COMPILER_CLANG_LIKE
                CX_STL_SUPPORT CX_LIBC_SUPPORT # Prevent some issues with redefinitions of things like std::align_val_t
                CX_HEADER_ONLY
                CX_NO_BELLIGERENT_ERRORS # Prevent CX from redefining exception primitives (try, throw, catch, etc.)
)
```

## Dependencies
 - [mdbook](https://github.com/rust-lang/mdBook)
 - [mdbook-linkcheck](https://github.com/Michael-F-Bryan/mdbook-linkcheck)
 - [mdbook-katex](https://github.com/Matthewacon/mdbook-katex)

## Building the docs
The build process will produce a static set of pages in `docs/build/html/`.
```sh
mdbook build docs/
```
