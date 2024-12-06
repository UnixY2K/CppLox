cpplox_incl = include_directories('include')

cpplox_srcs = [
    'src/chunk.cpp',
    'src/compiler.cpp',
    'src/debug.cpp',
    'src/obj.cpp',
    'src/scanner.cpp',
    'src/terminal.cpp',
    'src/value.cpp',
    'src/vm.cpp',
]
cpplox_args = []
cpplox_link = []
cpplox_deps = []


cpplox_debug_trace_instruction = get_option('DEBUG_trace_instruction')
cpplox_debug_trace_stack = get_option('DEBUG_trace_stack')

if cpplox_debug_trace_instruction
	cpplox_args += ['-DCPPLOX_CONSTANT_DEBUG_TRACE_INSTRUCTION=' + cpplox_debug_trace_instruction.to_string()]
endif

if cpplox_debug_trace_stack
	cpplox_args += ['-DCPPLOX_CONSTANT_DEBUG_TRACE_STACK=' + cpplox_debug_trace_stack.to_string()]
endif

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
