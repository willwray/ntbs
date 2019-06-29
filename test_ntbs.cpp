#include "ntbs.hpp"

using namespace ltl;

static_assert( ntbs::cat( ntbs::cat<',',' '>("Hello","world"), '!')
            == "Hello, world!" );

// test 'size' and 'data' overloads
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

constexpr auto a0 = ntbs::array{"a"};
static_assert( sizeof a0 == 2 );
static_assert( size(a0) == 2 );
static_assert( data(a0) == a0 );

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
static_assert( sizeof null == 1 );
static_assert( size(null) == 1 );
static_assert( null[0] == 0 );

// test cats producing single null-terminated char
constexpr auto car = ntbs::cat('c');
constexpr auto cas = ntbs::cat("c");
constexpr auto cac = cat(car); // ADL
static_assert( car == cas );
static_assert( car == cac );
static_assert( sizeof car == 2 );
static_assert( size(car) == 2 );

// test cat multi-char string-literal and returned array type
constexpr auto hello = ntbs::cat("hello");
constexpr auto hollo = cat(hello);
static_assert( hello == "hello" );
static_assert( hollo == "hello" );
static_assert( sizeof hello == 6 );
static_assert( size(hello) == 6 );

// test null cut
constexpr auto cul = ntbs::cut("");
static_assert( cul == ntbs::cut<0>("") );
static_assert( cul == ntbs::cut<-1>("") );
static_assert( cul == ntbs::cut<0,0>("") );
static_assert( cul == ntbs::cut<0,-1>("") );
static_assert( cul == ntbs::cut<-1,0>("") );
static_assert( cul == ntbs::cut<-1,-1>("") );
static_assert( sizeof cul == 1 && cul[0] == 0);

// test cut of single char
constexpr auto cut1 = ntbs::cut("c");
static_assert( cut1 == "c" );
static_assert( cut(cut1) == "c" );
static_assert( ntbs::cut<0>(cut1) == "c" ); // No ADL with explicit Args
static_assert( ntbs::cut<-2>(cut1) == "c" );
static_assert( ntbs::cut<-2,-1>(cut1) == "c" );
static_assert( sizeof cut1 == 2 );
static_assert( size(cut1) == 2 );

// test cut of multi-char string-literal
constexpr auto hullo = ntbs::cut("hello");
constexpr auto hullu = cut(hello);
static_assert( hullo == "hello" );
static_assert( hullu == "hello" );

// test lexicographic cmp
static_assert( ntbs::cmp("","") == 0 );
static_assert( ntbs::cmp("","a") < 0 );
static_assert( ntbs::cmp("a","a") == 0 );
static_assert( ntbs::cmp("a","aa") < 0 );
static_assert( ntbs::cmp("aa","a") > 0 );
static_assert( ntbs::cmp("a\0","a") == 0 ); // Embedded null, cmp equal
static_assert( ntbs::cat("a\0") != "a" );   // Embedded null, != unequal

// Little cats
static_assert( ntbs::cat('a','b') == "ab" );
static_assert( ntbs::cat('a',"b") == "ab" );
static_assert( ntbs::cat("a",'b') == "ab" );
static_assert( ntbs::cat("a","b") == "ab" );

// 'Big Cat' for compile time benchmarking
// (MSVC 19.2 warns that the K64 declaration will use 64K stack
//  and the K256 big cat fails)
//
//constexpr ntbs::array<65536> K64{};
//constexpr auto K256 = cat(K64, K64, K64, K64); // 1.1s
//static_assert(size(K256) == 262144 - 3);

/* Runtime tests */

// If ntbs.hpp is compiled with NTBS_NULL_CHECK = 1
// then null checks are enabled (follows NDEBUG by default)
// and these tests with non-null terminated arrays should throw
// yielding a compile error in constexpr evaluation
// or an exception at runtime.
//
auto test_cat_throw = []
{
    const char* thrown = nullptr;
    using char5 = char[5];
    try {
        ntbs::cat(char5{'t','h','r','o','w'});
    }
    catch (const char* str)
    {
        thrown = str;
    }
    return thrown;
};

auto test_cut_throw = []
{
    const char* thrown = nullptr;
    using char5 = char[5];
    try {
        ntbs::cut(char5{'t','h','r','o','w'});
    }
    catch (const char* str)
    {
        thrown = str;
    }
    return thrown;
};

#include <cassert>
#include <cstdio>

int main()
{
    auto cat_threw = test_cat_throw();
    auto cut_threw = test_cut_throw();

    if (cat_threw) puts(cat_threw);
    if (cut_threw) puts(cut_threw);

    using ntbs::cat;
    using ntbs::array;

    // cmp of non-null-terminated array fails in constexpr evaluation
    //static_assert( cmp(array<1>{'!'},array<1>{'!'}) == 0 ); // FAIL

    auto mut = cat("Hello");
    mut[5] = '!'; // write over null terminator

#if NTBS_NULL_CHECK
    try { cmp(mut,mut); }
    catch (const char* str) { puts(str); }
    assert( cat_threw );
    assert( cut_threw );
#else
    assert( ! cat_threw );
    assert( ! cut_threw );
#endif
}