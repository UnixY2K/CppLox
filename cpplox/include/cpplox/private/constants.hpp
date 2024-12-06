#pragma once

#ifndef CPPLOX_CONSTANT_DEBUG_TRACE_INSTRUCTION
#define CPPLOX_CONSTANT_DEBUG_TRACE_INSTRUCTION false
#endif
#ifndef CPPLOX_CONSTANT_DEBUG_TRACE_STACK
#define CPPLOX_CONSTANT_DEBUG_TRACE_STACK false
#endif


namespace lox::constants {
	constexpr bool debug_trace_instruction = CPPLOX_CONSTANT_DEBUG_TRACE_INSTRUCTION;
	constexpr bool debug_trace_stack = CPPLOX_CONSTANT_DEBUG_TRACE_STACK;
};

