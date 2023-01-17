#include "am_mcu_apollo.h"

#define AM_BSP_GPIO_VDDUSB33_SWITCH         103
#define AM_BSP_GPIO_VDDUSB0P9_SWITCH        101

static const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_VDDUSB33_SWITCH =
{
    .GP.cfg_b.uFuncSel             = AM_HAL_PIN_103_GPIO,
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
    .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.uSlewRate            = 0,
    .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.uNCE                 = 0,
    .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,
    .GP.cfg_b.uRsvd_0              = 0,
    .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE,
    .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.uRsvd_1              = 0,
};


static const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_VDDUSB0P9_SWITCH =
{
    .GP.cfg_b.uFuncSel             = AM_HAL_PIN_101_GPIO,
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
    .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.uSlewRate            = 0,
    .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.uNCE                 = 0,
    .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,
    .GP.cfg_b.uRsvd_0              = 0,
    .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE,
    .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.uRsvd_1              = 0,
};

//*****************************************************************************     
//      
//! @brief VDDUSB33 power switch.
//! 
//! @return None.
//      
//*****************************************************************************     
void    
am_bsp_external_vddusb33_switch(bool bEnable)
{   
    am_hal_gpio_write_type_e eGpioWrType;
    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;      
    am_hal_gpio_pinconfig(AM_BSP_GPIO_VDDUSB33_SWITCH, g_AM_BSP_GPIO_VDDUSB33_SWITCH);
    am_hal_gpio_state_write(AM_BSP_GPIO_VDDUSB33_SWITCH, eGpioWrType);
}   
        
//*****************************************************************************
//
//! @brief VDDUSB0P9 power switch.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_external_vddusb0p9_switch(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;
    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_VDDUSB0P9_SWITCH, g_AM_BSP_GPIO_VDDUSB0P9_SWITCH);
    am_hal_gpio_state_write(AM_BSP_GPIO_VDDUSB0P9_SWITCH, eGpioWrType);
}

