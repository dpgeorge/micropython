#include <string.h>
#include "am_mcu_apollo.h"

#define NVIC_PRIORITYGROUP_4 0x00000003 // 4 bits for pre-emption priority

extern uint32_t _sstack, _estack, _sidata, _sdata, _edata, _sbss, _ebss;

void amap_main(void);
void Reset_Handler(void) __attribute__((naked));
void SysTick_Handler(void);
void UART0_IRQHandler(void);

static void amap_init(void);

// Apollo4+ vector table.  Must be 0x200 bytes in size.
const uint32_t isr_vector[128] __attribute__((section(".isr_vector"))) = {
    (uint32_t)&_estack,
    (uint32_t)Reset_Handler,                // The reset handler
    (uint32_t)0,                            // The NMI handler
    (uint32_t)0,                            // The hard fault handler
    (uint32_t)0,                            // The MemManage_Handler
    (uint32_t)0,                            // The BusFault_Handler
    (uint32_t)0,                            // The UsageFault_Handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    (uint32_t)0,                            // SVCall handler
    (uint32_t)0,                            // Debug monitor handler
    0,                                      // Reserved
    (uint32_t)0,                            // The PendSV handler
    (uint32_t)SysTick_Handler,              // The SysTick handler

    0,                                      //  0: Brownout (rstgen)
    0,                                      //  1: Watchdog (WDT)
    0,                                      //  2: RTC
    0,                                      //  3: Voltage Comparator
    0,                                      //  4: I/O Slave general
    0,                                      //  5: I/O Slave access
    0,                                      //  6: I/O Master 0
    0,                                      //  7: I/O Master 1
    0,                                      //  8: I/O Master 2
    0,                                      //  9: I/O Master 3
    0,                                      // 10: I/O Master 4
    0,                                      // 11: I/O Master 5
    0,                                      // 12: I/O Master 6 (I3C/I2C/SPI)
    0,                                      // 13: I/O Master 7 (I3C/I2C/SPI)
    0,                                      // 14: OR of all timerX interrupts
    (uint32_t)UART0_IRQHandler,             // 15: UART0

    0,                                      // 16: UART1
    0,                                      // 17: UART2
    0,                                      // 18: UART3
    0,                                      // 19: ADC
    0,                                      // 20: MSPI0
    0,                                      // 21: MSPI1
    0,                                      // 22: MSPI2
    0,                                      // 23: ClkGen
    0,                                      // 24: Crypto Secure
    0,                                      // 25: Reserved
    0,                                      // 26: SDIO
    0,                                      // 27: USB
    0,                                      // 28: GPU
    0,                                      // 29: DISP
    0,                                      // 30: DSI
    0,                                      // 31: Reserved

    0,                                      // 32: System Timer Compare0
    0,                                      // 33: System Timer Compare1
    0,                                      // 34: System Timer Compare2
    0,                                      // 35: System Timer Compare3
    0,                                      // 36: System Timer Compare4
    0,                                      // 37: System Timer Compare5
    0,                                      // 38: System Timer Compare6
    0,                                      // 39: System Timer Compare7
    0,                                      // 40: System Timer Cap Overflow
    0,                                      // 41: Reserved
    0,                                      // 42: Audio ADC
    0,                                      // 43: Reserved
    0,                                      // 44: I2S0
    0,                                      // 45: I2S1
    0,                                      // 46: I2S2
    0,                                      // 47: I2S3

    0,                                      // 48: PDM0
    0,                                      // 49: PDM1
    0,                                      // 50: PDM2
    0,                                      // 51: PDM3
    0,                                      // 52: Reserved
    0,                                      // 53: Reserved
    0,                                      // 54: Reserved
    0,                                      // 55: Reserved
    0,                                      // 56: GPIO N0 pins  0-31
    0,                                      // 57: GPIO N0 pins 32-63
    0,                                      // 58: GPIO N0 pins 64-95
    0,                                      // 59: GPIO N0 pins 96-104, virtual 105-127
    0,                                      // 60: GPIO N1 pins  0-31
    0,                                      // 61: GPIO N1 pins 32-63
    0,                                      // 62: GPIO N1 pins 64-95
    0,                                      // 63: GPIO N1 pins 96-104, virtual 105-127

    0,                                      // 64: Reserved
    0,                                      // 65: Reserved
    0,                                      // 66: Reserved
    0,                                      // 67: timer0
    0,                                      // 68: timer1
    0,                                      // 69: timer2
    0,                                      // 70: timer3
    0,                                      // 71: timer4
    0,                                      // 72: timer5
    0,                                      // 73: timer6
    0,                                      // 74: timer7
    0,                                      // 75: timer8
    0,                                      // 76: timer9
    0,                                      // 77: timer10
    0,                                      // 78: timer11
    0,                                      // 79: timer12

    0,                                      // 80: timer13
    0,                                      // 81: timer14
    0,                                      // 82: timer15
    0,                                      // 83: CPU cache

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 12 entries
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16 entries
};

