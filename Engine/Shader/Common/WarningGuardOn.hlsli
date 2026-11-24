// Compiler agnostic way of disabling warnings found in external libraries
#ifdef _ZE_COMPILER_FXC
#	pragma warning( disable : 3078 3092 3203 3550 3556 3571 4000 4008 4714 )
#elif defined(_ZE_COMPILER_DXC)
#	pragma dxc diagnostic push
#	pragma dxc diagnostic ignored "-Wfor-redefinition"
#	pragma dxc diagnostic ignored "-Wambig-lit-shift"
#	pragma dxc diagnostic ignored "-Wconversion"
#else
#	error Unknown shader compiler!
#endif