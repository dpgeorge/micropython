/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <string.h>
#include "am_mcu_apollo.h"

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;

void Reset_Handler(void) __attribute__((naked));
void bare_main(void);

static void amap_init(void);

// Very simple ARM vector table.
// must be 0x200 bytes in size
const uint32_t isr_vector[] __attribute__((section(".isr_vector"))) = {
    (uint32_t)&_estack,
    (uint32_t)&Reset_Handler,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 14 entries
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16 entries
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16 entries
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16 entries
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16 entries
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16 entries
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16 entries
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16 entries
};

// The CPU runs this function after a reset.
void Reset_Handler(void) {
    // Set stack pointer.
    __asm volatile ("ldr sp, =_estack");

    // Copy .data section from flash to RAM.
    memcpy(&_sdata, &_sidata, (char *)&_edata - (char *)&_sdata);

    // Zero out .bss section.
    memset(&_sbss, 0, (char *)&_ebss - (char *)&_sbss);

    // SCB->CCR: enable 8-byte stack alignment for IRQ handlers, in accord with EABI.
    //*((volatile uint32_t *)0xe000ed14) |= 1 << 9;

    // Initialise the cpu and peripherals.
    amap_init();

    // Now that there is a basic system up and running, call the main application code.
    bare_main();

    // This function must not return.
    for (;;) {
    }
}

void delay(uint32_t n) {
    for (uint32_t i = 0; i < n; ++i) {
        for (uint32_t i = 0; i < 10000; ++i) {
            __NOP();
        }
    }
}

