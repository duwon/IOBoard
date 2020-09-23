#ifndef DP_H__
#define DP_H__ 1

#include "main.h"

#define DP_ADDRESS  0x28U
#define DP_OUTPUT_MAX  14775 /*!> 14745 counts (90% of 2^14 counts or 0x3999) */
#define DP_OUTPUT_MIN  1638 /*!> 1638 counts (10% of 214 counts or 0x0666) */
#define DP_FORCE_RATED 2 /*!> maximum value of force range (N, lb, g, or kg) */

float DP_Read(void);

#endif /* DP_H__ */
