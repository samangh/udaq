#include "globals.h"


#ifdef __cplusplus
extern "C" {
#endif
#include <lv_prolog.h>

	typedef struct {
		int32_t dimSize;
		int32_t elt[1];
	} arr1D;
	typedef arr1D** arr1DH;
	typedef arr1D*** arr1DHP;


#include <lv_epilog.h>
#ifdef __cplusplus
}
#endif
