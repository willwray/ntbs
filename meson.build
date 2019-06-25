project('ntbs', 'cpp', default_options : 'cpp_std=c++17')

test('test ntbs cat and cut functions',
  executable('test_ntbs', 'test_ntbs.cpp')
)

ntbs_dep = declare_dependency(
  include_directories : include_directories('.')
)
