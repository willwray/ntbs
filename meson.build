project('ntbs', 'cpp', default_options : 'cpp_std=c++17')

test('test cat and cut',
  executable('test_ntbs', 'test_ntbs.cpp')
)

cat_dep = declare_dependency(
  include_directories : include_directories('.')
)