// The CPU runs this function after a reset.
void Reset_Handler(void) {
    // Set stack pointer.
    __asm volatile ("ldr sp, =_estack");

    SCB->VTOR = (uint32_t)&isr_vector[0];
    SCB->CPACR |= 0xf << 20; // enable FPU (set CP10 and CP11 to full access)

    // Copy .data section from flash to RAM.
    memcpy(&_sdata, &_sidata, (char *)&_edata - (char *)&_sdata);

    // Zero out .bss section.
    memset(&_sbss, 0, (char *)&_ebss - (char *)&_sbss);

    // SCB->CCR: enable 8-byte stack alignment for IRQ handlers, in accord with EABI.
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;
    //*((volatile uint32_t *)0xe000ed14) |= 1 << 9;

    // Initialise the cpu and peripherals.
    amap_init();

    // Now that there is a basic system up and running, call the main application code.
    amap_main();

    // This function must not return.
    for (;;) {
    }
}

uint32_t ticks_ms;
void SysTick_Handler(void) {
    ++ticks_ms;

    // Read the systick control regster. This has the side effect of clearing
    // the COUNTFLAG bit, which makes the logic in mp_hal_ticks_us
    // work properly.
    SysTick->CTRL;
}

am_hal_gpio_pincfg_t g_AM_BSP_GPIO_COM_UART_TX = {
    .GP.cfg_b.uFuncSel             = AM_HAL_PIN_60_UART0TX,
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
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

am_hal_gpio_pincfg_t g_AM_BSP_GPIO_COM_UART_RX = {
    .GP.cfg_b.uFuncSel             = AM_HAL_PIN_47_UART0RX,
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
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

const am_hal_uart_config_t g_sUartConfig = {
    // Standard UART settings: 115200-8-N-1
    .ui32BaudRate = 115200,
    .eDataBits = AM_HAL_UART_DATA_BITS_8,
    .eParity = AM_HAL_UART_PARITY_NONE,
    .eStopBits = AM_HAL_UART_ONE_STOP_BIT,
    .eFlowControl = AM_HAL_UART_FLOW_CTRL_NONE,

    // Set TX and RX FIFOs to interrupt at half-full.
    .eTXFifoLevel = AM_HAL_UART_FIFO_LEVEL_16,
    .eRXFifoLevel = AM_HAL_UART_FIFO_LEVEL_16,
};

static void amap_init(void) {
    //NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_SIMOBUCK_INIT, 0);
    am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE);

    // Run SysTick IRQ at 1kHz.
    SysTick_Config(2*96000000 / 1000);
    NVIC_SetPriority(SysTick_IRQn, AM_IRQ_PRIORITY_DEFAULT);

    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_START, 0);
    am_hal_rtc_config_t rtc_config;
    rtc_config.eOscillator = AM_HAL_RTC_OSC_XT;
    rtc_config.b12Hour = false;
    am_hal_rtc_config(&rtc_config);
    am_hal_rtc_osc_enable();

    am_hal_rtc_time_t t;
    t.ui32CenturyEnable = 0;
    t.ui32Weekday = 2;
    t.ui32Year = 22;
    t.ui32Month = 4;
    t.ui32DayOfMonth = 26;
    t.ui32Hour = 9;
    t.ui32Minute = 0;
    t.ui32Second = 0;
    t.ui32Hundredths = 0;
    am_hal_rtc_time_set(&t);

    // Initialise UART0.
    void *phUART = NULL;
    am_hal_uart_initialize(0, &phUART);
    am_hal_uart_power_control(phUART, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_uart_configure(phUART, &g_sUartConfig);

    // Enable the UART0 pins.
    am_hal_gpio_pinconfig(60, g_AM_BSP_GPIO_COM_UART_TX);
    am_hal_gpio_pinconfig(47, g_AM_BSP_GPIO_COM_UART_RX);

    // Enable UART0 interrupts.
    NVIC_SetPriority(UART0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(UART0_IRQn);
    UARTn(0)->IER |= UART0_IER_RTIM_Msk | UART0_IER_RXIM_Msk;

    am_hal_interrupt_master_enable();
}
