/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */
#ifndef _AMEBA_SOC_H_
#define _AMEBA_SOC_H_

#ifndef CONFIG_BUILD_ROM
/* ram headers */
#include "platform_autoconf.h"
#endif

/* rom headers */
#include "ameba.h"

#ifndef CONFIG_BUILD_ROM
/* ram headers */
//#include "ameba_ota.h"

//#include "os_wrapper.h"

#include "ameba_pmu.h"
//#include "freertos_pmu.h"

#endif

#endif //_AMEBA_SOC_H_
