// Enable previous warnings

#if defined(_MSC_VER)
#	pragma warning(pop)
#elif defined(__clang__)
#	pragma clang diagnostic pop
#else
#	error Compiler not supported!
#endif