#include <iostream>
#define ALLOC_ERROR_STR "Failed to allocate"

#define COMET_LOG_ERROR(o, s) o << "Error at " << __FUNCTION__ << ":" << \
								__LINE__ << ", " << \
								       s << std::endl;
#define COMET_HANDLE_ALLOC(ptr) if (ptr == NULL) COMET_LOG_ERROR(std::cerr, ALLOC_ERROR_STR)
