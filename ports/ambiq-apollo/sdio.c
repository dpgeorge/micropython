#include <stdio.h>
#include "py/mphal.h"
#include "pins.h"
#include "sdio.h"

#define SDIO_PERIPHERAL_NUM (0)
#define SDIO_MULTIPLEXER_VALUE (1)

#define AMAP_PIN_PINCFG_SDIO                                                \
    {                                                                       \
        .GP.cfg_b.uFuncSel             = 0, /* to fill in */                \
        .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,        \
        .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,    \
        .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,       \
        .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,    \
        .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_1P0X, \
        .GP.cfg_b.uSlewRate            = 0,                                 \
        .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE,       \
        .GP.cfg_b.uNCE                 = 0,                                 \
        .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,   \
        .GP.cfg_b.uRsvd_0              = 0,                                 \
        .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE,      \
        .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE,      \
        .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE,      \
        .GP.cfg_b.uRsvd_1              = 0,                                 \
    }


static am_hal_card_host_t *sdio_host;

void sdio_init(void) {
    printf("sdio_init\n");
    mp_hal_pin_output(amap_pin_SDIO_MULTIPLEXER_SELECT);
    mp_hal_pin_write(amap_pin_SDIO_MULTIPLEXER_SELECT, SDIO_MULTIPLEXER_VALUE);

    am_hal_gpio_pincfg_t cfg = AMAP_PIN_PINCFG_SDIO;
    cfg.GP.cfg_b.uFuncSel = AM_HAL_PIN_83_SDIF_CMD;
    am_hal_gpio_pinconfig(amap_pin_SDIO2_CMD, cfg);
    cfg.GP.cfg_b.uFuncSel = AM_HAL_PIN_84_SDIF_DAT0;
    am_hal_gpio_pinconfig(amap_pin_SDIO2_D0, cfg);
    cfg.GP.cfg_b.uFuncSel = AM_HAL_PIN_85_SDIF_DAT1;
    am_hal_gpio_pinconfig(amap_pin_SDIO2_D1, cfg);
    cfg.GP.cfg_b.uFuncSel = AM_HAL_PIN_86_SDIF_DAT2;
    am_hal_gpio_pinconfig(amap_pin_SDIO2_D2, cfg);
    cfg.GP.cfg_b.uFuncSel = AM_HAL_PIN_87_SDIF_DAT3;
    am_hal_gpio_pinconfig(amap_pin_SDIO2_D3, cfg);
    cfg.GP.cfg_b.uFuncSel = AM_HAL_PIN_88_SDIF_CLKOUT;
    am_hal_gpio_pinconfig(amap_pin_SDIO2_CLK, cfg);

    sdio_host = am_hal_get_card_host(SDIO_PERIPHERAL_NUM, true);

    int res = sdio_host->ops->set_bus_voltage(sdio_host->pHandle, AM_HAL_HOST_BUS_VOLTAGE_1_8);
    printf("set voltage: %d\n", res);
    res = sdio_host->ops->set_bus_width(sdio_host->pHandle, AM_HAL_HOST_BUS_WIDTH_4);
    printf("set width: %d\n", res);

    am_hal_card_t eMMCard;
    for (int retry = 0; retry < 10; ++retry) {
        if (am_hal_card_host_find_card(sdio_host, &eMMCard) != AM_HAL_STATUS_SUCCESS) {
            printf("can't find card\n");
        } else {
            printf("found card\n");
            break;
        }
    }

    res = sdio_host->ops->set_bus_voltage(sdio_host->pHandle, AM_HAL_HOST_BUS_VOLTAGE_1_8);
    printf("set voltage: %d\n", res);
    res = sdio_host->ops->set_bus_width(sdio_host->pHandle, AM_HAL_HOST_BUS_WIDTH_4);
    printf("set width: %d\n", res);
}

void sdio_deinit(void) {
    printf("sdio_deinit\n");
}

void sdio_reenable(void) {
    printf("sdio_reenable\n");
}

void sdio_enable_irq(bool enable) {
    printf("sdio_enable_irq\n");
}

void sdio_enable_high_speed_4bit(void) {
    printf("sdio_enable_high_speed_4bit\n");
}

int sdio_transfer(uint32_t cmd, uint32_t arg, uint32_t *resp) {
    mp_hal_pin_write(amap_pin_SDIO_MULTIPLEXER_SELECT, SDIO_MULTIPLEXER_VALUE);
    printf("sdio_transfer %d %d\n", (int)cmd, (int)arg);

    am_hal_card_cmd_t cmd_struct;

    memset(&cmd_struct, 0, sizeof(cmd_struct));
    cmd_struct.ui8Idx   = cmd;
    cmd_struct.ui32Arg  = arg;
    cmd_struct.ui32RespType = MMC_RSP_R1;

    int res = sdio_host->ops->execute_cmd(sdio_host->pHandle, &cmd_struct, NULL);

    printf("result res=%d %d 0x%08x\n", res, cmd_struct.eError, (int)cmd_struct.ui32Resp[0]);

    if (res == AM_HAL_STATUS_SUCCESS) {
        return 0; // success
    } else {
        return -1; // failure
    }
}

int sdio_transfer_cmd53(bool write, uint32_t block_size, uint32_t arg, size_t len, uint8_t *buf) {
    mp_hal_pin_write(amap_pin_SDIO_MULTIPLEXER_SELECT, SDIO_MULTIPLEXER_VALUE);
    printf("sdio_transfer_cmd53\n");
    return -1; // failure
}
