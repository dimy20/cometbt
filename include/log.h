#include <iostream>
#define ALLOC_ERROR_STR "Failed to allocate"

#define COMET_LOG_ERROR(o, s) o << "Error at " << __FUNCTION__ << ":" << \
								__LINE__ << ", " << \
								       s << std::endl;
#define COMET_ASSERT_ALLOC(ptr) do { \
	if (ptr == NULL) { \
		COMET_LOG_ERROR(std::cerr, ALLOC_ERROR_STR); \
		exit(EXIT_FAILURE); \
	} \
}while(0)


