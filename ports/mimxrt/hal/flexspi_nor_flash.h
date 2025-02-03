/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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
#ifndef MICROPY_INCLUDED_MIMXRT_HAL_FLEXSPI_NOR_FLASH_H
#define MICROPY_INCLUDED_MIMXRT_HAL_FLEXSPI_NOR_FLASH_H

#include "fsl_flexspi.h"
#include "mpconfigboard.h"
#include BOARD_FLASH_CONFIG_HEADER_H

#if defined MICROPY_HW_FLASH_INTERNAL
#define BOARD_FLEX_SPI FLEXSPI2
#define BOARD_FLEX_SPI_ADDR_BASE FlexSPI2_AMBA_BASE
#elif defined MIMXRT117x_SERIES
#define BOARD_FLEX_SPI FLEXSPI1
#define BOARD_FLEX_SPI_ADDR_BASE FlexSPI1_AMBA_BASE
#else
#define BOARD_FLEX_SPI FLEXSPI
#define BOARD_FLEX_SPI_ADDR_BASE FlexSPI_AMBA_BASE
#endif

// Defined in boards flash_config.c
extern flexspi_nor_config_t qspiflash_config;

status_t flexspi_nor_get_vendor_id(FLEXSPI_Type *base, uint8_t *vendorId);
status_t flexspi_nor_init(void);
void flexspi_nor_update_lut(void);
status_t flexspi_nor_enable_quad_mode(FLEXSPI_Type *base);
status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address);
status_t flexspi_nor_flash_erase_block(FLEXSPI_Type *base, uint32_t address);
status_t flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t address, const uint32_t *src, uint32_t size);

#endif // MICROPY_INCLUDED_MIMXRT_HAL_FLEXSPI_NOR_FLASH_H
