#include <cassert>
#include <iostream>

#include "ntbs.hpp"

using namespace ltl;

static_assert( ntbs::cat( ntbs::cat<',',' '>("Hello","world"), '!')
            == "Hello, world!" );

static_assert( ntbs::size('c') == 2 );
static_assert( *ntbs::data('c') == 'c' );

constexpr char c = 'c';
static_assert( ntbs::size(c) == 2 );
static_assert( *ntbs::data(c) == 'c' );

static_assert( ntbs::size("c") == 2 );
static_assert( ntbs::array{"c"} == ntbs::data("c") );

constexpr auto& c0 = "c";
static_assert( ntbs::size(c0) == 2 );
static_assert( ntbs::data(c0) == c0 );

constexpr char d0[] = "d";
static_assert( ntbs::size(d0) == 2 );
static_assert( ntbs::data(d0) == d0 );

static_assert( ntbs::size(decltype(d0){}) == 2 );
static_assert( *ntbs::data(decltype(d0){}) == '\0' );

constexpr auto const& a0 = ntbs::array{"a"};
static_assert( std::is_same_v< decltype(a0), ntbs::array<2> const&> );
static_assert( ntbs::size(a0) == 2 );
static_assert( ntbs::data(a0) == a0 );

// Note that iteration includes null terminator, just as for char[N]
//
constexpr auto test_range_for_iterate = [](auto const& a)
{
    int i = 0;
    for (auto&& e : a)
        i += (e == "hi"[i]);
    return i == sizeof(a) && a[i-1] == 0;
};
static_assert( test_range_for_iterate("hi") );
static_assert( test_range_for_iterate(ntbs::array{"hi"}) );

constexpr auto test_sized_iterate = [](auto const& a)
{
    int i = 0;
    while ( i != size(a) )
        ++i;
    return i == sizeof(a) && a[i-1] == 0;
};
static_assert( test_sized_iterate(ntbs::array{"ho"}) );

// test cats producing null array
constexpr auto null = ntbs::cat();
static_assert( null == "" );
static_assert( null == ntbs::cat(""));
static_assert( size(null) == 1 );
static_assert( null[0] == 0 );
static_assert( std::is_same_v< decltype(null), ntbs::array<1> const> );

// test cats producing single-element array
constexpr auto car = ntbs::cat('c');
constexpr auto cas = ntbs::cat("c");
static_assert( car == cas );
static_assert( car == "c" );
static_assert( size(car) == 2 );
static_assert( std::is_same_v< decltype(car), ntbs::array<2> const> );

// test cat multi-char string-literal and returned array type
constexpr auto hello = ntbs::cat("hello");
constexpr auto hollo = ntbs::cat(hello);
static_assert( hello == "hello" );
static_assert( hollo == "hello" );
static_assert( std::is_same_v< decltype(hello), ntbs::array<6> const> );
static_assert( std::is_same_v< decltype(hollo), ntbs::array<6> const> );

// test cut multi-char string-literal and returned array type
constexpr auto hullo = ntbs::cut("hello");
constexpr auto hullu = ntbs::cut(hello);
static_assert( hullo == "hello" );
static_assert( hullu == "hello" );
static_assert( std::is_same_v< decltype(hullo), ntbs::array<6> const> );
static_assert( std::is_same_v< decltype(hullu), ntbs::array<6> const> );

auto test_cat_throw = []
{
    bool thrown = false;
    using char5 = char[5];
    try {
        ntbs::cat(char5{'t','h','r','o','w'});
    }
    catch (...)
    {
        thrown = true;
    }
    return thrown;
};

auto test_cut_throw = []
{
    bool thrown = false;
    using char5 = char[5];
    try {
        ntbs::cut(char5{'t','h','r','o','w'});
    }
    catch (...)
    {
        thrown = true;
    }
    return thrown;
};


int main()
{
    using ntbs::cat;

    auto mut = cat("Hello");
    static_assert( sizeof mut == 6);
    mut[5] = '!';

#if NTBS_NULL_CHECK
    assert( test_cat_throw() );
    assert( test_cut_throw() );
#else
    assert( ! test_cat_throw() );
    assert( ! test_cut_throw() );
#endif
    {
        static_assert( cat('a','b') == "ab" );
        static_assert( cat('a',"b") == "ab" );
        static_assert( cat("a",'b') == "ab" );
        static_assert( cat("a","b") == "ab" );
    }
    {
        // 'Big Cat' for compile timing
		// (MSVC 19.2 warns that the K64 declaration will use 64K stack
		//  and the K256 big cat fails)

        //constexpr ntbs::array<65536> K64{};
        //constexpr auto K256 = cat(K64, K64, K64, K64); // 1.1s
        //static_assert(size(K256) == 262144 - 3);
    }
}