// Disable current warnings for default ones

#if defined(_MSC_VER)
#	pragma warning(push, 0)
#elif defined(__clang__)
#	pragma clang diagnostic push
#else
#	error Compiler not supported!
#endif