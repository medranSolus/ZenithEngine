// Compiler agnostic way of enabling previously disabled warnings found in external libraries
#ifdef _ZE_COMPILER_FXC
// Cannot enable warnings since the disabled parts of code still generate messages under FXC, probably bug
#elif defined(_ZE_COMPILER_DXC)
#	pragma dxc diagnostic pop
#else
#	error Unknown shader compiler!
#endif