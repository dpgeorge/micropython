#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/builtin.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "shared/runtime/pyexec.h"

#if MICROPY_ENABLE_COMPILER
void do_str(const char *src, mp_parse_input_kind_t input_kind) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);
        mp_call_function_0(module_fun);
        nlr_pop();
    } else {
        // uncaught exception
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }
}
#endif

static char *stack_top;
#if MICROPY_ENABLE_GC
static char heap[MICROPY_HEAP_SIZE];
#endif

int main(int argc, char **argv) {
    int stack_dummy;
    stack_top = (char *)&stack_dummy;

    #if MICROPY_ENABLE_GC
    gc_init(heap, heap + sizeof(heap));
    #endif
    mp_init();
    #if MICROPY_ENABLE_COMPILER
    #if MICROPY_REPL_EVENT_DRIVEN
    pyexec_event_repl_init();
    for (;;) {
        int c = mp_hal_stdin_rx_chr();
        if (pyexec_event_repl_process_char(c)) {
            break;
        }
    }
    #else
    pyexec_friendly_repl();
    #endif
    // do_str("print('hello world!', list(x+1 for x in range(10)), end='eol\\n')", MP_PARSE_SINGLE_INPUT);
    // do_str("for i in range(10):\r\n  print(i)", MP_PARSE_FILE_INPUT);
    #else
    pyexec_frozen_module("frozentest.py", false);
    #endif
    mp_deinit();
    return 0;
}

#if MICROPY_ENABLE_GC
void gc_collect(void) {
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
    void *dummy;
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    gc_collect_end();
    gc_dump_info(&mp_plat_print);
}
#endif

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    mp_raise_OSError(MP_ENOENT);
}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

void nlr_jump_fail(void *val) {
    while (1) {
        ;
    }
}

void NORETURN __fatal_error(const char *msg) {
    while (1) {
        ;
    }
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

#if 0
#if MICROPY_MIN_USE_CORTEX_CPU

// this is a minimal IRQ and reset framework for any Cortex-M CPU

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;

void Reset_Handler(void) __attribute__((naked));
void Reset_Handler(void) {
    // set stack pointer
    __asm volatile ("ldr sp, =_estack");
    // copy .data section from flash to RAM
    for (uint32_t *src = &_sidata, *dest = &_sdata; dest < &_edata;) {
        *dest++ = *src++;
    }
    // zero out .bss section
    for (uint32_t *dest = &_sbss; dest < &_ebss;) {
        *dest++ = 0;
    }
    // jump to board initialisation
    void _start(void);
    _start();
}

void Default_Handler(void) {
    for (;;) {
    }
}

const uint32_t isr_vector[] __attribute__((section(".isr_vector"))) = {
    (uint32_t)&_estack,
    (uint32_t)&Reset_Handler,
    (uint32_t)&Default_Handler, // NMI_Handler
    (uint32_t)&Default_Handler, // HardFault_Handler
    (uint32_t)&Default_Handler, // MemManage_Handler
    (uint32_t)&Default_Handler, // BusFault_Handler
    (uint32_t)&Default_Handler, // UsageFault_Handler
    0,
    0,
    0,
    0,
    (uint32_t)&Default_Handler, // SVC_Handler
    (uint32_t)&Default_Handler, // DebugMon_Handler
    0,
    (uint32_t)&Default_Handler, // PendSV_Handler
    (uint32_t)&Default_Handler, // SysTick_Handler
};

void _start(void) {
    // when we get here: stack is initialised, bss is clear, data is copied

    // SCB->CCR: enable 8-byte stack alignment for IRQ handlers, in accord with EABI
    *((volatile uint32_t *)0xe000ed14) |= 1 << 9;

    // initialise the cpu and peripherals
    #if MICROPY_MIN_USE_STM32_MCU
    void stm32_init(void);
    stm32_init();
    #endif

    // now that we have a basic system up and running we can call main
    main(0, NULL);

    // we must not return
    for (;;) {
    }
}

#endif

#if MICROPY_MIN_USE_STM32_MCU

// this is minimal set-up code for an STM32 MCU

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    uint32_t _1[8];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    uint32_t _2;
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
} periph_rcc_t;

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint16_t BSRRL;
    volatile uint16_t BSRRH;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} periph_gpio_t;

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
} periph_uart_t;

#define USART1 ((periph_uart_t *)0x40011000)
#define GPIOA  ((periph_gpio_t *)0x40020000)
#define GPIOB  ((periph_gpio_t *)0x40020400)
#define RCC    ((periph_rcc_t *)0x40023800)

