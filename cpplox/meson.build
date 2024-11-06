cpplox_incl = include_directories('include')

cpplox_srcs = [
    'src/chunk.cpp'
]
cpplox_args = []
cpplox_link = []
cpplox_deps = []

cpplox_lib = library(
    'cpplox',
    cpplox_srcs,
    include_directories: [cpplox_incl],
    cpp_args: cpplox_args,
    dependencies: cpplox_deps,
)

cpplox_dep = declare_dependency(
    link_with: cpplox_lib,
    include_directories: cpplox_incl,
)