cpplox_cli_incl = include_directories('include')

cpplox_cli_srcs = [
    'src/repl.cpp',
    'src/source.cpp',
]
cpplox_cli_args = []
cpplox_cli_link = []
cpplox_cli_deps = [cpplox_dep]

executable(
    'lox',
    cpplox_cli_srcs,
    include_directories: [cpplox_cli_incl],
    cpp_args: cpplox_cli_args,
    dependencies: cpplox_cli_deps,
)
