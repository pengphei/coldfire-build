#ifndef M68K_CF_CACHEFLUSH_H
#define M68K_CF_CACHEFLUSH_H

#ifdef CONFIG_M5445X
#include "cf_5445x_cacheflush.h"
#elif defined(CONFIG_M5441X)
#include "cf_5441x_cacheflush.h"
#elif defined(CONFIG_M547X_8X)
#include "cf_548x_cacheflush.h"
#endif

#endif /* M68K_CF_CACHEFLUSH_H */
