// Compiler agnostic way of enabling previously disabled warnings found in external libraries
#ifdef _ZE_COMPILER_FXC
// Cannot enable below warnings since the disabled parts of code still generate messages under FXC, probably bug
//#	pragma warning( default : 3078 3203 3556 4000 4714 )
#elif defined(_ZE_COMPILER_DXC)
#	pragma dxc diagnostic pop
#else
#	error Unknown shader compiler!
#endif