am_hal_gpio_pincfg_t g_AM_BSP_GPIO_COM_UART_TX =
{
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

am_hal_gpio_pincfg_t g_AM_BSP_GPIO_COM_UART_RX =
{
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

#define BAUDCLK     (16) // Number of UART clocks needed per bit.

static void config_baudrate(uint32_t uart_id, uint32_t ui32DesiredBaudrate) {
    uint64_t ui64FractionDivisorLong;
    uint64_t ui64IntermediateLong;
    uint32_t ui32IntegerDivisor;
    uint32_t ui32FractionDivisor;
    uint32_t ui32BaudClk;
    uint32_t ui32UartClkFreq;

    //
    // Check that the baudrate is in range.
    //
    switch ( UARTn(uart_id)->CR_b.CLKSEL )
    {
        case UART0_CR_CLKSEL_48MHZ:
            ui32UartClkFreq = 48000000;
            break;

        case UART0_CR_CLKSEL_24MHZ:
            ui32UartClkFreq = 24000000;
            break;

        case UART0_CR_CLKSEL_12MHZ:
            ui32UartClkFreq = 12000000;
            break;

        case UART0_CR_CLKSEL_6MHZ:
            ui32UartClkFreq = 6000000;
            break;

        case UART0_CR_CLKSEL_3MHZ:
            ui32UartClkFreq = 3000000;
            break;

        default:
            return;
            //*pui32ActualBaud = 0;
            //return AM_HAL_UART_STATUS_CLOCK_NOT_CONFIGURED;
    }

    //
    // Calculate register values.
    //
    ui32BaudClk = BAUDCLK * ui32DesiredBaudrate;
    ui32IntegerDivisor = (uint32_t)(ui32UartClkFreq / ui32BaudClk);
    ui64IntermediateLong = (ui32UartClkFreq * 64) / ui32BaudClk; // Q58.6
    ui64FractionDivisorLong = ui64IntermediateLong - (ui32IntegerDivisor * 64); // Q58.6
    ui32FractionDivisor = (uint32_t)ui64FractionDivisorLong;

    //
    // Check the result.
    //
    if (ui32IntegerDivisor == 0)
    {
        // baudrate not possible
        return;
    }

    //
    // Write the UART regs.
    //
    UARTn(uart_id)->IBRD = ui32IntegerDivisor;
    UARTn(uart_id)->FBRD = ui32FractionDivisor;

    //
    // Return the actual baud rate.
    //
    //*pui32ActualBaud = (ui32UartClkFreq / ((BAUDCLK * ui32IntegerDivisor) + ui32FractionDivisor / 4));
}

// Set up the STM32 MCU.
static void amap_init(void) {
    // GP90, GP97, output
    am_hal_gpio_pincfg_t cfg = AM_HAL_GPIO_PINCFG_OUTPUT;
    GPIO->PADKEY = GPIO_PADKEY_PADKEY_Key;
    GPIO->PINCFG47 = g_AM_BSP_GPIO_COM_UART_RX.GP.cfg;
    GPIO->PINCFG60 = g_AM_BSP_GPIO_COM_UART_TX.GP.cfg;
    GPIO->PINCFG90 = cfg.GP.cfg;
    GPIO->PINCFG97 = cfg.GP.cfg;
    GPIO->PADKEY = 0;

    //am_hal_uart_initialize(0, &g_sCOMUART);
    //am_hal_uart_power_control(g_sCOMUART, AM_HAL_SYSCTRL_WAKE, false);
    uint32_t uart_id = 0;
    uint32_t baudrate = 115200;
        am_hal_pwrctrl_periph_e eUARTPowerModule = ((am_hal_pwrctrl_periph_e)
                                                (AM_HAL_PWRCTRL_PERIPH_UART0 +
                                                 uart_id));
    am_hal_pwrctrl_periph_enable(eUARTPowerModule);

    //am_hal_uart_configure(g_sCOMUART, &g_sBspUartConfig);
    UARTn(uart_id)->CR = 0;
        //
    // Start by enabling the clocks, which needs to happen in a critical
    // section.
    //
    // AM_CRITICAL_BEGIN
    UARTn(uart_id)->CR_b.CLKEN = 1;
    if ( baudrate > 1500000 ) {
        UARTn(uart_id)->CR_b.CLKSEL = UART0_CR_CLKSEL_48MHZ;
    } else {
        UARTn(uart_id)->CR_b.CLKSEL = UART0_CR_CLKSEL_24MHZ;
    }
    // AM_CRITICAL_END

    //
    // Disable the UART.
    //
    // AM_CRITICAL_BEGIN
    UARTn(uart_id)->CR_b.UARTEN = 0;
    UARTn(uart_id)->CR_b.RXE = 0;
    UARTn(uart_id)->CR_b.TXE = 0;
    // AM_CRITICAL_END

    //
    // Set the baud rate.
    //
    config_baudrate(uart_id, baudrate);

    //
    // Set the flow control options
    //
    // AM_CRITICAL_BEGIN
    UARTn(uart_id)->CR_b.RTSEN = 0;
    UARTn(uart_id)->CR_b.CTSEN = 0;
    UARTn(uart_id)->CR |= AM_HAL_UART_FLOW_CTRL_NONE;
    // AM_CRITICAL_END

    //
    // Set the data format.
    //
    // AM_CRITICAL_BEGIN
    UARTn(uart_id)->LCRH_b.BRK  = 0;
    UARTn(uart_id)->LCRH_b.PEN  = 0; // parity enable?
    UARTn(uart_id)->LCRH_b.EPS  = 0; // even parity?
    UARTn(uart_id)->LCRH_b.STP2 = AM_HAL_UART_ONE_STOP_BIT;
    UARTn(uart_id)->LCRH_b.FEN  = 1;
    UARTn(uart_id)->LCRH_b.WLEN = AM_HAL_UART_DATA_BITS_8;
    UARTn(uart_id)->LCRH_b.SPS  = 0;
    // AM_CRITICAL_END

    //
    // Set the FIFO levels.
    //
    // AM_CRITICAL_BEGIN
    UARTn(uart_id)->IFLS_b.TXIFLSEL = AM_HAL_UART_FIFO_LEVEL_16;
    UARTn(uart_id)->IFLS_b.RXIFLSEL = AM_HAL_UART_FIFO_LEVEL_16;
    // AM_CRITICAL_END

    //
    // Enable the UART, RX, and TX.
    //
    // AM_CRITICAL_BEGIN
    UARTn(uart_id)->CR_b.UARTEN = 1;
    UARTn(uart_id)->CR_b.RXE = 1;
    UARTn(uart_id)->CR_b.TXE = 1;
    // AM_CRITICAL_END

    UARTn(uart_id)->DR = 'M';
    UARTn(uart_id)->DR = 'P';

    /*
    for (;;) {
        am_hal_gpio_output_clear(90);
        am_hal_gpio_output_set(97);
        delay(1000);
        am_hal_gpio_output_set(90);
        am_hal_gpio_output_clear(97);
        delay(1000);
    }
    */
}

// Write a character out to the UART.
static inline void uart_write_char(int c) {
    /*
    // Wait for TXE, then write the character.
    while ((UART4->SR & (1 << 7)) == 0) {
    }
    UART4->DR = c;
    */
    uint32_t uart_id = 0;
    while (UARTn(uart_id)->FR_b.TXFF) {
    }
    UARTn(uart_id)->DR = c;
}

// Send string of given length to stdout, converting \n to \r\n.
void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    while (len--) {
        if (*str == '\n') {
            uart_write_char('\r');
        }
        uart_write_char(*str++);
    }
}