// simple GPIO interface
#define GPIO_MODE_IN (0)
#define GPIO_MODE_OUT (1)
#define GPIO_MODE_ALT (2)
#define GPIO_PULL_NONE (0)
#define GPIO_PULL_UP (0)
#define GPIO_PULL_DOWN (1)
void gpio_init(periph_gpio_t *gpio, int pin, int mode, int pull, int alt) {
    gpio->MODER = (gpio->MODER & ~(3 << (2 * pin))) | (mode << (2 * pin));
    // OTYPER is left as default push-pull
    // OSPEEDR is left as default low speed
    gpio->PUPDR = (gpio->PUPDR & ~(3 << (2 * pin))) | (pull << (2 * pin));
    gpio->AFR[pin >> 3] = (gpio->AFR[pin >> 3] & ~(15 << (4 * (pin & 7)))) | (alt << (4 * (pin & 7)));
}
#define gpio_get(gpio, pin) ((gpio->IDR >> (pin)) & 1)
#define gpio_set(gpio, pin, value) do { gpio->ODR = (gpio->ODR & ~(1 << (pin))) | (value << pin); } while (0)
#define gpio_low(gpio, pin) do { gpio->BSRRH = (1 << (pin)); } while (0)
#define gpio_high(gpio, pin) do { gpio->BSRRL = (1 << (pin)); } while (0)

void stm32_init(void) {
    // basic MCU config
    RCC->CR |= (uint32_t)0x00000001; // set HSION
    RCC->CFGR = 0x00000000; // reset all
    RCC->CR &= (uint32_t)0xfef6ffff; // reset HSEON, CSSON, PLLON
    RCC->PLLCFGR = 0x24003010; // reset PLLCFGR
    RCC->CR &= (uint32_t)0xfffbffff; // reset HSEBYP
    RCC->CIR = 0x00000000; // disable IRQs

    // leave the clock as-is (internal 16MHz)

    // enable GPIO clocks
    RCC->AHB1ENR |= 0x00000003; // GPIOAEN, GPIOBEN

    // turn on an LED! (on pyboard it's the red one)
    gpio_init(GPIOA, 13, GPIO_MODE_OUT, GPIO_PULL_NONE, 0);
    gpio_high(GPIOA, 13);

    // enable UART1 at 9600 baud (TX=B6, RX=B7)
    gpio_init(GPIOB, 6, GPIO_MODE_ALT, GPIO_PULL_NONE, 7);
    gpio_init(GPIOB, 7, GPIO_MODE_ALT, GPIO_PULL_NONE, 7);
    RCC->APB2ENR |= 0x00000010; // USART1EN
    USART1->BRR = (104 << 4) | 3; // 16MHz/(16*104.1875) = 9598 baud
    USART1->CR1 = 0x0000200c; // USART enable, tx enable, rx enable
}

#endif
#endif

#include "am_mcu_apollo.h"

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;

void Reset_Handler(void) __attribute__((naked));

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
    main(0, NULL);

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

    // Set the baud rate.
    config_baudrate(uart_id, baudrate);

    // Set the flow control options
    UARTn(uart_id)->CR_b.RTSEN = 0;
    UARTn(uart_id)->CR_b.CTSEN = 0;
    UARTn(uart_id)->CR |= AM_HAL_UART_FLOW_CTRL_NONE;

    // Set the data format.
    UARTn(uart_id)->LCRH_b.BRK  = 0;
    UARTn(uart_id)->LCRH_b.PEN  = 0; // parity enable?
    UARTn(uart_id)->LCRH_b.EPS  = 0; // even parity?
    UARTn(uart_id)->LCRH_b.STP2 = AM_HAL_UART_ONE_STOP_BIT;
    UARTn(uart_id)->LCRH_b.FEN  = 1;
    UARTn(uart_id)->LCRH_b.WLEN = AM_HAL_UART_DATA_BITS_8;
    UARTn(uart_id)->LCRH_b.SPS  = 0;

    // Set the FIFO levels.
    UARTn(uart_id)->IFLS_b.TXIFLSEL = AM_HAL_UART_FIFO_LEVEL_16;
    UARTn(uart_id)->IFLS_b.RXIFLSEL = AM_HAL_UART_FIFO_LEVEL_16;

    // Enable the UART, RX, and TX.
    UARTn(uart_id)->CR_b.UARTEN = 1;
    UARTn(uart_id)->CR_b.RXE = 1;
    UARTn(uart_id)->CR_b.TXE = 1;
}
