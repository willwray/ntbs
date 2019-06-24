#include "ntbs.hpp"
using namespace ltl::ntbs;

constexpr auto hello_world = cat(cat<',',' '>("Hello","world"),'!');
constexpr auto hello_comma = cut<0,6>(hello_world);
constexpr auto world_exclaim = cut<-7>(hello_world);

#include <cstdio>
int main() { puts( cat<' '>(hello_comma, world_exclaim) ); }

static_assert( hello_world == "Hello, world!" );
static_assert( hello_comma == "Hello," );
static_assert( world_exclaim == "world!" );

static_assert( sizeof(hello_world) == 14 );
static_assert( sizeof(hello_comma) == 7 );
static_assert( sizeof(world_exclaim) == 7 );

static_assert( hello_world == cat<' '>(hello_comma, world_exclaim) );
