# CMake fragment for MicroPython extmod component

set(MPY_EXTMOD_DIR "${MPY_DIR}/extmod")
set(OOFATFS_DIR "${MPY_DIR}/lib/oofatfs")

set(SOURCE_EXTMOD
    ${MPY_EXTMOD_DIR}/machine_i2c.c
    ${MPY_EXTMOD_DIR}/machine_mem.c
    ${MPY_EXTMOD_DIR}/machine_pulse.c
    ${MPY_EXTMOD_DIR}/machine_signal.c
    ${MPY_EXTMOD_DIR}/machine_spi.c
    ${MPY_EXTMOD_DIR}/modbtree.c
    ${MPY_EXTMOD_DIR}/modframebuf.c
    ${MPY_EXTMOD_DIR}/moduasyncio.c
    ${MPY_EXTMOD_DIR}/modubinascii.c
    ${MPY_EXTMOD_DIR}/moducryptolib.c
    ${MPY_EXTMOD_DIR}/moductypes.c
    ${MPY_EXTMOD_DIR}/moduhashlib.c
    ${MPY_EXTMOD_DIR}/moduheapq.c
    ${MPY_EXTMOD_DIR}/modujson.c
    ${MPY_EXTMOD_DIR}/modurandom.c
    ${MPY_EXTMOD_DIR}/modure.c
    ${MPY_EXTMOD_DIR}/moduselect.c
    ${MPY_EXTMOD_DIR}/modussl_axtls.c
    ${MPY_EXTMOD_DIR}/modussl_mbedtls.c
    ${MPY_EXTMOD_DIR}/modutimeq.c
    ${MPY_EXTMOD_DIR}/moduwebsocket.c
    ${MPY_EXTMOD_DIR}/moduzlib.c
    ${MPY_EXTMOD_DIR}/modwebrepl.c
    ${MPY_EXTMOD_DIR}/uos_dupterm.c
    ${MPY_EXTMOD_DIR}/utime_mphal.c
    ${MPY_EXTMOD_DIR}/vfs.c
    ${MPY_EXTMOD_DIR}/vfs_blockdev.c
    ${MPY_EXTMOD_DIR}/vfs_fat.c
    ${MPY_EXTMOD_DIR}/vfs_fat_diskio.c
    ${MPY_EXTMOD_DIR}/vfs_fat_file.c
    ${MPY_EXTMOD_DIR}/vfs_lfs.c
    ${MPY_EXTMOD_DIR}/vfs_posix.c
    ${MPY_EXTMOD_DIR}/vfs_posix_file.c
    ${MPY_EXTMOD_DIR}/vfs_reader.c
    ${MPY_EXTMOD_DIR}/virtpin.c
    ${MPY_EXTMOD_DIR}/modbluetooth.c
    ${MPY_EXTMOD_DIR}/nimble/modbluetooth_nimble.c
    ${MPY_EXTMOD_DIR}/nimble/hal/hal_uart.c
)

set (INCLUDE_EXTMOD
    ${IDF_PATH}/components/bt/host/nimble/nimble/nimble/host/include
    ${IDF_PATH}/components/bt/host/nimble/esp-hci/include/
    ${IDF_PATH}/components/bt/host/nimble/nimble/nimble/include
    ${IDF_PATH}/components/bt/host/nimble/nimble/porting/nimble/include
    ${IDF_PATH}/components/bt/host/nimble/port/include
    ${IDF_PATH}/components/bt/host/nimble/nimble/porting/npl/freertos/include
    ${IDF_PATH}/components/bt/common/osi/include/osi
    ${IDF_PATH}/components/bt/host/nimble/nimble/nimble/host/util/include
    ${IDF_PATH}/components/bt/host/nimble/nimble/porting/npl/freertos/include
    ${IDF_PATH}/components/bt/host/nimble/nimble/nimble/host/services/gap/include
    ${IDF_PATH}/components/bt/host/nimble/nimble/nimble/host/services/gatt/include
    ${IDF_PATH}/components/bt/host/nimble/nimble/nimble/transport/uart/include
)
