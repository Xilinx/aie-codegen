#ifndef XAIE_LOG_AIE_H
#define XAIE_LOG_AIE_H

#include <stdarg.h>
#include "xaiegbl_defs.h"
#include "xaiegbl_dynlink.h"

XAIE_AIG_EXPORT void XAie_Log(FILE *Fd, const char *prefix, const char *func, u32 line,
	const char *Format, ...);

#ifdef XAIE_ENABLE_INPUT_CHECK
#ifdef _ENABLE_IPU_LX6_
#include <printf.h>
#endif //_ENABLE_IPU_LX6_
#define XAIE_ERROR_RETURN(ERRCON, RET, ...) {	\
	if (ERRCON) {				\
		printf(__VA_ARGS__);		\
		return (RET);			\
	}					\
}
#else
#define XAIE_ERROR_RETURN(...)
#endif //XAIE_ENABLE_INPUT_CHECK

#define XAIE_ERROR_MSG(...)						\
	"[AIE ERROR] %s():%d: %s", __func__, __LINE__, __VA_ARGS__


#ifndef __SWIGINTERFACE__

#define XAIE_ERROR(...)							      \
	do {								      \
		XAie_Log((FILE*)(uintptr_t)stderr, "[AIE ERROR]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

#define XAIE_WARN(...)							      \
	do {								      \
		XAie_Log((FILE*)(uintptr_t)stderr, "[AIE WARNING]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

/**
* Note: Enable the definition of XAIE_FDEBUG macro to enable prints from aie-rt
*/
//#define XAIE_DEBUG 1

#ifdef XAIE_DEBUG

#define XAIE_DBG(...)							      \
	do {								      \
		XAie_Log(stdout, "[AIE DEBUG]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

#else

#define XAIE_DBG(DevInst, ...) {}

#endif /* XAIE_DEBUG */

#else

// redirect XAIE_ERROR to printf
#define XAIE_ERROR     printf

// no need for debug/warn printf so empty macro
#define XAIE_DBG(...)   {}
#define XAIE_WARN(...)  {}

#endif /* __SWIGINTERFACE__ */

#endif //XAIE_LOG_AIE_H

