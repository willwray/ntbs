//  Copyright (c) 2019 Will Wray https://keybase.io/willwray
//
//  Distributed under the Boost Software License, Version 1.0.
//        http://www.boost.org/LICENSE_1_0.txt
//
//  Repo: https://github.com/willwray/ntbs

#ifndef LTL_NTBS_HPP
#define LTL_NTBS_HPP

#include <cstdint>
#include <type_traits>

/*
   "ntbs.hpp": Concatenation and slicing of 'constexpr C strings'
    ^^^^^^^^
    Here 'NTBS' are constexpr 'null-terminated byte strings' defined as
    char array types of static size, including a char for the expected
    null terminator at size - 1. Embedded null characters may be present.

     namespace ltl::ntbs;

     cat(ntbs_args...);   // Concatenate the ntbs_args
     cat<s>(ntbs_args...) // Join ntbs_args with separator char s...
     cut<B>(ntbs_arg);    // Slice range [B,end), a suffix of ntbs_arg
     cut<B,E>(ntbs_arg);  // Slice range [B,E) of ntbs_arg

   Hello-world example:

     #include "ntbs.hpp"
     using namespace ltl::ntbs;
     constexpr auto hello_world = cat(cat<',',' '>("Hello","world"),'!');
     constexpr auto hello_comma = cut<0,6>(hello_world);  // prefix 6+'\0'
     constexpr auto world_exclaim = cut<-7>(hello_world); // suffix 6+'\0'
                              // == cut<-7,-1>(hello_world)
     #include <cstdio>
     int main() { puts( cat<' '>(hello_comma, world_exclaim) ); }

   Outputs "Hello, world!"

   The return value is of an internal character array class type.
   It is usable as char[N] via implicit conversion to its char array.
   It is usable as an argument for char* function parameters via
   implicit conversion to char array then array decay to pointer.

   Input NTBS args can be char[N] or a generic Char<N> class type.
   Generic access is via free-function 'data' and 'size' overloads
   plus an 'extent' trait giving the static size:

     data(Char<N>{}) returns a const pointer to the array begin char
     size(Char<N>{}) returns array size N, including null terminator
     extent<Char<N>> also gives array size N as an integral_constant.

   Char<N> generalizes string-literal and constexpr char[N] NTBS.

   'Static size' means that size is encoded in the type.
   The purpose of this lib is to auto-deduce and track the size.
   The proposed C++20 constexpr std::string will replace many uses
   of these 'sized' ntbs utilites with its 'size-erased' container.

 */

// NTBS_NULL_CHECK Debug macro - if defined then add null checks which
// throw if any arg is not null-terminated.
// The throw causes a compile error in constexpr context.
//
//#define NTBS_NULL_CHECK

namespace ltl {

namespace ntbs {

// Generic interface for array type A:
//  * data(A) function returning char* or char(&)[N]
//  * size(A) function returning array size N
//  * extent<A> trait for array size N

template <typename A>
struct extent;

template <typename A>
inline constexpr int32_t extent_v{extent<A>()};

// char[N] extent, data and size
// Free function data(char[N]) and size(char[N]) overloads for char arrays.
// Override std::size and std::data overloads, from <iterator>, if visible.
//
template <int32_t N>
struct extent<char[N]> : std::integral_constant<int32_t,N> {};
template <int32_t N>
constexpr auto const& data(char const(&a)[N]) noexcept { return a; }
template <int32_t N>
constexpr int32_t size(char const(&)[N]) noexcept { return extent_v<char[N]>; }

// ntbs::array<N>: char array class intended for null-terminated char strings.
// Implicit conversion to char array (ref) and so full decay-to-pointer char*.
//
template <int32_t N>
struct array
{
    char data[N];
    constexpr operator decltype(data)&() noexcept { return data; }
    constexpr operator decltype(data) const&() const noexcept { return data; }
};

template <int32_t N>
array(char const(&)[N]) -> array<N>;

template <int32_t N>
struct extent<array<N>> : std::integral_constant<int32_t,N> {};

template <int32_t N>
constexpr auto const* begin(array<N> const& a) noexcept { return a.data; }

template <int32_t N>
constexpr auto const* end(array<N> const& a) noexcept { return a.data + N; }

template <int32_t N>
constexpr auto const& data(array<N> const& a) noexcept { return a.data; }

template <int32_t N>
constexpr int32_t size(array<N> const&) noexcept { return N; }

// constexpr array == ntbs comparison operator, asymmetric (array lhs).
//
template <int32_t L, typename R>
constexpr bool operator==(array<L> const& lhs, R const& rhs) noexcept
{
    return L == extent_v<R> && [](char const(&l)[L], char const* r) {
        for (int32_t i = 0; i != L; ++i)
            if ( l[i] != r[i] )
                return false;
        return true;
    }(lhs.data, data(rhs));
}

// copy_n; C++17 constexpr version of std::copy_n, char-specific.
//
constexpr char* copy_n(char const* src, int32_t sz, char* dest)
{
    for (int i = 0; i != sz; ++i)
        dest[i] = src[i];
    return dest + sz;
}

// cut<B,E>(char[N]) Returns sub-array [B,E) of char array arg.
// The result is returned as ntbs::array<M> type with zero terminator.
// The [B,E) indices are signed integers (c.f. Python slice indexing).
// The input char array argument is assumed to be zero terminated.
//
template <int32_t B = 0, int32_t E = -1, typename A>
constexpr
auto
cut(A const& a)
{
    constexpr int32_t N = extent_v<A>;
#if defined(NTBS_NULL_CHECK)
    if constexpr ( sizeof(A) != 1 || N == 1 )
        if ( data(a)[N - 1] != 0 )
            throw "ntbs::cut arg not null-terminated";
#endif
    constexpr auto ind = [](int32_t i) -> int32_t {
        return i < 0 ? N + i : i;
    };
    constexpr int32_t b = ind(B), e = ind(E);
    static_assert( 0 <= b && b <= e && e <= N,
                   "index out of bounds");
    constexpr int32_t M = e - b;
    array<M + 1> chars{};
    copy_n( data(a) + b, M, chars);
    return chars;
}

// Single-char extent, data and size
// Free function data(c) and size(c) overloads for char c.
// Note that extent is 2, as-if a null-terminated char string.
//
template <>
struct extent<char> : std::integral_constant<int32_t,2> {};
constexpr char const* data(char const& c) noexcept { return &c; }
constexpr int32_t size(char const&) noexcept { return extent_v<char>; }

// cat(cs...) Concatenate input character sequences, char or ntbs, or
// join the inputs using variadic char template args act as separator.
// Return in an ntbs::array<M> type with zero terminator.
//
template <char... sep,
      typename... Cs>
constexpr
auto
cat(Cs const&... cs)
{
#if defined(NTBS_NULL_CHECK)
    ([](char const* dat) {
        if constexpr ( sizeof(Cs) != 1 || extent_v<Cs> == 1 )
            if ( dat[sizeof(Cs) - 1] != 0 )
                throw "ntbs::cut arg not null-terminated";
    }(data(cs)), ...);
#endif
    array<((extent_v<Cs> + sizeof...(sep) - 1) + ...
                        +(-sizeof...(sep) + 1))> acc{};
    char* p = acc;
    (( p = p == acc ? p : (((*p++ = sep), ...), p),
       p =  copy_n(data(cs), extent_v<Cs>-1, p)), ...);
    return acc;
}

} // namespace ntbs

} // namespace ltl

#endif
