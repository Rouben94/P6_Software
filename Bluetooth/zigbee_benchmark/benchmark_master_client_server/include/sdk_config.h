/**
 * Copyright (c) 2017 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIAFOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H
#include "bm_config.h"
// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif




// <q> NRFX_NVMC_ENABLED  - nrfx_nvmc - Non-volatile Memory Controller (NVMC) driver
 

#ifndef NRFX_NVMC_ENABLED
#define NRFX_NVMC_ENABLED 1
#endif



/*************************************** SEGGER RTT ***************************************************/

// <h> nRF_Segger_RTT

//==========================================================
// <h> segger_rtt - SEGGER RTT

//==========================================================
// <o> SEGGER_RTT_CONFIG_BUFFER_SIZE_UP - Size of upstream buffer.
// <i> Note that either @ref NRF_LOG_BACKEND_RTT_OUTPUT_BUFFER_SIZE
// <i> or this value is actually used. It depends on which one is bigger.

#ifndef SEGGER_RTT_CONFIG_BUFFER_SIZE_UP
#define SEGGER_RTT_CONFIG_BUFFER_SIZE_UP 512
#endif

// <o> SEGGER_RTT_CONFIG_MAX_NUM_UP_BUFFERS - Maximum number of upstream buffers.
#ifndef SEGGER_RTT_CONFIG_MAX_NUM_UP_BUFFERS
#define SEGGER_RTT_CONFIG_MAX_NUM_UP_BUFFERS 2
#endif

// <o> SEGGER_RTT_CONFIG_BUFFER_SIZE_DOWN - Size of downstream buffer.
#ifndef SEGGER_RTT_CONFIG_BUFFER_SIZE_DOWN
#define SEGGER_RTT_CONFIG_BUFFER_SIZE_DOWN 16
#endif

// <o> SEGGER_RTT_CONFIG_MAX_NUM_DOWN_BUFFERS - Maximum number of downstream buffers.
#ifndef SEGGER_RTT_CONFIG_MAX_NUM_DOWN_BUFFERS
#define SEGGER_RTT_CONFIG_MAX_NUM_DOWN_BUFFERS 2
#endif

// <o> SEGGER_RTT_CONFIG_DEFAULT_MODE  - RTT behavior if the buffer is full.

// <i> The following modes are supported:
// <i> - SKIP  - Do not block, output nothing.
// <i> - TRIM  - Do not block, output as much as fits.
// <i> - BLOCK - Wait until there is space in the buffer.
// <0=> SKIP
// <1=> TRIM
// <2=> BLOCK_IF_FIFO_FULL

#ifndef SEGGER_RTT_CONFIG_DEFAULT_MODE
#define SEGGER_RTT_CONFIG_DEFAULT_MODE 0
#endif

// </h>

// <e> NRF_CLI_RTT_ENABLED - nrf_cli_rtt - RTT command line interface transport
//==========================================================
#ifndef NRF_CLI_RTT_ENABLED
#define NRF_CLI_RTT_ENABLED 1
#endif
// <o> NRF_CLI_RTT_TERMINAL_ID - RTT terminal ID for CLI.
#ifndef NRF_CLI_RTT_TERMINAL_ID
#define NRF_CLI_RTT_TERMINAL_ID 0
#endif

// <o> NRF_CLI_RTT_TX_RETRY_DELAY_MS - Period before retrying writing to RTT
#ifndef NRF_CLI_RTT_TX_RETRY_DELAY_MS
#define NRF_CLI_RTT_TX_RETRY_DELAY_MS 10
#endif

// <o> NRF_CLI_RTT_TX_RETRY_CNT - Writing to RTT retries.
// <i> If RTT fails to accept any new data after retries
// <i> module assumes that host is not active and on next
// <i> request it will perform only one write attempt.
// <i> On successful writing, module assumes that host is active
// <i> and scheme with retry is applied again.

#ifndef NRF_CLI_RTT_TX_RETRY_CNT
#define NRF_CLI_RTT_TX_RETRY_CNT 5
#endif

// </e>

// <q> NRF_CLI_UART_ENABLED  - nrf_cli_uart - UART command line interface transport

#ifndef NRF_CLI_UART_ENABLED
#define NRF_CLI_UART_ENABLED 0
#endif
//==========================================================

/*************************************** APP USBD ***************************************************/

// </e>

// <e> APP_USBD_ENABLED - app_usbd - USB Device library
//==========================================================
#ifndef APP_USBD_ENABLED
#define APP_USBD_ENABLED 1
#endif
// <s> APP_USBD_VID - Vendor ID.

// <i> Note: This value is not editable in Configuration Wizard.
// <i> Vendor ID ordered from USB IF: http://www.usb.org/developers/vendor/
#ifndef APP_USBD_VID
#define APP_USBD_VID 0x1915
#endif

// <s> APP_USBD_PID - Product ID.

// <i> Note: This value is not editable in Configuration Wizard.
// <i> Selected Product ID
#ifndef APP_USBD_PID
#define APP_USBD_PID 0x520F
#endif

// <o> APP_USBD_DEVICE_VER_MAJOR - Major device version  <0-99>

// <i> Major device version, will be converted automatically to BCD notation. Use just decimal values.

#ifndef APP_USBD_DEVICE_VER_MAJOR
#define APP_USBD_DEVICE_VER_MAJOR 1
#endif

// <o> APP_USBD_DEVICE_VER_MINOR - Minor device version  <0-9>

// <i> Minor device version, will be converted automatically to BCD notation. Use just decimal values.

#ifndef APP_USBD_DEVICE_VER_MINOR
#define APP_USBD_DEVICE_VER_MINOR 0
#endif

// <o> APP_USBD_DEVICE_VER_SUB - Sub-minor device version  <0-9>

// <i> Sub-minor device version, will be converted automatically to BCD notation. Use just decimal values.

#ifndef APP_USBD_DEVICE_VER_SUB
#define APP_USBD_DEVICE_VER_SUB 0
#endif

// <q> APP_USBD_CONFIG_SELF_POWERED  - Self-powered device, as opposed to bus-powered.

#ifndef APP_USBD_CONFIG_SELF_POWERED
#define APP_USBD_CONFIG_SELF_POWERED 1
#endif

// <o> APP_USBD_CONFIG_MAX_POWER - MaxPower field in configuration descriptor in milliamps.  <0-500>

#ifndef APP_USBD_CONFIG_MAX_POWER
#define APP_USBD_CONFIG_MAX_POWER 100
#endif

// <q> APP_USBD_CONFIG_POWER_EVENTS_PROCESS  - Process power events.

// <i> Enable processing power events in USB event handler.

#ifndef APP_USBD_CONFIG_POWER_EVENTS_PROCESS
#define APP_USBD_CONFIG_POWER_EVENTS_PROCESS 1
#endif

// <e> APP_USBD_CONFIG_EVENT_QUEUE_ENABLE - Enable event queue.

// <i> This is the default configuration when all the events are placed into internal queue.
// <i> Disable it when an external queue is used like app_scheduler or if you wish to process all events inside interrupts.
// <i> Processing all events from the interrupt level adds requirement not to call any functions that modifies the USBD library state from the context higher than USB interrupt context.
// <i> Functions that modify USBD state are functions for sleep, wakeup, start, stop, enable, and disable.
//==========================================================
#ifndef APP_USBD_CONFIG_EVENT_QUEUE_ENABLE
#define APP_USBD_CONFIG_EVENT_QUEUE_ENABLE 1
#endif
// <o> APP_USBD_CONFIG_EVENT_QUEUE_SIZE - The size of the event queue.  <16-64>

// <i> The size of the queue for the events that would be processed in the main loop.

#ifndef APP_USBD_CONFIG_EVENT_QUEUE_SIZE
#define APP_USBD_CONFIG_EVENT_QUEUE_SIZE 32
#endif

// <o> APP_USBD_CONFIG_SOF_HANDLING_MODE  - Change SOF events handling mode.

// <i> Normal queue   - SOF events are pushed normally into the event queue.
// <i> Compress queue - SOF events are counted and binded with other events or executed when the queue is empty.
// <i>                  This prevents the queue from filling up with SOF events.
// <i> Interrupt      - SOF events are processed in interrupt.
// <0=> Normal queue
// <1=> Compress queue
// <2=> Interrupt

#ifndef APP_USBD_CONFIG_SOF_HANDLING_MODE
#define APP_USBD_CONFIG_SOF_HANDLING_MODE 1
#endif

// </e>

// <q> APP_USBD_CONFIG_SOF_TIMESTAMP_PROVIDE  - Provide a function that generates timestamps for logs based on the current SOF.

// <i> The function app_usbd_sof_timestamp_get is implemented if the logger is enabled.
// <i> Use it when initializing the logger.
// <i> SOF processing is always enabled when this configuration parameter is active.
// <i> Note: This option is configured outside of APP_USBD_CONFIG_LOG_ENABLED.
// <i> This means that it works even if the logging in this very module is disabled.

#ifndef APP_USBD_CONFIG_SOF_TIMESTAMP_PROVIDE
#define APP_USBD_CONFIG_SOF_TIMESTAMP_PROVIDE 0
#endif

// <o> APP_USBD_CONFIG_DESC_STRING_SIZE - Maximum size of the NULL-terminated string of the string descriptor.  <31-254>

// <i> 31 characters can be stored in the internal USB buffer used for transfers.
// <i> Any value higher than 31 creates an additional buffer just for descriptor strings.

#ifndef APP_USBD_CONFIG_DESC_STRING_SIZE
#define APP_USBD_CONFIG_DESC_STRING_SIZE 31
#endif

// <q> APP_USBD_CONFIG_DESC_STRING_UTF_ENABLED  - Enable UTF8 conversion.

// <i> Enable UTF8-encoded characters. In normal processing, only ASCII characters are available.

#ifndef APP_USBD_CONFIG_DESC_STRING_UTF_ENABLED
#define APP_USBD_CONFIG_DESC_STRING_UTF_ENABLED 0
#endif

// <s> APP_USBD_STRINGS_LANGIDS - Supported languages identifiers.

// <i> Note: This value is not editable in Configuration Wizard.
// <i> Comma-separated list of supported languages.
#ifndef APP_USBD_STRINGS_LANGIDS
#define APP_USBD_STRINGS_LANGIDS APP_USBD_LANG_AND_SUBLANG(APP_USBD_LANG_ENGLISH, APP_USBD_SUBLANG_ENGLISH_US)
#endif

// <e> APP_USBD_STRING_ID_MANUFACTURER - Define manufacturer string ID.

// <i> Setting ID to 0 disables the string.
//==========================================================
#ifndef APP_USBD_STRING_ID_MANUFACTURER
#define APP_USBD_STRING_ID_MANUFACTURER 1
#endif
// <q> APP_USBD_STRINGS_MANUFACTURER_EXTERN  - Define whether @ref APP_USBD_STRINGS_MANUFACTURER is created by macro or declared as a global variable.

#ifndef APP_USBD_STRINGS_MANUFACTURER_EXTERN
#define APP_USBD_STRINGS_MANUFACTURER_EXTERN 0
#endif

// <s> APP_USBD_STRINGS_MANUFACTURER - String descriptor for the manufacturer name.

// <i> Note: This value is not editable in Configuration Wizard.
// <i> Comma-separated list of manufacturer names for each defined language.
// <i> Use @ref APP_USBD_STRING_DESC macro to create string descriptor from a NULL-terminated string.
// <i> Use @ref APP_USBD_STRING_RAW8_DESC macro to create string descriptor from comma-separated uint8_t values.
// <i> Use @ref APP_USBD_STRING_RAW16_DESC macro to create string descriptor from comma-separated uint16_t values.
// <i> Alternatively, configure the macro to point to any internal variable pointer that already contains the descriptor.
// <i> Setting string to NULL disables that string.
// <i> The order of manufacturer names must be the same like in @ref APP_USBD_STRINGS_LANGIDS.
#ifndef APP_USBD_STRINGS_MANUFACTURER
#define APP_USBD_STRINGS_MANUFACTURER APP_USBD_STRING_DESC("Nordic Semiconductor")
#endif

// </e>

// <e> APP_USBD_STRING_ID_PRODUCT - Define product string ID.

// <i> Setting ID to 0 disables the string.
//==========================================================
#ifndef APP_USBD_STRING_ID_PRODUCT
#define APP_USBD_STRING_ID_PRODUCT 2
#endif
// <q> APP_USBD_STRINGS_PRODUCT_EXTERN  - Define whether @ref APP_USBD_STRINGS_PRODUCT is created by macro or declared as a global variable.

#ifndef APP_USBD_STRINGS_PRODUCT_EXTERN
#define APP_USBD_STRINGS_PRODUCT_EXTERN 0
#endif

// <s> APP_USBD_STRINGS_PRODUCT - String descriptor for the product name.

// <i> Note: This value is not editable in Configuration Wizard.
// <i> List of product names that is defined the same way like in @ref APP_USBD_STRINGS_MANUFACTURER.
#ifndef APP_USBD_STRINGS_PRODUCT
#define APP_USBD_STRINGS_PRODUCT APP_USBD_STRING_DESC("nRF52 USB CDC Demo")
#endif

// </e>

// <e> APP_USBD_STRING_ID_SERIAL - Define serial number string ID.

// <i> Setting ID to 0 disables the string.
//==========================================================
#ifndef APP_USBD_STRING_ID_SERIAL
#define APP_USBD_STRING_ID_SERIAL 3
#endif
// <q> APP_USBD_STRING_SERIAL_EXTERN  - Define whether @ref APP_USBD_STRING_SERIAL is created by macro or declared as a global variable.

#ifndef APP_USBD_STRING_SERIAL_EXTERN
#define APP_USBD_STRING_SERIAL_EXTERN 0
#endif

// <s> APP_USBD_STRING_SERIAL - String descriptor for the serial number.

// <i> Note: This value is not editable in Configuration Wizard.
// <i> Serial number that is defined the same way like in @ref APP_USBD_STRINGS_MANUFACTURER.
#ifndef APP_USBD_STRING_SERIAL
#define APP_USBD_STRING_SERIAL APP_USBD_STRING_DESC("000000000000")
#endif

// </e>

// <e> APP_USBD_STRING_ID_CONFIGURATION - Define configuration string ID.

// <i> Setting ID to 0 disables the string.
//==========================================================
#ifndef APP_USBD_STRING_ID_CONFIGURATION
#define APP_USBD_STRING_ID_CONFIGURATION 4
#endif
// <q> APP_USBD_STRING_CONFIGURATION_EXTERN  - Define whether @ref APP_USBD_STRINGS_CONFIGURATION is created by macro or declared as global variable.

#ifndef APP_USBD_STRING_CONFIGURATION_EXTERN
#define APP_USBD_STRING_CONFIGURATION_EXTERN 0
#endif

// <s> APP_USBD_STRINGS_CONFIGURATION - String descriptor for the device configuration.

// <i> Note: This value is not editable in Configuration Wizard.
// <i> Configuration string that is defined the same way like in @ref APP_USBD_STRINGS_MANUFACTURER.
#ifndef APP_USBD_STRINGS_CONFIGURATION
#define APP_USBD_STRINGS_CONFIGURATION APP_USBD_STRING_DESC("Default configuration")
#endif

// </e>

// <s> APP_USBD_STRINGS_USER - Default values for user strings.

// <i> Note: This value is not editable in Configuration Wizard.
// <i> This value stores all application specific user strings with the default initialization.
// <i> The setup is done by X-macros.
// <i> Expected macro parameters:
// <i> @code
// <i> X(mnemonic, [=str_idx], ...)
// <i> @endcode
// <i> - @c mnemonic: Mnemonic of the string descriptor that would be added to
// <i>                @ref app_usbd_string_desc_idx_t enumerator.
// <i> - @c str_idx : String index value, can be set or left empty.
// <i>                For example, WinUSB driver requires descriptor to be present on 0xEE index.
// <i>                Then use X(USBD_STRING_WINUSB, =0xEE, (APP_USBD_STRING_DESC(...)))
// <i> - @c ...     : List of string descriptors for each defined language.
#ifndef APP_USBD_STRINGS_USER
#define APP_USBD_STRINGS_USER X(APP_USER_1, , APP_USBD_STRING_DESC("User 1"))
#endif

/*************************************** NRF CLI Support ***************************************************/

// <h> nrf_cli - Command line interface

//==========================================================
// <q> NRF_CLI_ENABLED  - Enable/disable the CLI module.

#ifndef NRF_CLI_ENABLED
#define NRF_CLI_ENABLED 1
#endif

// <o> NRF_CLI_ARGC_MAX - Maximum number of parameters passed to the command handler.
#ifndef NRF_CLI_ARGC_MAX
#define NRF_CLI_ARGC_MAX 12
#endif

// <q> NRF_CLI_BUILD_IN_CMDS_ENABLED  - CLI built-in commands.

#ifndef NRF_CLI_BUILD_IN_CMDS_ENABLED
#define NRF_CLI_BUILD_IN_CMDS_ENABLED 1
#endif

// <o> NRF_CLI_CMD_BUFF_SIZE - Maximum buffer size for a single command.
#ifndef NRF_CLI_CMD_BUFF_SIZE
#define NRF_CLI_CMD_BUFF_SIZE 512
#endif

// <q> NRF_CLI_ECHO_STATUS  - CLI echo status. If set, echo is ON.

#ifndef NRF_CLI_ECHO_STATUS
#define NRF_CLI_ECHO_STATUS 1
#endif

// <q> NRF_CLI_WILDCARD_ENABLED  - Enable wildcard functionality for CLI commands.

#ifndef NRF_CLI_WILDCARD_ENABLED
#define NRF_CLI_WILDCARD_ENABLED 1
#endif

// <q> NRF_CLI_METAKEYS_ENABLED  - Enable additional control keys for CLI commands like ctrl+a, ctrl+e, ctrl+w, ctrl+u

#ifndef NRF_CLI_METAKEYS_ENABLED
#define NRF_CLI_METAKEYS_ENABLED 1
#endif

// <o> NRF_CLI_PRINTF_BUFF_SIZE - Maximum print buffer size.
#ifndef NRF_CLI_PRINTF_BUFF_SIZE
#define NRF_CLI_PRINTF_BUFF_SIZE 23
#endif

// <e> NRF_CLI_HISTORY_ENABLED - Enable CLI history mode.
//==========================================================
#ifndef NRF_CLI_HISTORY_ENABLED
#define NRF_CLI_HISTORY_ENABLED 1
#endif
// <o> NRF_CLI_HISTORY_ELEMENT_SIZE - Size of one memory object reserved for CLI history.
#ifndef NRF_CLI_HISTORY_ELEMENT_SIZE
#define NRF_CLI_HISTORY_ELEMENT_SIZE 32
#endif

// <o> NRF_CLI_HISTORY_ELEMENT_COUNT - Number of history memory objects.
#ifndef NRF_CLI_HISTORY_ELEMENT_COUNT
#define NRF_CLI_HISTORY_ELEMENT_COUNT 8
#endif

// </e>

// <q> NRF_CLI_VT100_COLORS_ENABLED  - CLI VT100 colors.

#ifndef NRF_CLI_VT100_COLORS_ENABLED
#define NRF_CLI_VT100_COLORS_ENABLED 1
#endif

// <q> NRF_CLI_STATISTICS_ENABLED  - Enable CLI statistics.

#ifndef NRF_CLI_STATISTICS_ENABLED
#define NRF_CLI_STATISTICS_ENABLED 1
#endif

// <q> NRF_CLI_LOG_BACKEND  - Enable logger backend interface.

#ifndef NRF_CLI_LOG_BACKEND
#define NRF_CLI_LOG_BACKEND 1
#endif

// <q> NRF_CLI_USES_TASK_MANAGER_ENABLED  - Enable CLI to use task_manager

#ifndef NRF_CLI_USES_TASK_MANAGER_ENABLED
#define NRF_CLI_USES_TASK_MANAGER_ENABLED 0
#endif

// </h>
//==========================================================

// <h> nrf_cli_cdc_acm - CDC ACM command line interface transport

//==========================================================
// <q> NRF_CLI_CDC_ACM_ENABLED  - Enable/disable the CLI CDC ACM module.

#ifndef NRF_CLI_CDC_ACM_ENABLED
#define NRF_CLI_CDC_ACM_ENABLED 0
#endif

// <o> NRF_CLI_CDC_ACM_COMM_INTERFACE - COMM interface number.
#ifndef NRF_CLI_CDC_ACM_COMM_INTERFACE
#define NRF_CLI_CDC_ACM_COMM_INTERFACE 0
#endif

// <s> NRF_CLI_CDC_ACM_COMM_EPIN - COMM IN endpoint number.
#ifndef NRF_CLI_CDC_ACM_COMM_EPIN
#define NRF_CLI_CDC_ACM_COMM_EPIN NRF_DRV_USBD_EPIN2
#endif

// <o> NRF_CLI_CDC_ACM_DATA_INTERFACE - DATA interface number.
#ifndef NRF_CLI_CDC_ACM_DATA_INTERFACE
#define NRF_CLI_CDC_ACM_DATA_INTERFACE 1
#endif

// <s> NRF_CLI_CDC_ACM_DATA_EPIN - DATA IN endpoint number.
#ifndef NRF_CLI_CDC_ACM_DATA_EPIN
#define NRF_CLI_CDC_ACM_DATA_EPIN NRF_DRV_USBD_EPIN1
#endif

// <s> NRF_CLI_CDC_ACM_DATA_EPOUT - DATA OUT endpoint number.
#ifndef NRF_CLI_CDC_ACM_DATA_EPOUT
#define NRF_CLI_CDC_ACM_DATA_EPOUT NRF_DRV_USBD_EPOUT1
#endif

// </h>
//==========================================================

// <e> USBD_ENABLED - nrf_drv_usbd - Software Component
//==========================================================
#ifndef USBD_ENABLED
#define USBD_ENABLED 1
#endif
// <o> USBD_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef USBD_CONFIG_IRQ_PRIORITY
#define USBD_CONFIG_IRQ_PRIORITY 6
#endif

// <o> USBD_CONFIG_DMASCHEDULER_MODE  - USBD SMA scheduler working scheme

// <0=> Prioritized access
// <1=> Round Robin

#ifndef USBD_CONFIG_DMASCHEDULER_MODE
#define USBD_CONFIG_DMASCHEDULER_MODE 0
#endif

// <q> USBD_CONFIG_DMASCHEDULER_ISO_BOOST  - Give priority to isochronous transfers

// <i> This option gives priority to isochronous transfers.
// <i> Enabling it assures that isochronous transfers are always processed,
// <i> even if multiple other transfers are pending.
// <i> Isochronous endpoints are prioritized before the usbd_dma_scheduler_algorithm
// <i> function is called, so the option is independent of the algorithm chosen.

#ifndef USBD_CONFIG_DMASCHEDULER_ISO_BOOST
#define USBD_CONFIG_DMASCHEDULER_ISO_BOOST 1
#endif

// <q> USBD_CONFIG_ISO_IN_ZLP  - Respond to an IN token on ISO IN endpoint with ZLP when no data is ready

// <i> If set, ISO IN endpoint will respond to an IN token with ZLP when no data is ready to be sent.
// <i> Else, there will be no response.
// <i> NOTE: This option does not work on Engineering A chip.

#ifndef USBD_CONFIG_ISO_IN_ZLP
#define USBD_CONFIG_ISO_IN_ZLP 0
#endif

// </e>

// </h>
//==========================================================
// <e> POWER_ENABLED - nrf_drv_power - POWER peripheral driver - legacy layer
//==========================================================
#ifndef POWER_ENABLED
#define POWER_ENABLED 1
#endif
// <o> POWER_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef POWER_CONFIG_IRQ_PRIORITY
#define POWER_CONFIG_IRQ_PRIORITY 6
#endif

// <q> POWER_CONFIG_DEFAULT_DCDCEN  - The default configuration of main DCDC regulator

// <i> This settings means only that components for DCDC regulator are installed and it can be enabled.

#ifndef NRFX_POWER_CONFIG_DEFAULT_DCDCEN
#define NRFX_POWER_CONFIG_DEFAULT_DCDCEN 0
#endif

// <q> NRFX_POWER_CONFIG_DEFAULT_DCDCENHV  - The default configuration of High Voltage DCDC regulator

// <i> This settings means only that components for DCDC regulator are installed and it can be enabled.

#ifndef NRFX_POWER_CONFIG_DEFAULT_DCDCENHV
#define NRFX_POWER_CONFIG_DEFAULT_DCDCENHV 0
#endif

// </e>

// <h> app_usbd_cdc_acm - USB CDC ACM class

//==========================================================
// <q> APP_USBD_CDC_ACM_ENABLED  - Enabling USBD CDC ACM Class library

#ifndef APP_USBD_CDC_ACM_ENABLED
#define APP_USBD_CDC_ACM_ENABLED 1
#endif

// <q> APP_USBD_CDC_ACM_ZLP_ON_EPSIZE_WRITE  - Send ZLP on write with same size as endpoint

// <i> If enabled, CDC ACM class will automatically send a zero length packet after transfer which has the same size as endpoint.
// <i> This may limit throughput if a lot of binary data is sent, but in terminal mode operation it makes sure that the data is always displayed right after it is sent.

#ifndef APP_USBD_CDC_ACM_ZLP_ON_EPSIZE_WRITE
#define APP_USBD_CDC_ACM_ZLP_ON_EPSIZE_WRITE 1
#endif

// </h>
//==========================================================



// </h>
//==========================================================

/*************************************** FDS Flash Save Definitions ***************************************************/

// <e> FDS_ENABLED - fds - Flash data storage module
//==========================================================
#ifndef FDS_ENABLED
#define FDS_ENABLED 1
#endif
// <h> Pages - Virtual page settings

// <i> Configure the number of virtual pages to use and their size.
//==========================================================
// <o> FDS_VIRTUAL_PAGES - Number of virtual flash pages to use.
// <i> One of the virtual pages is reserved by the system for garbage collection.
// <i> Therefore, the minimum is two virtual pages: one page to store data and one page to be used by the system for garbage collection.
// <i> The total amount of flash memory that is used by FDS amounts to @ref FDS_VIRTUAL_PAGES * @ref FDS_VIRTUAL_PAGE_SIZE * 4 bytes.

#ifndef FDS_VIRTUAL_PAGES
#define FDS_VIRTUAL_PAGES 20
#endif

// <o> FDS_VIRTUAL_PAGE_SIZE  - The size of a virtual flash page.

// <i> Expressed in number of 4-byte words.
// <i> By default, a virtual page is the same size as a physical page.
// <i> The size of a virtual page must be a multiple of the size of a physical page.
// <1024=> 1024
// <2048=> 2048

#ifndef FDS_VIRTUAL_PAGE_SIZE
#define FDS_VIRTUAL_PAGE_SIZE 1024
#endif

// <o> FDS_VIRTUAL_PAGES_RESERVED - The number of virtual flash pages that are used by other modules.
// <i> FDS module stores its data in the last pages of the flash memory.
// <i> By setting this value, you can move flash end address used by the FDS.
// <i> As a result the reserved space can be used by other modules.

#ifndef FDS_VIRTUAL_PAGES_RESERVED
#define FDS_VIRTUAL_PAGES_RESERVED 0
#endif

// </h>
//==========================================================

// <h> Backend - Backend configuration

// <i> Configure which nrf_fstorage backend is used by FDS to write to flash.
//==========================================================
// <o> FDS_BACKEND  - FDS flash backend.

// <i> NRF_FSTORAGE_SD uses the nrf_fstorage_sd backend implementation using the SoftDevice API. Use this if you have a SoftDevice present.
// <i> NRF_FSTORAGE_NVMC uses the nrf_fstorage_nvmc implementation. Use this setting if you don't use the SoftDevice.
// <1=> NRF_FSTORAGE_NVMC
// <2=> NRF_FSTORAGE_SD

#ifndef FDS_BACKEND
#define FDS_BACKEND 2
#endif

// </h>
//==========================================================

// <h> Queue - Queue settings

//==========================================================
// <o> FDS_OP_QUEUE_SIZE - Size of the internal queue.
// <i> Increase this value if you frequently get synchronous FDS_ERR_NO_SPACE_IN_QUEUES errors.

#ifndef FDS_OP_QUEUE_SIZE
#define FDS_OP_QUEUE_SIZE 4
#endif

// </h>
//==========================================================

// <h> CRC - CRC functionality

//==========================================================
// <e> FDS_CRC_CHECK_ON_READ - Enable CRC checks.

// <i> Save a record's CRC when it is written to flash and check it when the record is opened.
// <i> Records with an incorrect CRC can still be 'seen' by the user using FDS functions, but they cannot be opened.
// <i> Additionally, they will not be garbage collected until they are deleted.
//==========================================================
#ifndef FDS_CRC_CHECK_ON_READ
#define FDS_CRC_CHECK_ON_READ 1
#endif
// <o> FDS_CRC_CHECK_ON_WRITE  - Perform a CRC check on newly written records.

// <i> Perform a CRC check on newly written records.
// <i> This setting can be used to make sure that the record data was not altered while being written to flash.
// <1=> Enabled
// <0=> Disabled

#ifndef FDS_CRC_CHECK_ON_WRITE
#define FDS_CRC_CHECK_ON_WRITE 0
#endif

// </e>

// </h>
//==========================================================

// <h> Users - Number of users

//==========================================================
// <o> FDS_MAX_USERS - Maximum number of callbacks that can be registered.
#ifndef FDS_MAX_USERS
#define FDS_MAX_USERS 4
#endif

// <q> CRC16_ENABLED  - crc16 - CRC16 calculation routines

#ifndef CRC16_ENABLED
#define CRC16_ENABLED 1
#endif

// <e> MEM_MANAGER_ENABLED - mem_manager - Dynamic memory allocator
//==========================================================
#ifndef MEM_MANAGER_ENABLED
#define MEM_MANAGER_ENABLED 1
#endif
// <o> MEMORY_MANAGER_SMALL_BLOCK_COUNT - Size of each memory blocks identified as 'small' block.  <0-255>

#ifndef MEMORY_MANAGER_SMALL_BLOCK_COUNT
#define MEMORY_MANAGER_SMALL_BLOCK_COUNT 64
#endif

// <o> MEMORY_MANAGER_SMALL_BLOCK_SIZE -  Size of each memory blocks identified as 'small' block.
// <i>  Size of each memory blocks identified as 'small' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_SMALL_BLOCK_SIZE
#define MEMORY_MANAGER_SMALL_BLOCK_SIZE 64
#endif

// <o> MEMORY_MANAGER_MEDIUM_BLOCK_COUNT - Size of each memory blocks identified as 'medium' block.  <0-255>

#ifndef MEMORY_MANAGER_MEDIUM_BLOCK_COUNT
#define MEMORY_MANAGER_MEDIUM_BLOCK_COUNT 16
#endif

// <o> MEMORY_MANAGER_MEDIUM_BLOCK_SIZE -  Size of each memory blocks identified as 'medium' block.
// <i>  Size of each memory blocks identified as 'medium' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_MEDIUM_BLOCK_SIZE
#define MEMORY_MANAGER_MEDIUM_BLOCK_SIZE 256
#endif

// <o> MEMORY_MANAGER_LARGE_BLOCK_COUNT - Size of each memory blocks identified as 'large' block.  <0-255>

#ifndef MEMORY_MANAGER_LARGE_BLOCK_COUNT
#define MEMORY_MANAGER_LARGE_BLOCK_COUNT 4
#endif

// <o> MEMORY_MANAGER_LARGE_BLOCK_SIZE -  Size of each memory blocks identified as 'large' block.
// <i>  Size of each memory blocks identified as 'large' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_LARGE_BLOCK_SIZE
#define MEMORY_MANAGER_LARGE_BLOCK_SIZE 512
#endif

// <o> MEMORY_MANAGER_XLARGE_BLOCK_COUNT - Size of each memory blocks identified as 'extra large' block.  <0-255>

#ifndef MEMORY_MANAGER_XLARGE_BLOCK_COUNT
#define MEMORY_MANAGER_XLARGE_BLOCK_COUNT 5
#endif

// <o> MEMORY_MANAGER_XLARGE_BLOCK_SIZE -  Size of each memory blocks identified as 'extra large' block.
// <i>  Size of each memory blocks identified as 'extra large' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_XLARGE_BLOCK_SIZE
#define MEMORY_MANAGER_XLARGE_BLOCK_SIZE 2816
#endif

// <o> MEMORY_MANAGER_XXLARGE_BLOCK_COUNT - Size of each memory blocks identified as 'extra extra large' block.  <0-255>

#ifndef MEMORY_MANAGER_XXLARGE_BLOCK_COUNT
#define MEMORY_MANAGER_XXLARGE_BLOCK_COUNT 0
#endif

// <o> MEMORY_MANAGER_XXLARGE_BLOCK_SIZE -  Size of each memory blocks identified as 'extra extra large' block.
// <i>  Size of each memory blocks identified as 'extra extra large' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_XXLARGE_BLOCK_SIZE
#define MEMORY_MANAGER_XXLARGE_BLOCK_SIZE 3444
#endif

// <o> MEMORY_MANAGER_XSMALL_BLOCK_COUNT - Size of each memory blocks identified as 'extra small' block.  <0-255>

#ifndef MEMORY_MANAGER_XSMALL_BLOCK_COUNT
#define MEMORY_MANAGER_XSMALL_BLOCK_COUNT 0
#endif

// <o> MEMORY_MANAGER_XSMALL_BLOCK_SIZE -  Size of each memory blocks identified as 'extra small' block.
// <i>  Size of each memory blocks identified as 'extra large' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_XSMALL_BLOCK_SIZE
#define MEMORY_MANAGER_XSMALL_BLOCK_SIZE 64
#endif

// <o> MEMORY_MANAGER_XXSMALL_BLOCK_COUNT - Size of each memory blocks identified as 'extra extra small' block.  <0-255>

#ifndef MEMORY_MANAGER_XXSMALL_BLOCK_COUNT
#define MEMORY_MANAGER_XXSMALL_BLOCK_COUNT 0
#endif

// <o> MEMORY_MANAGER_XXSMALL_BLOCK_SIZE -  Size of each memory blocks identified as 'extra extra small' block.
// <i>  Size of each memory blocks identified as 'extra extra small' block. Memory block are recommended to be word-sized.

#ifndef MEMORY_MANAGER_XXSMALL_BLOCK_SIZE
#define MEMORY_MANAGER_XXSMALL_BLOCK_SIZE 32
#endif

// <e> MEM_MANAGER_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef MEM_MANAGER_CONFIG_LOG_ENABLED
#define MEM_MANAGER_CONFIG_LOG_ENABLED 0
#endif
// <o> MEM_MANAGER_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef MEM_MANAGER_CONFIG_LOG_LEVEL
#define MEM_MANAGER_CONFIG_LOG_LEVEL 3
#endif

// <o> MEM_MANAGER_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef MEM_MANAGER_CONFIG_INFO_COLOR
#define MEM_MANAGER_CONFIG_INFO_COLOR 0
#endif

// <o> MEM_MANAGER_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef MEM_MANAGER_CONFIG_DEBUG_COLOR
#define MEM_MANAGER_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <q> MEM_MANAGER_DISABLE_API_PARAM_CHECK  - Disable API parameter checks in the module.

#ifndef MEM_MANAGER_DISABLE_API_PARAM_CHECK
#define MEM_MANAGER_DISABLE_API_PARAM_CHECK 0
#endif

// </e>

/************************************************************************************************************************/


// </h>


// </h>
//==========================================================

// </h>
//==========================================================

//==========================================================
// <e> NRF_MPU_LIB_ENABLED - nrf_mpu_lib - Module for MPU
//==========================================================
#ifndef NRF_MPU_LIB_ENABLED
#define NRF_MPU_LIB_ENABLED 1
#endif
// <q> NRF_MPU_LIB_CLI_CMDS  - Enable CLI commands specific to the module.

#ifndef NRF_MPU_LIB_CLI_CMDS
#define NRF_MPU_LIB_CLI_CMDS 0
#endif

// </e>

// <e> NRF_STACK_GUARD_ENABLED - nrf_stack_guard - Stack guard
//==========================================================
#ifndef NRF_STACK_GUARD_ENABLED
#define NRF_STACK_GUARD_ENABLED 1
#endif
// <o> NRF_STACK_GUARD_CONFIG_SIZE  - Size of the stack guard.

// <5=> 32 bytes
// <6=> 64 bytes
// <7=> 128 bytes
// <8=> 256 bytes
// <9=> 512 bytes
// <10=> 1024 bytes
// <11=> 2048 bytes
// <12=> 4096 bytes

#ifndef NRF_STACK_GUARD_CONFIG_SIZE
#define NRF_STACK_GUARD_CONFIG_SIZE 7
#endif

// </e>

// </h>
//==========================================================

// <h> nRF_Drivers

//==========================================================
// <e> GPIOTE_ENABLED - nrf_drv_gpiote - GPIOTE peripheral driver - legacy layer
//==========================================================
#ifndef GPIOTE_ENABLED
#define GPIOTE_ENABLED 1
#endif
// <o> GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS - Number of lower power input pins
#ifndef GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS
#define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 4
#endif

// <o> GPIOTE_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef GPIOTE_CONFIG_IRQ_PRIORITY
#define GPIOTE_CONFIG_IRQ_PRIORITY 6
#endif

// </e>

// <e> NRFX_PPI_ENABLED - nrfx_ppi - PPI peripheral allocator
//==========================================================
#ifndef NRFX_PPI_ENABLED
#define NRFX_PPI_ENABLED 1
#endif
// <e> NRFX_PPI_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRFX_PPI_CONFIG_LOG_ENABLED
#define NRFX_PPI_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_PPI_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRFX_PPI_CONFIG_LOG_LEVEL
#define NRFX_PPI_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_PPI_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_PPI_CONFIG_INFO_COLOR
#define NRFX_PPI_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_PPI_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_PPI_CONFIG_DEBUG_COLOR
#define NRFX_PPI_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRFX_CLOCK_ENABLED - nrfx_clock - CLOCK peripheral driver
//==========================================================
#ifndef NRFX_CLOCK_ENABLED
#define NRFX_CLOCK_ENABLED 1
#endif
// <o> NRFX_CLOCK_CONFIG_LF_SRC  - LF Clock Source

// <0=> RC
// <1=> XTAL
// <2=> Synth
// <131073=> External Low Swing
// <196609=> External Full Swing

#ifndef NRFX_CLOCK_CONFIG_LF_SRC
#define NRFX_CLOCK_CONFIG_LF_SRC 1
#endif

// <o> NRFX_CLOCK_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef NRFX_CLOCK_CONFIG_IRQ_PRIORITY
#define NRFX_CLOCK_CONFIG_IRQ_PRIORITY 6
#endif

// <e> NRFX_CLOCK_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRFX_CLOCK_CONFIG_LOG_ENABLED
#define NRFX_CLOCK_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_CLOCK_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRFX_CLOCK_CONFIG_LOG_LEVEL
#define NRFX_CLOCK_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_CLOCK_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_CLOCK_CONFIG_INFO_COLOR
#define NRFX_CLOCK_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_CLOCK_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_CLOCK_CONFIG_DEBUG_COLOR
#define NRFX_CLOCK_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> NRFX_GPIOTE_ENABLED - nrfx_gpiote - GPIOTE peripheral driver
//==========================================================
#ifndef NRFX_GPIOTE_ENABLED
#define NRFX_GPIOTE_ENABLED 1
#endif
// <o> NRFX_GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS - Number of lower power input pins
#ifndef NRFX_GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS
#define NRFX_GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 1
#endif

// <o> NRFX_GPIOTE_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef NRFX_GPIOTE_CONFIG_IRQ_PRIORITY
#define NRFX_GPIOTE_CONFIG_IRQ_PRIORITY 6
#endif

// <e> NRFX_GPIOTE_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRFX_GPIOTE_CONFIG_LOG_ENABLED
#define NRFX_GPIOTE_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_GPIOTE_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRFX_GPIOTE_CONFIG_LOG_LEVEL
#define NRFX_GPIOTE_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_GPIOTE_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_GPIOTE_CONFIG_INFO_COLOR
#define NRFX_GPIOTE_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_GPIOTE_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_GPIOTE_CONFIG_DEBUG_COLOR
#define NRFX_GPIOTE_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> NRFX_PRS_ENABLED - nrfx_prs - Peripheral Resource Sharing module
//==========================================================
#ifndef NRFX_PRS_ENABLED
#define NRFX_PRS_ENABLED 1
#endif
// <q> NRFX_PRS_BOX_0_ENABLED  - Enables box 0 in the module.

#ifndef NRFX_PRS_BOX_0_ENABLED
#define NRFX_PRS_BOX_0_ENABLED 0
#endif

// <q> NRFX_PRS_BOX_1_ENABLED  - Enables box 1 in the module.

#ifndef NRFX_PRS_BOX_1_ENABLED
#define NRFX_PRS_BOX_1_ENABLED 0
#endif

// <q> NRFX_PRS_BOX_2_ENABLED  - Enables box 2 in the module.

#ifndef NRFX_PRS_BOX_2_ENABLED
#define NRFX_PRS_BOX_2_ENABLED 0
#endif

// <q> NRFX_PRS_BOX_3_ENABLED  - Enables box 3 in the module.

#ifndef NRFX_PRS_BOX_3_ENABLED
#define NRFX_PRS_BOX_3_ENABLED 0
#endif

// <q> NRFX_PRS_BOX_4_ENABLED  - Enables box 4 in the module.

#ifndef NRFX_PRS_BOX_4_ENABLED
#define NRFX_PRS_BOX_4_ENABLED 1
#endif

// <e> NRFX_PRS_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRFX_PRS_CONFIG_LOG_ENABLED
#define NRFX_PRS_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_PRS_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRFX_PRS_CONFIG_LOG_LEVEL
#define NRFX_PRS_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_PRS_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_PRS_CONFIG_INFO_COLOR
#define NRFX_PRS_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_PRS_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_PRS_CONFIG_DEBUG_COLOR
#define NRFX_PRS_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> NRFX_RNG_ENABLED - nrfx_rng - RNG peripheral driver
//==========================================================
#ifndef NRFX_RNG_ENABLED
#define NRFX_RNG_ENABLED 0
#endif
// <q> NRFX_RNG_CONFIG_ERROR_CORRECTION  - Error correction

#ifndef NRFX_RNG_CONFIG_ERROR_CORRECTION
#define NRFX_RNG_CONFIG_ERROR_CORRECTION 1
#endif

// <o> NRFX_RNG_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef NRFX_RNG_CONFIG_IRQ_PRIORITY
#define NRFX_RNG_CONFIG_IRQ_PRIORITY 6
#endif

// <e> NRFX_RNG_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRFX_RNG_CONFIG_LOG_ENABLED
#define NRFX_RNG_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_RNG_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRFX_RNG_CONFIG_LOG_LEVEL
#define NRFX_RNG_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_RNG_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_RNG_CONFIG_INFO_COLOR
#define NRFX_RNG_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_RNG_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_RNG_CONFIG_DEBUG_COLOR
#define NRFX_RNG_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <q> NRFX_SYSTICK_ENABLED  - nrfx_systick - ARM(R) SysTick driver

#ifndef NRFX_SYSTICK_ENABLED
#define NRFX_SYSTICK_ENABLED 1
#endif

// <e> NRFX_TIMER_ENABLED - nrfx_timer - TIMER periperal driver
//==========================================================
#ifndef NRFX_TIMER_ENABLED
#define NRFX_TIMER_ENABLED 0
#endif
// <q> NRFX_TIMER0_ENABLED  - Enable TIMER0 instance

#ifndef NRFX_TIMER0_ENABLED
#define NRFX_TIMER0_ENABLED 1
#endif

// <q> NRFX_TIMER1_ENABLED  - Enable TIMER1 instance

#ifndef NRFX_TIMER1_ENABLED
#define NRFX_TIMER1_ENABLED 0
#endif

// <q> NRFX_TIMER2_ENABLED  - Enable TIMER2 instance

#ifndef NRFX_TIMER2_ENABLED
#define NRFX_TIMER2_ENABLED 0
#endif

// <q> NRFX_TIMER3_ENABLED  - Enable TIMER3 instance

#ifndef NRFX_TIMER3_ENABLED
#define NRFX_TIMER3_ENABLED 0
#endif

// <q> NRFX_TIMER4_ENABLED  - Enable TIMER4 instance

#ifndef NRFX_TIMER4_ENABLED
#define NRFX_TIMER4_ENABLED 1
#endif

// <o> NRFX_TIMER_DEFAULT_CONFIG_FREQUENCY  - Timer frequency if in Timer mode

// <0=> 16 MHz
// <1=> 8 MHz
// <2=> 4 MHz
// <3=> 2 MHz
// <4=> 1 MHz
// <5=> 500 kHz
// <6=> 250 kHz
// <7=> 125 kHz
// <8=> 62.5 kHz
// <9=> 31.25 kHz

#ifndef NRFX_TIMER_DEFAULT_CONFIG_FREQUENCY
#define NRFX_TIMER_DEFAULT_CONFIG_FREQUENCY 0
#endif

// <o> NRFX_TIMER_DEFAULT_CONFIG_MODE  - Timer mode or operation

// <0=> Timer
// <1=> Counter

#ifndef NRFX_TIMER_DEFAULT_CONFIG_MODE
#define NRFX_TIMER_DEFAULT_CONFIG_MODE 0
#endif

// <o> NRFX_TIMER_DEFAULT_CONFIG_BIT_WIDTH  - Timer counter bit width

// <0=> 16 bit
// <1=> 8 bit
// <2=> 24 bit
// <3=> 32 bit

#ifndef NRFX_TIMER_DEFAULT_CONFIG_BIT_WIDTH
#define NRFX_TIMER_DEFAULT_CONFIG_BIT_WIDTH 0
#endif

// <o> NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY
#define NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <e> NRFX_TIMER_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRFX_TIMER_CONFIG_LOG_ENABLED
#define NRFX_TIMER_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_TIMER_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRFX_TIMER_CONFIG_LOG_LEVEL
#define NRFX_TIMER_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_TIMER_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_TIMER_CONFIG_INFO_COLOR
#define NRFX_TIMER_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_TIMER_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_TIMER_CONFIG_DEBUG_COLOR
#define NRFX_TIMER_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> NRFX_UARTE_ENABLED - nrfx_uarte - UARTE peripheral driver
//==========================================================
#ifndef NRFX_UARTE_ENABLED
#define NRFX_UARTE_ENABLED 1
#endif
// <o> NRFX_UARTE0_ENABLED - Enable UARTE0 instance
#ifndef NRFX_UARTE0_ENABLED
#define NRFX_UARTE0_ENABLED 0
#endif

// <o> NRFX_UARTE1_ENABLED - Enable UARTE1 instance
#ifndef NRFX_UARTE1_ENABLED
#define NRFX_UARTE1_ENABLED 0
#endif

// <o> NRFX_UARTE_DEFAULT_CONFIG_HWFC  - Hardware Flow Control

// <0=> Disabled
// <1=> Enabled

#ifndef NRFX_UARTE_DEFAULT_CONFIG_HWFC
#define NRFX_UARTE_DEFAULT_CONFIG_HWFC 0
#endif

// <o> NRFX_UARTE_DEFAULT_CONFIG_PARITY  - Parity

// <0=> Excluded
// <14=> Included

#ifndef NRFX_UARTE_DEFAULT_CONFIG_PARITY
#define NRFX_UARTE_DEFAULT_CONFIG_PARITY 0
#endif

// <o> NRFX_UARTE_DEFAULT_CONFIG_BAUDRATE  - Default Baudrate

// <323584=> 1200 baud
// <643072=> 2400 baud
// <1290240=> 4800 baud
// <2576384=> 9600 baud
// <3862528=> 14400 baud
// <5152768=> 19200 baud
// <7716864=> 28800 baud
// <8388608=> 31250 baud
// <10289152=> 38400 baud
// <15007744=> 56000 baud
// <15400960=> 57600 baud
// <20615168=> 76800 baud
// <30801920=> 115200 baud
// <61865984=> 230400 baud
// <67108864=> 250000 baud
// <121634816=> 460800 baud
// <251658240=> 921600 baud
// <268435456=> 1000000 baud

#ifndef NRFX_UARTE_DEFAULT_CONFIG_BAUDRATE
#define NRFX_UARTE_DEFAULT_CONFIG_BAUDRATE 30801920
#endif

// <o> NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY
#define NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <e> NRFX_UARTE_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRFX_UARTE_CONFIG_LOG_ENABLED
#define NRFX_UARTE_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_UARTE_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRFX_UARTE_CONFIG_LOG_LEVEL
#define NRFX_UARTE_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_UARTE_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_UARTE_CONFIG_INFO_COLOR
#define NRFX_UARTE_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_UARTE_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_UARTE_CONFIG_DEBUG_COLOR
#define NRFX_UARTE_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> NRFX_UART_ENABLED - nrfx_uart - UART peripheral driver
//==========================================================
#ifndef NRFX_UART_ENABLED
#define NRFX_UART_ENABLED 1
#endif
// <o> NRFX_UART0_ENABLED - Enable UART0 instance
#ifndef NRFX_UART0_ENABLED
#define NRFX_UART0_ENABLED 0
#endif

// <o> NRFX_UART_DEFAULT_CONFIG_HWFC  - Hardware Flow Control

// <0=> Disabled
// <1=> Enabled

#ifndef NRFX_UART_DEFAULT_CONFIG_HWFC
#define NRFX_UART_DEFAULT_CONFIG_HWFC 0
#endif

// <o> NRFX_UART_DEFAULT_CONFIG_PARITY  - Parity

// <0=> Excluded
// <14=> Included

#ifndef NRFX_UART_DEFAULT_CONFIG_PARITY
#define NRFX_UART_DEFAULT_CONFIG_PARITY 0
#endif

// <o> NRFX_UART_DEFAULT_CONFIG_BAUDRATE  - Default Baudrate

// <323584=> 1200 baud
// <643072=> 2400 baud
// <1290240=> 4800 baud
// <2576384=> 9600 baud
// <3866624=> 14400 baud
// <5152768=> 19200 baud
// <7729152=> 28800 baud
// <8388608=> 31250 baud
// <10309632=> 38400 baud
// <15007744=> 56000 baud
// <15462400=> 57600 baud
// <20615168=> 76800 baud
// <30924800=> 115200 baud
// <61845504=> 230400 baud
// <67108864=> 250000 baud
// <123695104=> 460800 baud
// <247386112=> 921600 baud
// <268435456=> 1000000 baud

#ifndef NRFX_UART_DEFAULT_CONFIG_BAUDRATE
#define NRFX_UART_DEFAULT_CONFIG_BAUDRATE 30801920
#endif

// <o> NRFX_UART_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef NRFX_UART_DEFAULT_CONFIG_IRQ_PRIORITY
#define NRFX_UART_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <e> NRFX_UART_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRFX_UART_CONFIG_LOG_ENABLED
#define NRFX_UART_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_UART_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRFX_UART_CONFIG_LOG_LEVEL
#define NRFX_UART_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_UART_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_UART_CONFIG_INFO_COLOR
#define NRFX_UART_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_UART_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_UART_CONFIG_DEBUG_COLOR
#define NRFX_UART_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> NRFX_USBD_ENABLED - nrfx_usbd - USBD peripheral driver
//==========================================================
#ifndef NRFX_USBD_ENABLED
#define NRFX_USBD_ENABLED 1
#endif
// <o> NRFX_USBD_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef NRFX_USBD_CONFIG_IRQ_PRIORITY
#define NRFX_USBD_CONFIG_IRQ_PRIORITY 6
#endif

// <o> NRFX_USBD_CONFIG_DMASCHEDULER_MODE  - USBD DMA scheduler working scheme

// <0=> Prioritized access
// <1=> Round Robin

#ifndef NRFX_USBD_CONFIG_DMASCHEDULER_MODE
#define NRFX_USBD_CONFIG_DMASCHEDULER_MODE 0
#endif

// <q> NRFX_USBD_CONFIG_DMASCHEDULER_ISO_BOOST  - Give priority to isochronous transfers

// <i> This option gives priority to isochronous transfers.
// <i> Enabling it assures that isochronous transfers are always processed,
// <i> even if multiple other transfers are pending.
// <i> Isochronous endpoints are prioritized before the usbd_dma_scheduler_algorithm
// <i> function is called, so the option is independent of the algorithm chosen.

#ifndef NRFX_USBD_CONFIG_DMASCHEDULER_ISO_BOOST
#define NRFX_USBD_CONFIG_DMASCHEDULER_ISO_BOOST 1
#endif

// <q> NRFX_USBD_CONFIG_ISO_IN_ZLP  - Respond to an IN token on ISO IN endpoint with ZLP when no data is ready

// <i> If set, ISO IN endpoint will respond to an IN token with ZLP when no data is ready to be sent.
// <i> Else, there will be no response.

#ifndef NRFX_USBD_CONFIG_ISO_IN_ZLP
#define NRFX_USBD_CONFIG_ISO_IN_ZLP 0
#endif

// </e>

// <e> NRF_CLOCK_ENABLED - nrf_drv_clock - CLOCK peripheral driver - legacy layer
//==========================================================
#ifndef NRF_CLOCK_ENABLED
#define NRF_CLOCK_ENABLED 1
#endif
// <o> CLOCK_CONFIG_LF_SRC  - LF Clock Source

// <0=> RC
// <1=> XTAL
// <2=> Synth
// <131073=> External Low Swing
// <196609=> External Full Swing

#ifndef CLOCK_CONFIG_LF_SRC
#define CLOCK_CONFIG_LF_SRC 1
#endif

// <q> CLOCK_CONFIG_LF_CAL_ENABLED  - Calibration enable for LF Clock Source

#ifndef CLOCK_CONFIG_LF_CAL_ENABLED
#define CLOCK_CONFIG_LF_CAL_ENABLED 0
#endif

// <o> CLOCK_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef CLOCK_CONFIG_IRQ_PRIORITY
#define CLOCK_CONFIG_IRQ_PRIORITY 6
#endif

// </e>

// <e> RNG_ENABLED - nrf_drv_rng - RNG peripheral driver - legacy layer
//==========================================================
#ifndef RNG_ENABLED
#define RNG_ENABLED 0
#endif
// <q> RNG_CONFIG_ERROR_CORRECTION  - Error correction

#ifndef RNG_CONFIG_ERROR_CORRECTION
#define RNG_CONFIG_ERROR_CORRECTION 1
#endif

// <o> RNG_CONFIG_POOL_SIZE - Pool size
#ifndef RNG_CONFIG_POOL_SIZE
#define RNG_CONFIG_POOL_SIZE 64
#endif

// <o> RNG_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef RNG_CONFIG_IRQ_PRIORITY
#define RNG_CONFIG_IRQ_PRIORITY 6
#endif

// </e>

// <q> SYSTICK_ENABLED  - nrf_drv_systick - ARM(R) SysTick driver - legacy layer

#ifndef SYSTICK_ENABLED
#define SYSTICK_ENABLED 1
#endif

// <e> TIMER_ENABLED - nrf_drv_timer - TIMER periperal driver - legacy layer
//==========================================================
#ifndef TIMER_ENABLED
#define TIMER_ENABLED 1
#endif
// <o> TIMER_DEFAULT_CONFIG_FREQUENCY  - Timer frequency if in Timer mode

// <0=> 16 MHz
// <1=> 8 MHz
// <2=> 4 MHz
// <3=> 2 MHz
// <4=> 1 MHz
// <5=> 500 kHz
// <6=> 250 kHz
// <7=> 125 kHz
// <8=> 62.5 kHz
// <9=> 31.25 kHz

#ifndef TIMER_DEFAULT_CONFIG_FREQUENCY
#define TIMER_DEFAULT_CONFIG_FREQUENCY 0
#endif

// <o> TIMER_DEFAULT_CONFIG_MODE  - Timer mode or operation

// <0=> Timer
// <1=> Counter

#ifndef TIMER_DEFAULT_CONFIG_MODE
#define TIMER_DEFAULT_CONFIG_MODE 0
#endif

// <o> TIMER_DEFAULT_CONFIG_BIT_WIDTH  - Timer counter bit width

// <0=> 16 bit
// <1=> 8 bit
// <2=> 24 bit
// <3=> 32 bit

#ifndef TIMER_DEFAULT_CONFIG_BIT_WIDTH
#define TIMER_DEFAULT_CONFIG_BIT_WIDTH 0
#endif

// <o> TIMER_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef TIMER_DEFAULT_CONFIG_IRQ_PRIORITY
#define TIMER_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <q> TIMER0_ENABLED  - Enable TIMER0 instance

#ifndef TIMER0_ENABLED
#define TIMER0_ENABLED 1
#endif

// <q> TIMER1_ENABLED  - Enable TIMER1 instance

#ifndef TIMER1_ENABLED
#define TIMER1_ENABLED 1
#endif

// <q> TIMER2_ENABLED  - Enable TIMER2 instance

#ifndef TIMER2_ENABLED
#define TIMER2_ENABLED 0
#endif

// <q> TIMER3_ENABLED  - Enable TIMER3 instance

#ifndef TIMER3_ENABLED
#define TIMER3_ENABLED 0
#endif

// <q> TIMER4_ENABLED  - Enable TIMER4 instance

#ifndef TIMER4_ENABLED
#define TIMER4_ENABLED 0
#endif

// </e>

// <e> UART_ENABLED - nrf_drv_uart - UART/UARTE peripheral driver - legacy layer
//==========================================================
#ifndef UART_ENABLED
#define UART_ENABLED 1
#endif
// <o> UART_DEFAULT_CONFIG_HWFC  - Hardware Flow Control

// <0=> Disabled
// <1=> Enabled

#ifndef UART_DEFAULT_CONFIG_HWFC
#define UART_DEFAULT_CONFIG_HWFC 0
#endif

// <o> UART_DEFAULT_CONFIG_PARITY  - Parity

// <0=> Excluded
// <14=> Included

#ifndef UART_DEFAULT_CONFIG_PARITY
#define UART_DEFAULT_CONFIG_PARITY 0
#endif

// <o> UART_DEFAULT_CONFIG_BAUDRATE  - Default Baudrate

// <323584=> 1200 baud
// <643072=> 2400 baud
// <1290240=> 4800 baud
// <2576384=> 9600 baud
// <3862528=> 14400 baud
// <5152768=> 19200 baud
// <7716864=> 28800 baud
// <10289152=> 38400 baud
// <15400960=> 57600 baud
// <20615168=> 76800 baud
// <30801920=> 115200 baud
// <61865984=> 230400 baud
// <67108864=> 250000 baud
// <121634816=> 460800 baud
// <251658240=> 921600 baud
// <268435456=> 1000000 baud

#ifndef UART_DEFAULT_CONFIG_BAUDRATE
#define UART_DEFAULT_CONFIG_BAUDRATE 30801920
#endif

// <o> UART_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef UART_DEFAULT_CONFIG_IRQ_PRIORITY
#define UART_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <q> UART_EASY_DMA_SUPPORT  - Driver supporting EasyDMA

#ifndef UART_EASY_DMA_SUPPORT
#define UART_EASY_DMA_SUPPORT 1
#endif

// <q> UART_LEGACY_SUPPORT  - Driver supporting Legacy mode

#ifndef UART_LEGACY_SUPPORT
#define UART_LEGACY_SUPPORT 1
#endif

// <e> UART0_ENABLED - Enable UART0 instance
//==========================================================
#ifndef UART0_ENABLED
#define UART0_ENABLED 1
#endif
// <q> UART0_CONFIG_USE_EASY_DMA  - Default setting for using EasyDMA

#ifndef UART0_CONFIG_USE_EASY_DMA
#define UART0_CONFIG_USE_EASY_DMA 1
#endif

// </e>

// <e> UART1_ENABLED - Enable UART1 instance
//==========================================================
#ifndef UART1_ENABLED
#define UART1_ENABLED 0
#endif
// </e>

// </e>

// </h>
//==========================================================

// <h> nRF_Libraries

//==========================================================
// <e> APP_SCHEDULER_ENABLED - app_scheduler - Events scheduler
//==========================================================
#ifndef APP_SCHEDULER_ENABLED
#define APP_SCHEDULER_ENABLED 1
#endif
// <q> APP_SCHEDULER_WITH_PAUSE  - Enabling pause feature

#ifndef APP_SCHEDULER_WITH_PAUSE
#define APP_SCHEDULER_WITH_PAUSE 0
#endif

// <q> APP_SCHEDULER_WITH_PROFILER  - Enabling scheduler profiling

#ifndef APP_SCHEDULER_WITH_PROFILER
#define APP_SCHEDULER_WITH_PROFILER 0
#endif

// </e>

// <e> APP_TIMER_ENABLED - app_timer - Application timer functionality
//==========================================================
#ifndef APP_TIMER_ENABLED
#define APP_TIMER_ENABLED 1
#endif
// <o> APP_TIMER_CONFIG_RTC_FREQUENCY  - Configure RTC prescaler.

// <0=> 32768 Hz
// <1=> 16384 Hz
// <3=> 8192 Hz
// <7=> 4096 Hz
// <15=> 2048 Hz
// <31=> 1024 Hz

#ifndef APP_TIMER_CONFIG_RTC_FREQUENCY
#define APP_TIMER_CONFIG_RTC_FREQUENCY 1
#endif

// <o> APP_TIMER_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef APP_TIMER_CONFIG_IRQ_PRIORITY
#define APP_TIMER_CONFIG_IRQ_PRIORITY 6
#endif

// <o> APP_TIMER_CONFIG_OP_QUEUE_SIZE - Capacity of timer requests queue.
// <i> Size of the queue depends on how many timers are used
// <i> in the system, how often timers are started and overall
// <i> system latency. If queue size is too small app_timer calls
// <i> will fail.

#ifndef APP_TIMER_CONFIG_OP_QUEUE_SIZE
#define APP_TIMER_CONFIG_OP_QUEUE_SIZE 10
#endif

// <q> APP_TIMER_CONFIG_USE_SCHEDULER  - Enable scheduling app_timer events to app_scheduler

#ifndef APP_TIMER_CONFIG_USE_SCHEDULER
#define APP_TIMER_CONFIG_USE_SCHEDULER 0
#endif

// <q> APP_TIMER_KEEPS_RTC_ACTIVE  - Enable RTC always on

// <i> If option is enabled RTC is kept running even if there is no active timers.
// <i> This option can be used when app_timer is used for timestamping.

#ifndef APP_TIMER_KEEPS_RTC_ACTIVE
#define APP_TIMER_KEEPS_RTC_ACTIVE 0
#endif

// <o> APP_TIMER_SAFE_WINDOW_MS - Maximum possible latency (in milliseconds) of handling app_timer event.
// <i> Maximum possible timeout that can be set is reduced by safe window.
// <i> Example: RTC frequency 16384 Hz, maximum possible timeout 1024 seconds - APP_TIMER_SAFE_WINDOW_MS.
// <i> Since RTC is not stopped when processor is halted in debugging session, this value
// <i> must cover it if debugging is needed. It is possible to halt processor for APP_TIMER_SAFE_WINDOW_MS
// <i> without corrupting app_timer behavior.

#ifndef APP_TIMER_SAFE_WINDOW_MS
#define APP_TIMER_SAFE_WINDOW_MS 300000
#endif

// <h> App Timer Legacy configuration - Legacy configuration.

//==========================================================
// <q> APP_TIMER_WITH_PROFILER  - Enable app_timer profiling

#ifndef APP_TIMER_WITH_PROFILER
#define APP_TIMER_WITH_PROFILER 0
#endif

// <q> APP_TIMER_CONFIG_SWI_NUMBER  - Configure SWI instance used.

#ifndef APP_TIMER_CONFIG_SWI_NUMBER
#define APP_TIMER_CONFIG_SWI_NUMBER 0
#endif

// </h>
//==========================================================

// </e>

// <e> NRF_BALLOC_ENABLED - nrf_balloc - Block allocator module
//==========================================================
#ifndef NRF_BALLOC_ENABLED
#define NRF_BALLOC_ENABLED 1
#endif
// <e> NRF_BALLOC_CONFIG_DEBUG_ENABLED - Enables debug mode in the module.
//==========================================================
#ifndef NRF_BALLOC_CONFIG_DEBUG_ENABLED
#define NRF_BALLOC_CONFIG_DEBUG_ENABLED 0
#endif
// <o> NRF_BALLOC_CONFIG_HEAD_GUARD_WORDS - Number of words used as head guard.  <0-255>

#ifndef NRF_BALLOC_CONFIG_HEAD_GUARD_WORDS
#define NRF_BALLOC_CONFIG_HEAD_GUARD_WORDS 1
#endif

// <o> NRF_BALLOC_CONFIG_TAIL_GUARD_WORDS - Number of words used as tail guard.  <0-255>

#ifndef NRF_BALLOC_CONFIG_TAIL_GUARD_WORDS
#define NRF_BALLOC_CONFIG_TAIL_GUARD_WORDS 1
#endif

// <q> NRF_BALLOC_CONFIG_BASIC_CHECKS_ENABLED  - Enables basic checks in this module.

#ifndef NRF_BALLOC_CONFIG_BASIC_CHECKS_ENABLED
#define NRF_BALLOC_CONFIG_BASIC_CHECKS_ENABLED 0
#endif

// <q> NRF_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED  - Enables double memory free check in this module.

#ifndef NRF_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED
#define NRF_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED 0
#endif

// <q> NRF_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED  - Enables free memory corruption check in this module.

#ifndef NRF_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED
#define NRF_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED 0
#endif

// <q> NRF_BALLOC_CLI_CMDS  - Enable CLI commands specific to the module

#ifndef NRF_BALLOC_CLI_CMDS
#define NRF_BALLOC_CLI_CMDS 0
#endif

// </e>

// </e>

// <e> NRF_FSTORAGE_ENABLED - nrf_fstorage - Flash abstraction library
//==========================================================
#ifndef NRF_FSTORAGE_ENABLED
#define NRF_FSTORAGE_ENABLED 1
#endif
// <h> nrf_fstorage - Common settings

// <i> Common settings to all fstorage implementations
//==========================================================
// <q> NRF_FSTORAGE_PARAM_CHECK_DISABLED  - Disable user input validation

// <i> If selected, use ASSERT to validate user input.
// <i> This effectively removes user input validation in production code.
// <i> Recommended setting: OFF, only enable this setting if size is a major concern.

#ifndef NRF_FSTORAGE_PARAM_CHECK_DISABLED
#define NRF_FSTORAGE_PARAM_CHECK_DISABLED 0
#endif

// </h>
//==========================================================

// <h> nrf_fstorage_sd - Implementation using the SoftDevice

// <i> Configuration options for the fstorage implementation using the SoftDevice
//==========================================================
// <o> NRF_FSTORAGE_SD_QUEUE_SIZE - Size of the internal queue of operations
// <i> Increase this value if API calls frequently return the error @ref NRF_ERROR_NO_MEM.

#ifndef NRF_FSTORAGE_SD_QUEUE_SIZE
#define NRF_FSTORAGE_SD_QUEUE_SIZE 4
#endif

// <o> NRF_FSTORAGE_SD_MAX_RETRIES - Maximum number of attempts at executing an operation when the SoftDevice is busy
// <i> Increase this value if events frequently return the @ref NRF_ERROR_TIMEOUT error.
// <i> The SoftDevice might fail to schedule flash access due to high BLE activity.

#ifndef NRF_FSTORAGE_SD_MAX_RETRIES
#define NRF_FSTORAGE_SD_MAX_RETRIES 8
#endif

// <o> NRF_FSTORAGE_SD_MAX_WRITE_SIZE - Maximum number of bytes to be written to flash in a single operation
// <i> This value must be a multiple of four.
// <i> Lowering this value can increase the chances of the SoftDevice being able to execute flash operations in between radio activity.
// <i> This value is bound by the maximum number of bytes that can be written to flash in a single call to @ref sd_flash_write.
// <i> That is 1024 bytes for nRF51 ICs and 4096 bytes for nRF52 ICs.

#ifndef NRF_FSTORAGE_SD_MAX_WRITE_SIZE
#define NRF_FSTORAGE_SD_MAX_WRITE_SIZE 4096
#endif

// </h>
//==========================================================

// </e>

// <q> NRF_MEMOBJ_ENABLED  - nrf_memobj - Linked memory allocator module

#ifndef NRF_MEMOBJ_ENABLED
#define NRF_MEMOBJ_ENABLED 1
#endif

// <e> NRF_PWR_MGMT_ENABLED - nrf_pwr_mgmt - Power management module
//==========================================================
#ifndef NRF_PWR_MGMT_ENABLED
#define NRF_PWR_MGMT_ENABLED 1
#endif
// <e> NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED - Enables pin debug in the module.

// <i> Selected pin will be set when CPU is in sleep mode.
//==========================================================
#ifndef NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED
#define NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED 0
#endif
// <o> NRF_PWR_MGMT_SLEEP_DEBUG_PIN  - Pin number

// <0=> 0 (P0.0)
// <1=> 1 (P0.1)
// <2=> 2 (P0.2)
// <3=> 3 (P0.3)
// <4=> 4 (P0.4)
// <5=> 5 (P0.5)
// <6=> 6 (P0.6)
// <7=> 7 (P0.7)
// <8=> 8 (P0.8)
// <9=> 9 (P0.9)
// <10=> 10 (P0.10)
// <11=> 11 (P0.11)
// <12=> 12 (P0.12)
// <13=> 13 (P0.13)
// <14=> 14 (P0.14)
// <15=> 15 (P0.15)
// <16=> 16 (P0.16)
// <17=> 17 (P0.17)
// <18=> 18 (P0.18)
// <19=> 19 (P0.19)
// <20=> 20 (P0.20)
// <21=> 21 (P0.21)
// <22=> 22 (P0.22)
// <23=> 23 (P0.23)
// <24=> 24 (P0.24)
// <25=> 25 (P0.25)
// <26=> 26 (P0.26)
// <27=> 27 (P0.27)
// <28=> 28 (P0.28)
// <29=> 29 (P0.29)
// <30=> 30 (P0.30)
// <31=> 31 (P0.31)
// <32=> 32 (P1.0)
// <33=> 33 (P1.1)
// <34=> 34 (P1.2)
// <35=> 35 (P1.3)
// <36=> 36 (P1.4)
// <37=> 37 (P1.5)
// <38=> 38 (P1.6)
// <39=> 39 (P1.7)
// <40=> 40 (P1.8)
// <41=> 41 (P1.9)
// <42=> 42 (P1.10)
// <43=> 43 (P1.11)
// <44=> 44 (P1.12)
// <45=> 45 (P1.13)
// <46=> 46 (P1.14)
// <47=> 47 (P1.15)
// <4294967295=> Not connected

#ifndef NRF_PWR_MGMT_SLEEP_DEBUG_PIN
#define NRF_PWR_MGMT_SLEEP_DEBUG_PIN 31
#endif

// </e>

// <q> NRF_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED  - Enables CPU usage monitor.

// <i> Module will trace percentage of CPU usage in one second intervals.

#ifndef NRF_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED
#define NRF_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED 0
#endif

// <e> NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED - Enable standby timeout.
//==========================================================
#ifndef NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED
#define NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED 0
#endif
// <o> NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S - Standby timeout (in seconds).
// <i> Shutdown procedure will begin no earlier than after this number of seconds.

#ifndef NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S
#define NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S 3
#endif

// </e>

// <q> NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED  - Enables FPU event cleaning.

#ifndef NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED
#define NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED 1
#endif

// <q> NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY  - Blocked shutdown procedure will be retried every second.

#ifndef NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY
#define NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY 0
#endif

// <q> NRF_PWR_MGMT_CONFIG_USE_SCHEDULER  - Module will use @ref app_scheduler.

#ifndef NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
#define NRF_PWR_MGMT_CONFIG_USE_SCHEDULER 0
#endif

// <o> NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT - The number of priorities for module handlers.
// <i> The number of stages of the shutdown process.

#ifndef NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT
#define NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT 3
#endif

// </e>

// <e> NRF_QUEUE_ENABLED - nrf_queue - Queue module
//==========================================================
#ifndef NRF_QUEUE_ENABLED
#define NRF_QUEUE_ENABLED 1
#endif
// <q> NRF_QUEUE_CLI_CMDS  - Enable CLI commands specific to the module

#ifndef NRF_QUEUE_CLI_CMDS
#define NRF_QUEUE_CLI_CMDS 1
#endif

// </e>

// <q> NRF_SECTION_ITER_ENABLED  - nrf_section_iter - Section iterator

#ifndef NRF_SECTION_ITER_ENABLED
#define NRF_SECTION_ITER_ENABLED 1
#endif

// <q> NRF_SORTLIST_ENABLED  - nrf_sortlist - Sorted list

#ifndef NRF_SORTLIST_ENABLED
#define NRF_SORTLIST_ENABLED 1
#endif

// <q> NRF_STRERROR_ENABLED  - nrf_strerror - Library for converting error code to string.

#ifndef NRF_STRERROR_ENABLED
#define NRF_STRERROR_ENABLED 1
#endif

// <h> app_button - buttons handling module

//==========================================================
// <q> BUTTON_ENABLED  - Enables Button module

#ifndef BUTTON_ENABLED
#define BUTTON_ENABLED 1
#endif

// <q> BUTTON_HIGH_ACCURACY_ENABLED  - Enables GPIOTE high accuracy for buttons

#ifndef BUTTON_HIGH_ACCURACY_ENABLED
#define BUTTON_HIGH_ACCURACY_ENABLED 0
#endif

// </h>
//==========================================================

// <h> nrf_fprintf - fprintf function.

//==========================================================
// <q> NRF_FPRINTF_ENABLED  - Enable/disable fprintf module.

#ifndef NRF_FPRINTF_ENABLED
#define NRF_FPRINTF_ENABLED 1
#endif

// <q> NRF_FPRINTF_FLAG_AUTOMATIC_CR_ON_LF_ENABLED  - For each printed LF, function will add CR.

#ifndef NRF_FPRINTF_FLAG_AUTOMATIC_CR_ON_LF_ENABLED
#define NRF_FPRINTF_FLAG_AUTOMATIC_CR_ON_LF_ENABLED 1
#endif

// <q> NRF_FPRINTF_DOUBLE_ENABLED  - Enable IEEE-754 double precision formatting.

#ifndef NRF_FPRINTF_DOUBLE_ENABLED
#define NRF_FPRINTF_DOUBLE_ENABLED 0
#endif

// </h>
//==========================================================

// </h>
//==========================================================

// </h>
//==========================================================

// <h> nRF_Log

//==========================================================
// <e> NRF_LOG_BACKEND_FLASH_ENABLED - nrf_log_backend_flash - Log flash backend
//==========================================================
#ifndef NRF_LOG_BACKEND_FLASH_ENABLED
#define NRF_LOG_BACKEND_FLASH_ENABLED 1
#endif
// <q> NRF_LOG_BACKEND_FLASH_CLI_CMDS  - Enable CLI commands for this module.

#ifndef NRF_LOG_BACKEND_FLASH_CLI_CMDS
#define NRF_LOG_BACKEND_FLASH_CLI_CMDS 0
#endif

// <e> NRF_LOG_BACKEND_FLASHLOG_ENABLED - Enable flashlog backend.
//==========================================================
#ifndef NRF_LOG_BACKEND_FLASHLOG_ENABLED
#define NRF_LOG_BACKEND_FLASHLOG_ENABLED 0
#endif
// <o> NRF_LOG_BACKEND_FLASHLOG_QUEUE_SIZE - Logger messages queue size.
// <i> Queue holds log messages pending to be written to flash.
// <i> Note that the queue holds logger messages and thus the queue size determines
// <i> increasing the pool of logger messages (see log message pool configuration).

#ifndef NRF_LOG_BACKEND_FLASHLOG_QUEUE_SIZE
#define NRF_LOG_BACKEND_FLASHLOG_QUEUE_SIZE 8
#endif

// </e>

// <e> NRF_LOG_BACKEND_CRASHLOG_ENABLED - Enable crashlog backend.
//==========================================================
#ifndef NRF_LOG_BACKEND_CRASHLOG_ENABLED
#define NRF_LOG_BACKEND_CRASHLOG_ENABLED 1
#endif
// <o> NRF_LOG_BACKEND_CRASHLOG_FIFO_SIZE - Number of log messages held to be flushed in panic.
// <i> Crashlog FIFO always keeps a defined number of the most
// <i> recent logs (severity level is set on runtime).
// <i> Note that the FIFO holds logger messages and thus the FIFO size determines
// <i> increasing the pool of logger messages (see log message pool configuration).

#ifndef NRF_LOG_BACKEND_CRASHLOG_FIFO_SIZE
#define NRF_LOG_BACKEND_CRASHLOG_FIFO_SIZE 8
#endif

// </e>

// <o> NRF_LOG_BACKEND_FLASH_SER_BUFFER_SIZE - Size of the buffer used for serialize log message.
// <i> Message is trimmed if it is longer. It may happen in case of
// <i> hexdump message. Buffer size must be multiple of 4.

#ifndef NRF_LOG_BACKEND_FLASH_SER_BUFFER_SIZE
#define NRF_LOG_BACKEND_FLASH_SER_BUFFER_SIZE 64
#endif

// <h> Flash log location - Configuration of flash area used for storing the logs.

//==========================================================
// <o> NRF_LOG_BACKEND_FLASH_START_PAGE - Starting page.
// <i> If 0, then pages directly after the application are used.

#ifndef NRF_LOG_BACKEND_FLASH_START_PAGE
#define NRF_LOG_BACKEND_FLASH_START_PAGE 0
#endif

// <o> NRF_LOG_BACKEND_PAGES - Number of pages.
#ifndef NRF_LOG_BACKEND_PAGES
#define NRF_LOG_BACKEND_PAGES 1
#endif

// </h>
//==========================================================

// </e>

// <e> NRF_LOG_BACKEND_RTT_ENABLED - nrf_log_backend_rtt - Log RTT backend
//==========================================================
#ifndef NRF_LOG_BACKEND_RTT_ENABLED
#define NRF_LOG_BACKEND_RTT_ENABLED 0
#endif
// <o> NRF_LOG_BACKEND_RTT_TEMP_BUFFER_SIZE - Size of buffer for partially processed strings.
// <i> Size of the buffer is a trade-off between RAM usage and processing.
// <i> if buffer is smaller then strings will often be fragmented.
// <i> It is recommended to use size which will fit typical log and only the
// <i> longer one will be fragmented.

#ifndef NRF_LOG_BACKEND_RTT_TEMP_BUFFER_SIZE
#define NRF_LOG_BACKEND_RTT_TEMP_BUFFER_SIZE 64
#endif

// <o> NRF_LOG_BACKEND_RTT_TX_RETRY_DELAY_MS - Period before retrying writing to RTT
#ifndef NRF_LOG_BACKEND_RTT_TX_RETRY_DELAY_MS
#define NRF_LOG_BACKEND_RTT_TX_RETRY_DELAY_MS 1
#endif

// <o> NRF_LOG_BACKEND_RTT_TX_RETRY_CNT - Writing to RTT retries.
// <i> If RTT fails to accept any new data after retries
// <i> module assumes that host is not active and on next
// <i> request it will perform only one write attempt.
// <i> On successful writing, module assumes that host is active
// <i> and scheme with retry is applied again.

#ifndef NRF_LOG_BACKEND_RTT_TX_RETRY_CNT
#define NRF_LOG_BACKEND_RTT_TX_RETRY_CNT 3
#endif

// </e>

// <h> nRF_Log

//==========================================================
// <e> NRF_LOG_BACKEND_UART_ENABLED - nrf_log_backend_uart - Log UART backend
//==========================================================
#ifndef NRF_LOG_BACKEND_UART_ENABLED
#define NRF_LOG_BACKEND_UART_ENABLED 0
#endif
// <o> NRF_LOG_BACKEND_UART_TX_PIN - UART TX pin
#ifndef NRF_LOG_BACKEND_UART_TX_PIN
#define NRF_LOG_BACKEND_UART_TX_PIN 6
#endif

// <o> NRF_LOG_BACKEND_UART_BAUDRATE  - Default Baudrate

// <323584=> 1200 baud
// <643072=> 2400 baud
// <1290240=> 4800 baud
// <2576384=> 9600 baud
// <3862528=> 14400 baud
// <5152768=> 19200 baud
// <7716864=> 28800 baud
// <10289152=> 38400 baud
// <15400960=> 57600 baud
// <20615168=> 76800 baud
// <30801920=> 115200 baud
// <61865984=> 230400 baud
// <67108864=> 250000 baud
// <121634816=> 460800 baud
// <251658240=> 921600 baud
// <268435456=> 1000000 baud

#ifndef NRF_LOG_BACKEND_UART_BAUDRATE
#define NRF_LOG_BACKEND_UART_BAUDRATE 30801920
#endif

// <o> NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE - Size of buffer for partially processed strings.
// <i> Size of the buffer is a trade-off between RAM usage and processing.
// <i> if buffer is smaller then strings will often be fragmented.
// <i> It is recommended to use size which will fit typical log and only the
// <i> longer one will be fragmented.

#ifndef NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE
#define NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE 64
#endif

// </e>

// <e> NRF_LOG_ENABLED - nrf_log - Logger
//==========================================================
#ifndef NRF_LOG_ENABLED
#define NRF_LOG_ENABLED 1
#endif
// <h> Log message pool - Configuration of log message pool

//==========================================================
// <o> NRF_LOG_MSGPOOL_ELEMENT_SIZE - Size of a single element in the pool of memory objects.
// <i> If a small value is set, then performance of logs processing
// <i> is degraded because data is fragmented. Bigger value impacts
// <i> RAM memory utilization. The size is set to fit a message with
// <i> a timestamp and up to 2 arguments in a single memory object.

#ifndef NRF_LOG_MSGPOOL_ELEMENT_SIZE
#define NRF_LOG_MSGPOOL_ELEMENT_SIZE 20
#endif

// <o> NRF_LOG_MSGPOOL_ELEMENT_COUNT - Number of elements in the pool of memory objects
// <i> If a small value is set, then it may lead to a deadlock
// <i> in certain cases if backend has high latency and holds
// <i> multiple messages for long time. Bigger value impacts
// <i> RAM memory usage.

#ifndef NRF_LOG_MSGPOOL_ELEMENT_COUNT
#define NRF_LOG_MSGPOOL_ELEMENT_COUNT 8
#endif

// </h>
//==========================================================

// <q> NRF_LOG_ALLOW_OVERFLOW  - Configures behavior when circular buffer is full.

// <i> If set then oldest logs are overwritten. Otherwise a
// <i> marker is injected informing about overflow.

#ifndef NRF_LOG_ALLOW_OVERFLOW
#define NRF_LOG_ALLOW_OVERFLOW 1
#endif

// <o> NRF_LOG_BUFSIZE  - Size of the buffer for storing logs (in bytes).

// <i> Must be power of 2 and multiple of 4.
// <i> If NRF_LOG_DEFERRED = 0 then buffer size can be reduced to minimum.
// <128=> 128
// <256=> 256
// <512=> 512
// <1024=> 1024
// <2048=> 2048
// <4096=> 4096
// <8192=> 8192
// <16384=> 16384

#ifndef NRF_LOG_BUFSIZE
#define NRF_LOG_BUFSIZE 16384
#endif

// <q> NRF_LOG_CLI_CMDS  - Enable CLI commands for the module.

#ifndef NRF_LOG_CLI_CMDS
#define NRF_LOG_CLI_CMDS 1
#endif

// <o> NRF_LOG_DEFAULT_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_LOG_DEFAULT_LEVEL
#define NRF_LOG_DEFAULT_LEVEL 3
#endif

// <q> NRF_LOG_DEFERRED  - Enable deffered logger.

// <i> Log data is buffered and can be processed in idle.

#ifndef NRF_LOG_DEFERRED
#define NRF_LOG_DEFERRED 1
#endif

// <q> NRF_LOG_FILTERS_ENABLED  - Enable dynamic filtering of logs.

#ifndef NRF_LOG_FILTERS_ENABLED
#define NRF_LOG_FILTERS_ENABLED 1
#endif

// <q> NRF_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED  - Enable use of critical region for non deffered mode when flushing logs.

// <i> When enabled NRF_LOG_FLUSH is called from critical section when non deffered mode is used.
// <i> Log output will never be corrupted as access to the log backend is exclusive
// <i> but system will spend significant amount of time in critical section

#ifndef NRF_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED
#define NRF_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED 0
#endif

// <o> NRF_LOG_STR_PUSH_BUFFER_SIZE  - Size of the buffer dedicated for strings stored using @ref NRF_LOG_PUSH.

// <16=> 16
// <32=> 32
// <64=> 64
// <128=> 128
// <256=> 256
// <512=> 512
// <1024=> 1024

#ifndef NRF_LOG_STR_PUSH_BUFFER_SIZE
#define NRF_LOG_STR_PUSH_BUFFER_SIZE 1024
#endif

// <e> NRF_LOG_USES_COLORS - If enabled then ANSI escape code for colors is prefixed to every string
//==========================================================
#ifndef NRF_LOG_USES_COLORS
#define NRF_LOG_USES_COLORS 1
#endif
// <o> NRF_LOG_COLOR_DEFAULT  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_LOG_COLOR_DEFAULT
#define NRF_LOG_COLOR_DEFAULT 0
#endif

// <o> NRF_LOG_ERROR_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_LOG_ERROR_COLOR
#define NRF_LOG_ERROR_COLOR 2
#endif

// <o> NRF_LOG_WARNING_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_LOG_WARNING_COLOR
#define NRF_LOG_WARNING_COLOR 4
#endif

// </e>

// <e> NRF_LOG_USES_TIMESTAMP - Enable timestamping

// <i> Function for getting the timestamp is provided by the user
//==========================================================
#ifndef NRF_LOG_USES_TIMESTAMP
#define NRF_LOG_USES_TIMESTAMP 0
#endif
// <o> NRF_LOG_TIMESTAMP_DEFAULT_FREQUENCY - Default frequency of the timestamp (in Hz) or 0 to use app_timer frequency.
#ifndef NRF_LOG_TIMESTAMP_DEFAULT_FREQUENCY
#define NRF_LOG_TIMESTAMP_DEFAULT_FREQUENCY 0
#endif

// </e>

// <h> nrf_log module configuration

//==========================================================
// <h> nrf_log in nRF_Core

//==========================================================
// <e> NRF_MPU_LIB_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_MPU_LIB_CONFIG_LOG_ENABLED
#define NRF_MPU_LIB_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_MPU_LIB_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_MPU_LIB_CONFIG_LOG_LEVEL
#define NRF_MPU_LIB_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_MPU_LIB_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_MPU_LIB_CONFIG_INFO_COLOR
#define NRF_MPU_LIB_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_MPU_LIB_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_MPU_LIB_CONFIG_DEBUG_COLOR
#define NRF_MPU_LIB_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_STACK_GUARD_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_STACK_GUARD_CONFIG_LOG_ENABLED
#define NRF_STACK_GUARD_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_STACK_GUARD_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_STACK_GUARD_CONFIG_LOG_LEVEL
#define NRF_STACK_GUARD_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_STACK_GUARD_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_STACK_GUARD_CONFIG_INFO_COLOR
#define NRF_STACK_GUARD_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_STACK_GUARD_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_STACK_GUARD_CONFIG_DEBUG_COLOR
#define NRF_STACK_GUARD_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> TASK_MANAGER_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef TASK_MANAGER_CONFIG_LOG_ENABLED
#define TASK_MANAGER_CONFIG_LOG_ENABLED 0
#endif
// <o> TASK_MANAGER_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef TASK_MANAGER_CONFIG_LOG_LEVEL
#define TASK_MANAGER_CONFIG_LOG_LEVEL 3
#endif

// <o> TASK_MANAGER_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef TASK_MANAGER_CONFIG_INFO_COLOR
#define TASK_MANAGER_CONFIG_INFO_COLOR 0
#endif

// <o> TASK_MANAGER_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef TASK_MANAGER_CONFIG_DEBUG_COLOR
#define TASK_MANAGER_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </h>
//==========================================================

// <h> nrf_log in nRF_Drivers

//==========================================================
// <e> CLOCK_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef CLOCK_CONFIG_LOG_ENABLED
#define CLOCK_CONFIG_LOG_ENABLED 0
#endif
// <o> CLOCK_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef CLOCK_CONFIG_LOG_LEVEL
#define CLOCK_CONFIG_LOG_LEVEL 3
#endif

// <o> CLOCK_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef CLOCK_CONFIG_INFO_COLOR
#define CLOCK_CONFIG_INFO_COLOR 0
#endif

// <o> CLOCK_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef CLOCK_CONFIG_DEBUG_COLOR
#define CLOCK_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> COMP_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef COMP_CONFIG_LOG_ENABLED
#define COMP_CONFIG_LOG_ENABLED 0
#endif
// <o> COMP_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef COMP_CONFIG_LOG_LEVEL
#define COMP_CONFIG_LOG_LEVEL 3
#endif

// <o> COMP_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef COMP_CONFIG_INFO_COLOR
#define COMP_CONFIG_INFO_COLOR 0
#endif

// <o> COMP_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef COMP_CONFIG_DEBUG_COLOR
#define COMP_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> GPIOTE_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef GPIOTE_CONFIG_LOG_ENABLED
#define GPIOTE_CONFIG_LOG_ENABLED 0
#endif
// <o> GPIOTE_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef GPIOTE_CONFIG_LOG_LEVEL
#define GPIOTE_CONFIG_LOG_LEVEL 3
#endif

// <o> GPIOTE_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef GPIOTE_CONFIG_INFO_COLOR
#define GPIOTE_CONFIG_INFO_COLOR 0
#endif

// <o> GPIOTE_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef GPIOTE_CONFIG_DEBUG_COLOR
#define GPIOTE_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> LPCOMP_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef LPCOMP_CONFIG_LOG_ENABLED
#define LPCOMP_CONFIG_LOG_ENABLED 0
#endif
// <o> LPCOMP_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef LPCOMP_CONFIG_LOG_LEVEL
#define LPCOMP_CONFIG_LOG_LEVEL 3
#endif

// <o> LPCOMP_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef LPCOMP_CONFIG_INFO_COLOR
#define LPCOMP_CONFIG_INFO_COLOR 0
#endif

// <o> LPCOMP_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef LPCOMP_CONFIG_DEBUG_COLOR
#define LPCOMP_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> MAX3421E_HOST_CONFIG_LOG_ENABLED - Enable logging in the module
//==========================================================
#ifndef MAX3421E_HOST_CONFIG_LOG_ENABLED
#define MAX3421E_HOST_CONFIG_LOG_ENABLED 0
#endif
// <o> MAX3421E_HOST_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef MAX3421E_HOST_CONFIG_LOG_LEVEL
#define MAX3421E_HOST_CONFIG_LOG_LEVEL 3
#endif

// <o> MAX3421E_HOST_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef MAX3421E_HOST_CONFIG_INFO_COLOR
#define MAX3421E_HOST_CONFIG_INFO_COLOR 0
#endif

// <o> MAX3421E_HOST_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef MAX3421E_HOST_CONFIG_DEBUG_COLOR
#define MAX3421E_HOST_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRFX_USBD_CONFIG_LOG_ENABLED - Enable logging in the module
//==========================================================
#ifndef NRFX_USBD_CONFIG_LOG_ENABLED
#define NRFX_USBD_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_USBD_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRFX_USBD_CONFIG_LOG_LEVEL
#define NRFX_USBD_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_USBD_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_USBD_CONFIG_INFO_COLOR
#define NRFX_USBD_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_USBD_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_USBD_CONFIG_DEBUG_COLOR
#define NRFX_USBD_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> PDM_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef PDM_CONFIG_LOG_ENABLED
#define PDM_CONFIG_LOG_ENABLED 0
#endif
// <o> PDM_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef PDM_CONFIG_LOG_LEVEL
#define PDM_CONFIG_LOG_LEVEL 3
#endif

// <o> PDM_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef PDM_CONFIG_INFO_COLOR
#define PDM_CONFIG_INFO_COLOR 0
#endif

// <o> PDM_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef PDM_CONFIG_DEBUG_COLOR
#define PDM_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> PPI_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef PPI_CONFIG_LOG_ENABLED
#define PPI_CONFIG_LOG_ENABLED 0
#endif
// <o> PPI_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef PPI_CONFIG_LOG_LEVEL
#define PPI_CONFIG_LOG_LEVEL 3
#endif

// <o> PPI_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef PPI_CONFIG_INFO_COLOR
#define PPI_CONFIG_INFO_COLOR 0
#endif

// <o> PPI_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef PPI_CONFIG_DEBUG_COLOR
#define PPI_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> PWM_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef PWM_CONFIG_LOG_ENABLED
#define PWM_CONFIG_LOG_ENABLED 0
#endif
// <o> PWM_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef PWM_CONFIG_LOG_LEVEL
#define PWM_CONFIG_LOG_LEVEL 3
#endif

// <o> PWM_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef PWM_CONFIG_INFO_COLOR
#define PWM_CONFIG_INFO_COLOR 0
#endif

// <o> PWM_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef PWM_CONFIG_DEBUG_COLOR
#define PWM_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> QDEC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef QDEC_CONFIG_LOG_ENABLED
#define QDEC_CONFIG_LOG_ENABLED 0
#endif
// <o> QDEC_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef QDEC_CONFIG_LOG_LEVEL
#define QDEC_CONFIG_LOG_LEVEL 3
#endif

// <o> QDEC_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef QDEC_CONFIG_INFO_COLOR
#define QDEC_CONFIG_INFO_COLOR 0
#endif

// <o> QDEC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef QDEC_CONFIG_DEBUG_COLOR
#define QDEC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> RNG_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef RNG_CONFIG_LOG_ENABLED
#define RNG_CONFIG_LOG_ENABLED 0
#endif
// <o> RNG_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef RNG_CONFIG_LOG_LEVEL
#define RNG_CONFIG_LOG_LEVEL 3
#endif

// <o> RNG_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef RNG_CONFIG_INFO_COLOR
#define RNG_CONFIG_INFO_COLOR 0
#endif

// <o> RNG_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef RNG_CONFIG_DEBUG_COLOR
#define RNG_CONFIG_DEBUG_COLOR 0
#endif

// <q> RNG_CONFIG_RANDOM_NUMBER_LOG_ENABLED  - Enables logging of random numbers.

#ifndef RNG_CONFIG_RANDOM_NUMBER_LOG_ENABLED
#define RNG_CONFIG_RANDOM_NUMBER_LOG_ENABLED 0
#endif

// </e>

// <e> RTC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef RTC_CONFIG_LOG_ENABLED
#define RTC_CONFIG_LOG_ENABLED 0
#endif
// <o> RTC_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef RTC_CONFIG_LOG_LEVEL
#define RTC_CONFIG_LOG_LEVEL 3
#endif

// <o> RTC_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef RTC_CONFIG_INFO_COLOR
#define RTC_CONFIG_INFO_COLOR 0
#endif

// <o> RTC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef RTC_CONFIG_DEBUG_COLOR
#define RTC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRFX_RTC_ENABLED - nrfx_rtc - RTC peripheral driver
//==========================================================
#ifndef NRFX_RTC_ENABLED
#define NRFX_RTC_ENABLED 0
#endif
// <q> NRFX_RTC0_ENABLED  - Enable RTC0 instance

#ifndef NRFX_RTC0_ENABLED
#define NRFX_RTC0_ENABLED 0
#endif

// <q> NRFX_RTC1_ENABLED  - Enable RTC1 instance

#ifndef NRFX_RTC1_ENABLED
#define NRFX_RTC1_ENABLED 0
#endif

// <q> NRFX_RTC2_ENABLED  - Enable RTC2 instance

#ifndef NRFX_RTC2_ENABLED
#define NRFX_RTC2_ENABLED 0
#endif

// <o> NRFX_RTC_MAXIMUM_LATENCY_US - Maximum possible time[us] in highest priority interrupt
#ifndef NRFX_RTC_MAXIMUM_LATENCY_US
#define NRFX_RTC_MAXIMUM_LATENCY_US 2000
#endif

// <o> NRFX_RTC_DEFAULT_CONFIG_FREQUENCY - Frequency  <16-32768>

#ifndef NRFX_RTC_DEFAULT_CONFIG_FREQUENCY
#define NRFX_RTC_DEFAULT_CONFIG_FREQUENCY 32768
#endif

// <q> NRFX_RTC_DEFAULT_CONFIG_RELIABLE  - Ensures safe compare event triggering

#ifndef NRFX_RTC_DEFAULT_CONFIG_RELIABLE
#define NRFX_RTC_DEFAULT_CONFIG_RELIABLE 0
#endif

// <o> NRFX_RTC_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority

// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef NRFX_RTC_DEFAULT_CONFIG_IRQ_PRIORITY
#define NRFX_RTC_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <e> NRFX_RTC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRFX_RTC_CONFIG_LOG_ENABLED
#define NRFX_RTC_CONFIG_LOG_ENABLED 0
#endif
// <o> NRFX_RTC_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRFX_RTC_CONFIG_LOG_LEVEL
#define NRFX_RTC_CONFIG_LOG_LEVEL 3
#endif

// <o> NRFX_RTC_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_RTC_CONFIG_INFO_COLOR
#define NRFX_RTC_CONFIG_INFO_COLOR 0
#endif

// <o> NRFX_RTC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRFX_RTC_CONFIG_DEBUG_COLOR
#define NRFX_RTC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <e> SAADC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef SAADC_CONFIG_LOG_ENABLED
#define SAADC_CONFIG_LOG_ENABLED 0
#endif
// <o> SAADC_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef SAADC_CONFIG_LOG_LEVEL
#define SAADC_CONFIG_LOG_LEVEL 3
#endif

// <o> SAADC_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef SAADC_CONFIG_INFO_COLOR
#define SAADC_CONFIG_INFO_COLOR 0
#endif

// <o> SAADC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef SAADC_CONFIG_DEBUG_COLOR
#define SAADC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> SPIS_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef SPIS_CONFIG_LOG_ENABLED
#define SPIS_CONFIG_LOG_ENABLED 0
#endif
// <o> SPIS_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef SPIS_CONFIG_LOG_LEVEL
#define SPIS_CONFIG_LOG_LEVEL 3
#endif

// <o> SPIS_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef SPIS_CONFIG_INFO_COLOR
#define SPIS_CONFIG_INFO_COLOR 0
#endif

// <o> SPIS_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef SPIS_CONFIG_DEBUG_COLOR
#define SPIS_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> SPI_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef SPI_CONFIG_LOG_ENABLED
#define SPI_CONFIG_LOG_ENABLED 0
#endif
// <o> SPI_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef SPI_CONFIG_LOG_LEVEL
#define SPI_CONFIG_LOG_LEVEL 3
#endif

// <o> SPI_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef SPI_CONFIG_INFO_COLOR
#define SPI_CONFIG_INFO_COLOR 0
#endif

// <o> SPI_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef SPI_CONFIG_DEBUG_COLOR
#define SPI_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> TIMER_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef TIMER_CONFIG_LOG_ENABLED
#define TIMER_CONFIG_LOG_ENABLED 0
#endif
// <o> TIMER_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef TIMER_CONFIG_LOG_LEVEL
#define TIMER_CONFIG_LOG_LEVEL 3
#endif

// <o> TIMER_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef TIMER_CONFIG_INFO_COLOR
#define TIMER_CONFIG_INFO_COLOR 0
#endif

// <o> TIMER_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef TIMER_CONFIG_DEBUG_COLOR
#define TIMER_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> TWIS_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef TWIS_CONFIG_LOG_ENABLED
#define TWIS_CONFIG_LOG_ENABLED 0
#endif
// <o> TWIS_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef TWIS_CONFIG_LOG_LEVEL
#define TWIS_CONFIG_LOG_LEVEL 3
#endif

// <o> TWIS_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef TWIS_CONFIG_INFO_COLOR
#define TWIS_CONFIG_INFO_COLOR 0
#endif

// <o> TWIS_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef TWIS_CONFIG_DEBUG_COLOR
#define TWIS_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> TWI_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef TWI_CONFIG_LOG_ENABLED
#define TWI_CONFIG_LOG_ENABLED 0
#endif
// <o> TWI_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef TWI_CONFIG_LOG_LEVEL
#define TWI_CONFIG_LOG_LEVEL 3
#endif

// <o> TWI_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef TWI_CONFIG_INFO_COLOR
#define TWI_CONFIG_INFO_COLOR 0
#endif

// <o> TWI_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef TWI_CONFIG_DEBUG_COLOR
#define TWI_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> UART_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef UART_CONFIG_LOG_ENABLED
#define UART_CONFIG_LOG_ENABLED 0
#endif
// <o> UART_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef UART_CONFIG_LOG_LEVEL
#define UART_CONFIG_LOG_LEVEL 3
#endif

// <o> UART_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef UART_CONFIG_INFO_COLOR
#define UART_CONFIG_INFO_COLOR 0
#endif

// <o> UART_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef UART_CONFIG_DEBUG_COLOR
#define UART_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> USBD_CONFIG_LOG_ENABLED - Enable logging in the module
//==========================================================
#ifndef USBD_CONFIG_LOG_ENABLED
#define USBD_CONFIG_LOG_ENABLED 0
#endif
// <o> USBD_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef USBD_CONFIG_LOG_LEVEL
#define USBD_CONFIG_LOG_LEVEL 3
#endif

// <o> USBD_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef USBD_CONFIG_INFO_COLOR
#define USBD_CONFIG_INFO_COLOR 0
#endif

// <o> USBD_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef USBD_CONFIG_DEBUG_COLOR
#define USBD_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> WDT_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef WDT_CONFIG_LOG_ENABLED
#define WDT_CONFIG_LOG_ENABLED 0
#endif
// <o> WDT_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef WDT_CONFIG_LOG_LEVEL
#define WDT_CONFIG_LOG_LEVEL 3
#endif

// <o> WDT_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef WDT_CONFIG_INFO_COLOR
#define WDT_CONFIG_INFO_COLOR 0
#endif

// <o> WDT_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef WDT_CONFIG_DEBUG_COLOR
#define WDT_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </h>
//==========================================================

// <h> nrf_log in nRF_Libraries

//==========================================================
// <e> APP_BUTTON_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_BUTTON_CONFIG_LOG_ENABLED
#define APP_BUTTON_CONFIG_LOG_ENABLED 0
#endif
// <o> APP_BUTTON_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_BUTTON_CONFIG_LOG_LEVEL
#define APP_BUTTON_CONFIG_LOG_LEVEL 3
#endif

// <o> APP_BUTTON_CONFIG_INITIAL_LOG_LEVEL  - Initial severity level if dynamic filtering is enabled.

// <i> If module generates a lot of logs, initial log level can
// <i> be decreased to prevent flooding. Severity level can be
// <i> increased on instance basis.
// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_BUTTON_CONFIG_INITIAL_LOG_LEVEL
#define APP_BUTTON_CONFIG_INITIAL_LOG_LEVEL 3
#endif

// <o> APP_BUTTON_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_BUTTON_CONFIG_INFO_COLOR
#define APP_BUTTON_CONFIG_INFO_COLOR 0
#endif

// <o> APP_BUTTON_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_BUTTON_CONFIG_DEBUG_COLOR
#define APP_BUTTON_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> APP_TIMER_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_TIMER_CONFIG_LOG_ENABLED
#define APP_TIMER_CONFIG_LOG_ENABLED 0
#endif
// <o> APP_TIMER_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_TIMER_CONFIG_LOG_LEVEL
#define APP_TIMER_CONFIG_LOG_LEVEL 3
#endif

// <o> APP_TIMER_CONFIG_INITIAL_LOG_LEVEL  - Initial severity level if dynamic filtering is enabled.

// <i> If module generates a lot of logs, initial log level can
// <i> be decreased to prevent flooding. Severity level can be
// <i> increased on instance basis.
// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_TIMER_CONFIG_INITIAL_LOG_LEVEL
#define APP_TIMER_CONFIG_INITIAL_LOG_LEVEL 3
#endif

// <o> APP_TIMER_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_TIMER_CONFIG_INFO_COLOR
#define APP_TIMER_CONFIG_INFO_COLOR 0
#endif

// <o> APP_TIMER_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_TIMER_CONFIG_DEBUG_COLOR
#define APP_TIMER_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> APP_USBD_CDC_ACM_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_USBD_CDC_ACM_CONFIG_LOG_ENABLED
#define APP_USBD_CDC_ACM_CONFIG_LOG_ENABLED 0
#endif
// <o> APP_USBD_CDC_ACM_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_USBD_CDC_ACM_CONFIG_LOG_LEVEL
#define APP_USBD_CDC_ACM_CONFIG_LOG_LEVEL 3
#endif

// <o> APP_USBD_CDC_ACM_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_USBD_CDC_ACM_CONFIG_INFO_COLOR
#define APP_USBD_CDC_ACM_CONFIG_INFO_COLOR 0
#endif

// <o> APP_USBD_CDC_ACM_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_USBD_CDC_ACM_CONFIG_DEBUG_COLOR
#define APP_USBD_CDC_ACM_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> APP_USBD_CONFIG_LOG_ENABLED - Enable logging in the module.
//==========================================================
#ifndef APP_USBD_CONFIG_LOG_ENABLED
#define APP_USBD_CONFIG_LOG_ENABLED 0
#endif
// <o> APP_USBD_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_USBD_CONFIG_LOG_LEVEL
#define APP_USBD_CONFIG_LOG_LEVEL 3
#endif

// <o> APP_USBD_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_USBD_CONFIG_INFO_COLOR
#define APP_USBD_CONFIG_INFO_COLOR 0
#endif

// <o> APP_USBD_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_USBD_CONFIG_DEBUG_COLOR
#define APP_USBD_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> APP_USBD_DUMMY_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_USBD_DUMMY_CONFIG_LOG_ENABLED
#define APP_USBD_DUMMY_CONFIG_LOG_ENABLED 0
#endif
// <o> APP_USBD_DUMMY_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_USBD_DUMMY_CONFIG_LOG_LEVEL
#define APP_USBD_DUMMY_CONFIG_LOG_LEVEL 3
#endif

// <o> APP_USBD_DUMMY_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_USBD_DUMMY_CONFIG_INFO_COLOR
#define APP_USBD_DUMMY_CONFIG_INFO_COLOR 0
#endif

// <o> APP_USBD_DUMMY_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_USBD_DUMMY_CONFIG_DEBUG_COLOR
#define APP_USBD_DUMMY_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> APP_USBD_MSC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_USBD_MSC_CONFIG_LOG_ENABLED
#define APP_USBD_MSC_CONFIG_LOG_ENABLED 0
#endif
// <o> APP_USBD_MSC_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_USBD_MSC_CONFIG_LOG_LEVEL
#define APP_USBD_MSC_CONFIG_LOG_LEVEL 3
#endif

// <o> APP_USBD_MSC_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_USBD_MSC_CONFIG_INFO_COLOR
#define APP_USBD_MSC_CONFIG_INFO_COLOR 0
#endif

// <o> APP_USBD_MSC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_USBD_MSC_CONFIG_DEBUG_COLOR
#define APP_USBD_MSC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> APP_USBD_NRF_DFU_TRIGGER_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_USBD_NRF_DFU_TRIGGER_CONFIG_LOG_ENABLED
#define APP_USBD_NRF_DFU_TRIGGER_CONFIG_LOG_ENABLED 0
#endif
// <o> APP_USBD_NRF_DFU_TRIGGER_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef APP_USBD_NRF_DFU_TRIGGER_CONFIG_LOG_LEVEL
#define APP_USBD_NRF_DFU_TRIGGER_CONFIG_LOG_LEVEL 3
#endif

// <o> APP_USBD_NRF_DFU_TRIGGER_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_USBD_NRF_DFU_TRIGGER_CONFIG_INFO_COLOR
#define APP_USBD_NRF_DFU_TRIGGER_CONFIG_INFO_COLOR 0
#endif

// <o> APP_USBD_NRF_DFU_TRIGGER_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef APP_USBD_NRF_DFU_TRIGGER_CONFIG_DEBUG_COLOR
#define APP_USBD_NRF_DFU_TRIGGER_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_ATFIFO_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_ATFIFO_CONFIG_LOG_ENABLED
#define NRF_ATFIFO_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_ATFIFO_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_ATFIFO_CONFIG_LOG_LEVEL
#define NRF_ATFIFO_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_ATFIFO_CONFIG_LOG_INIT_FILTER_LEVEL  - Initial severity level if dynamic filtering is enabled

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_ATFIFO_CONFIG_LOG_INIT_FILTER_LEVEL
#define NRF_ATFIFO_CONFIG_LOG_INIT_FILTER_LEVEL 3
#endif

// <o> NRF_ATFIFO_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_ATFIFO_CONFIG_INFO_COLOR
#define NRF_ATFIFO_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_ATFIFO_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_ATFIFO_CONFIG_DEBUG_COLOR
#define NRF_ATFIFO_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_BALLOC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_BALLOC_CONFIG_LOG_ENABLED
#define NRF_BALLOC_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_BALLOC_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_BALLOC_CONFIG_LOG_LEVEL
#define NRF_BALLOC_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_BALLOC_CONFIG_INITIAL_LOG_LEVEL  - Initial severity level if dynamic filtering is enabled.

// <i> If module generates a lot of logs, initial log level can
// <i> be decreased to prevent flooding. Severity level can be
// <i> increased on instance basis.
// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_BALLOC_CONFIG_INITIAL_LOG_LEVEL
#define NRF_BALLOC_CONFIG_INITIAL_LOG_LEVEL 3
#endif

// <o> NRF_BALLOC_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_BALLOC_CONFIG_INFO_COLOR
#define NRF_BALLOC_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_BALLOC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_BALLOC_CONFIG_DEBUG_COLOR
#define NRF_BALLOC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_BLOCK_DEV_EMPTY_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_BLOCK_DEV_EMPTY_CONFIG_LOG_ENABLED
#define NRF_BLOCK_DEV_EMPTY_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_BLOCK_DEV_EMPTY_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_BLOCK_DEV_EMPTY_CONFIG_LOG_LEVEL
#define NRF_BLOCK_DEV_EMPTY_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_BLOCK_DEV_EMPTY_CONFIG_LOG_INIT_FILTER_LEVEL  - Initial severity level if dynamic filtering is enabled

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_BLOCK_DEV_EMPTY_CONFIG_LOG_INIT_FILTER_LEVEL
#define NRF_BLOCK_DEV_EMPTY_CONFIG_LOG_INIT_FILTER_LEVEL 3
#endif

// <o> NRF_BLOCK_DEV_EMPTY_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_BLOCK_DEV_EMPTY_CONFIG_INFO_COLOR
#define NRF_BLOCK_DEV_EMPTY_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_BLOCK_DEV_EMPTY_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_BLOCK_DEV_EMPTY_CONFIG_DEBUG_COLOR
#define NRF_BLOCK_DEV_EMPTY_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_BLOCK_DEV_QSPI_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_BLOCK_DEV_QSPI_CONFIG_LOG_ENABLED
#define NRF_BLOCK_DEV_QSPI_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_BLOCK_DEV_QSPI_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_BLOCK_DEV_QSPI_CONFIG_LOG_LEVEL
#define NRF_BLOCK_DEV_QSPI_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_BLOCK_DEV_QSPI_CONFIG_LOG_INIT_FILTER_LEVEL  - Initial severity level if dynamic filtering is enabled

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_BLOCK_DEV_QSPI_CONFIG_LOG_INIT_FILTER_LEVEL
#define NRF_BLOCK_DEV_QSPI_CONFIG_LOG_INIT_FILTER_LEVEL 3
#endif

// <o> NRF_BLOCK_DEV_QSPI_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_BLOCK_DEV_QSPI_CONFIG_INFO_COLOR
#define NRF_BLOCK_DEV_QSPI_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_BLOCK_DEV_QSPI_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_BLOCK_DEV_QSPI_CONFIG_DEBUG_COLOR
#define NRF_BLOCK_DEV_QSPI_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_BLOCK_DEV_RAM_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_BLOCK_DEV_RAM_CONFIG_LOG_ENABLED
#define NRF_BLOCK_DEV_RAM_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_BLOCK_DEV_RAM_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_BLOCK_DEV_RAM_CONFIG_LOG_LEVEL
#define NRF_BLOCK_DEV_RAM_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_BLOCK_DEV_RAM_CONFIG_LOG_INIT_FILTER_LEVEL  - Initial severity level if dynamic filtering is enabled

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_BLOCK_DEV_RAM_CONFIG_LOG_INIT_FILTER_LEVEL
#define NRF_BLOCK_DEV_RAM_CONFIG_LOG_INIT_FILTER_LEVEL 3
#endif

// <o> NRF_BLOCK_DEV_RAM_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_BLOCK_DEV_RAM_CONFIG_INFO_COLOR
#define NRF_BLOCK_DEV_RAM_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_BLOCK_DEV_RAM_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_BLOCK_DEV_RAM_CONFIG_DEBUG_COLOR
#define NRF_BLOCK_DEV_RAM_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_CLI_BLE_UART_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_CLI_BLE_UART_CONFIG_LOG_ENABLED
#define NRF_CLI_BLE_UART_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_CLI_BLE_UART_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_CLI_BLE_UART_CONFIG_LOG_LEVEL
#define NRF_CLI_BLE_UART_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_CLI_BLE_UART_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_CLI_BLE_UART_CONFIG_INFO_COLOR
#define NRF_CLI_BLE_UART_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_CLI_BLE_UART_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_CLI_BLE_UART_CONFIG_DEBUG_COLOR
#define NRF_CLI_BLE_UART_CONFIG_DEBUG_COLOR 0
#endif

// </e>
// <q> NRF_LIBUARTE_ASYNC_WITH_APP_TIMER  - nrf_libuarte_async - libUARTE_async library

#ifndef NRF_LIBUARTE_ASYNC_WITH_APP_TIMER
#define NRF_LIBUARTE_ASYNC_WITH_APP_TIMER 1
#endif

// <e> NRF_CLI_LIBUARTE_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_CLI_LIBUARTE_CONFIG_LOG_ENABLED
#define NRF_CLI_LIBUARTE_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_CLI_LIBUARTE_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_CLI_LIBUARTE_CONFIG_LOG_LEVEL
#define NRF_CLI_LIBUARTE_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_CLI_LIBUARTE_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_CLI_LIBUARTE_CONFIG_INFO_COLOR
#define NRF_CLI_LIBUARTE_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_CLI_LIBUARTE_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_CLI_LIBUARTE_CONFIG_DEBUG_COLOR
#define NRF_CLI_LIBUARTE_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <h> nrf_cli_libuarte - libUARTE command line interface transport

//==========================================================
// <q> NRF_CLI_LIBUARTE_ENABLED  - Enable/disable the CLI libUARTE module.

#ifndef NRF_CLI_LIBUARTE_ENABLED
#define NRF_CLI_LIBUARTE_ENABLED 1
#endif

// <o> NRF_CLI_LIBUARTE_UARTE_INSTANCE  - UARTE instance

// <0=> UARTE0
// <1=> UARTE1

#ifndef NRF_CLI_LIBUARTE_UARTE_INSTANCE
#define NRF_CLI_LIBUARTE_UARTE_INSTANCE 0
#endif

// <o> NRF_CLI_LIBUARTE_TIMER_INSTANCE  - UARTE TIMER instance

// <0=> TIMER0
// <1=> TIMER1
// <2=> TIMER2
// <3=> TIMER3
// <4=> TIMER4

#ifndef NRF_CLI_LIBUARTE_TIMER_INSTANCE
#define NRF_CLI_LIBUARTE_TIMER_INSTANCE 0
#endif

// <o> NRF_CLI_LIBUARTE_TIMEOUT_TIMER_INSTANCE  - UARTE timeout TIMER instance

// <0=> TIMER0
// <1=> TIMER1
// <2=> TIMER2
// <3=> TIMER3
// <4=> TIMER4
// <255=> RTC used

#ifndef NRF_CLI_LIBUARTE_TIMEOUT_TIMER_INSTANCE
#define NRF_CLI_LIBUARTE_TIMEOUT_TIMER_INSTANCE 255
#endif

// <o> NRF_CLI_LIBUARTE_TIMEOUT_RTC_INSTANCE  - UARTE timeout RTC instance

// <0=> RTC0
// <1=> RTC1
// <2=> RTC2
// <255=> TIMER used

#ifndef NRF_CLI_LIBUARTE_TIMEOUT_RTC_INSTANCE
#define NRF_CLI_LIBUARTE_TIMEOUT_RTC_INSTANCE 1
#endif

// </h>
//==========================================================

// <h> nrf_libuarte_drv - libUARTE library

//==========================================================
// <q> NRF_LIBUARTE_DRV_HWFC_ENABLED  - Enable HWFC support in the driver

#ifndef NRF_LIBUARTE_DRV_HWFC_ENABLED
#define NRF_LIBUARTE_DRV_HWFC_ENABLED 0
#endif

// <q> NRF_LIBUARTE_DRV_UARTE0  - UARTE0 instance

#ifndef NRF_LIBUARTE_DRV_UARTE0
#define NRF_LIBUARTE_DRV_UARTE0 1
#endif

// <q> NRF_LIBUARTE_DRV_UARTE1  - UARTE1 instance

#ifndef NRF_LIBUARTE_DRV_UARTE1
#define NRF_LIBUARTE_DRV_UARTE1 0
#endif

// </h>
//==========================================================

// <e> NRF_CLI_UART_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_CLI_UART_CONFIG_LOG_ENABLED
#define NRF_CLI_UART_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_CLI_UART_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_CLI_UART_CONFIG_LOG_LEVEL
#define NRF_CLI_UART_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_CLI_UART_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_CLI_UART_CONFIG_INFO_COLOR
#define NRF_CLI_UART_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_CLI_UART_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_CLI_UART_CONFIG_DEBUG_COLOR
#define NRF_CLI_UART_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_LIBUARTE_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_LIBUARTE_CONFIG_LOG_ENABLED
#define NRF_LIBUARTE_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_LIBUARTE_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_LIBUARTE_CONFIG_LOG_LEVEL
#define NRF_LIBUARTE_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_LIBUARTE_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_LIBUARTE_CONFIG_INFO_COLOR
#define NRF_LIBUARTE_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_LIBUARTE_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_LIBUARTE_CONFIG_DEBUG_COLOR
#define NRF_LIBUARTE_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_MEMOBJ_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_MEMOBJ_CONFIG_LOG_ENABLED
#define NRF_MEMOBJ_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_MEMOBJ_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_MEMOBJ_CONFIG_LOG_LEVEL
#define NRF_MEMOBJ_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_MEMOBJ_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_MEMOBJ_CONFIG_INFO_COLOR
#define NRF_MEMOBJ_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_MEMOBJ_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_MEMOBJ_CONFIG_DEBUG_COLOR
#define NRF_MEMOBJ_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_PWR_MGMT_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_PWR_MGMT_CONFIG_LOG_ENABLED
#define NRF_PWR_MGMT_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_PWR_MGMT_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_PWR_MGMT_CONFIG_LOG_LEVEL
#define NRF_PWR_MGMT_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_PWR_MGMT_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_PWR_MGMT_CONFIG_INFO_COLOR
#define NRF_PWR_MGMT_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_PWR_MGMT_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_PWR_MGMT_CONFIG_DEBUG_COLOR
#define NRF_PWR_MGMT_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_QUEUE_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_QUEUE_CONFIG_LOG_ENABLED
#define NRF_QUEUE_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_QUEUE_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_QUEUE_CONFIG_LOG_LEVEL
#define NRF_QUEUE_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_QUEUE_CONFIG_LOG_INIT_FILTER_LEVEL  - Initial severity level if dynamic filtering is enabled

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_QUEUE_CONFIG_LOG_INIT_FILTER_LEVEL
#define NRF_QUEUE_CONFIG_LOG_INIT_FILTER_LEVEL 3
#endif

// <o> NRF_QUEUE_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_QUEUE_CONFIG_INFO_COLOR
#define NRF_QUEUE_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_QUEUE_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_QUEUE_CONFIG_DEBUG_COLOR
#define NRF_QUEUE_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_SDH_ANT_LOG_ENABLED - Enable logging in SoftDevice handler (ANT) module.
//==========================================================
#ifndef NRF_SDH_ANT_LOG_ENABLED
#define NRF_SDH_ANT_LOG_ENABLED 0
#endif
// <o> NRF_SDH_ANT_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_SDH_ANT_LOG_LEVEL
#define NRF_SDH_ANT_LOG_LEVEL 3
#endif

// <o> NRF_SDH_ANT_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_SDH_ANT_INFO_COLOR
#define NRF_SDH_ANT_INFO_COLOR 0
#endif

// <o> NRF_SDH_ANT_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_SDH_ANT_DEBUG_COLOR
#define NRF_SDH_ANT_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_SDH_BLE_LOG_ENABLED - Enable logging in SoftDevice handler (BLE) module.
//==========================================================
#ifndef NRF_SDH_BLE_LOG_ENABLED
#define NRF_SDH_BLE_LOG_ENABLED 0
#endif
// <o> NRF_SDH_BLE_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_SDH_BLE_LOG_LEVEL
#define NRF_SDH_BLE_LOG_LEVEL 3
#endif

// <o> NRF_SDH_BLE_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_SDH_BLE_INFO_COLOR
#define NRF_SDH_BLE_INFO_COLOR 0
#endif

// <o> NRF_SDH_BLE_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_SDH_BLE_DEBUG_COLOR
#define NRF_SDH_BLE_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_SDH_LOG_ENABLED - Enable logging in SoftDevice handler module.
//==========================================================
#ifndef NRF_SDH_LOG_ENABLED
#define NRF_SDH_LOG_ENABLED 0
#endif
// <o> NRF_SDH_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_SDH_LOG_LEVEL
#define NRF_SDH_LOG_LEVEL 3
#endif

// <o> NRF_SDH_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_SDH_INFO_COLOR
#define NRF_SDH_INFO_COLOR 0
#endif

// <o> NRF_SDH_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_SDH_DEBUG_COLOR
#define NRF_SDH_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_SDH_SOC_LOG_ENABLED - Enable logging in SoftDevice handler (SoC) module.
//==========================================================
#ifndef NRF_SDH_SOC_LOG_ENABLED
#define NRF_SDH_SOC_LOG_ENABLED 0
#endif
// <o> NRF_SDH_SOC_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_SDH_SOC_LOG_LEVEL
#define NRF_SDH_SOC_LOG_LEVEL 3
#endif

// <o> NRF_SDH_SOC_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_SDH_SOC_INFO_COLOR
#define NRF_SDH_SOC_INFO_COLOR 0
#endif

// <o> NRF_SDH_SOC_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_SDH_SOC_DEBUG_COLOR
#define NRF_SDH_SOC_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_SORTLIST_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_SORTLIST_CONFIG_LOG_ENABLED
#define NRF_SORTLIST_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_SORTLIST_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_SORTLIST_CONFIG_LOG_LEVEL
#define NRF_SORTLIST_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_SORTLIST_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_SORTLIST_CONFIG_INFO_COLOR
#define NRF_SORTLIST_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_SORTLIST_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_SORTLIST_CONFIG_DEBUG_COLOR
#define NRF_SORTLIST_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_TWI_SENSOR_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_TWI_SENSOR_CONFIG_LOG_ENABLED
#define NRF_TWI_SENSOR_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_TWI_SENSOR_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_TWI_SENSOR_CONFIG_LOG_LEVEL
#define NRF_TWI_SENSOR_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_TWI_SENSOR_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_TWI_SENSOR_CONFIG_INFO_COLOR
#define NRF_TWI_SENSOR_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_TWI_SENSOR_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_TWI_SENSOR_CONFIG_DEBUG_COLOR
#define NRF_TWI_SENSOR_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> PM_LOG_ENABLED - Enable logging in Peer Manager and its submodules.
//==========================================================
#ifndef PM_LOG_ENABLED
#define PM_LOG_ENABLED 1
#endif
// <o> PM_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef PM_LOG_LEVEL
#define PM_LOG_LEVEL 3
#endif

// <o> PM_LOG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef PM_LOG_INFO_COLOR
#define PM_LOG_INFO_COLOR 0
#endif

// <o> PM_LOG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef PM_LOG_DEBUG_COLOR
#define PM_LOG_DEBUG_COLOR 0
#endif

// </e>

// </h>
//==========================================================

// <h> nrf_log in nRF_Serialization

//==========================================================
// <e> SER_HAL_TRANSPORT_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef SER_HAL_TRANSPORT_CONFIG_LOG_ENABLED
#define SER_HAL_TRANSPORT_CONFIG_LOG_ENABLED 0
#endif
// <o> SER_HAL_TRANSPORT_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef SER_HAL_TRANSPORT_CONFIG_LOG_LEVEL
#define SER_HAL_TRANSPORT_CONFIG_LOG_LEVEL 3
#endif

// <o> SER_HAL_TRANSPORT_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef SER_HAL_TRANSPORT_CONFIG_INFO_COLOR
#define SER_HAL_TRANSPORT_CONFIG_INFO_COLOR 0
#endif

// <o> SER_HAL_TRANSPORT_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef SER_HAL_TRANSPORT_CONFIG_DEBUG_COLOR
#define SER_HAL_TRANSPORT_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </h>
//==========================================================

// </h>
//==========================================================

// </e>

// <q> NRF_LOG_STR_FORMATTER_TIMESTAMP_FORMAT_ENABLED  - nrf_log_str_formatter - Log string formatter

#ifndef NRF_LOG_STR_FORMATTER_TIMESTAMP_FORMAT_ENABLED
#define NRF_LOG_STR_FORMATTER_TIMESTAMP_FORMAT_ENABLED 1
#endif

// </h>
//==========================================================

// <h> nRF_drv_radio_802_15_4

//==========================================================
// <h> nrf_fem_config - nRF 21540 FEM configuration options

//==========================================================
// <o> FEM_CONTROL_DEFAULT_PA_PIN - Pin number, controlling Power Amplifier of the FEM module
#ifndef FEM_CONTROL_DEFAULT_PA_PIN
#define FEM_CONTROL_DEFAULT_PA_PIN 22
#endif

// <o> FEM_CONTROL_DEFAULT_LNA_PIN - Pin number, controlling Low Noise Amplifier of the FEM module
#ifndef FEM_CONTROL_DEFAULT_LNA_PIN
#define FEM_CONTROL_DEFAULT_LNA_PIN 19
#endif

// <o> FEM_CONTROL_DEFAULT_PDN_PIN - Pin number, controlling Power Down pin of the FEM module
#ifndef FEM_CONTROL_DEFAULT_PDN_PIN
#define FEM_CONTROL_DEFAULT_PDN_PIN 23
#endif

// <o> FEM_CONTROL_DEFAULT_MODE_PIN - Pin number, selecting Mode of the FEM module
#ifndef FEM_CONTROL_DEFAULT_MODE_PIN
#define FEM_CONTROL_DEFAULT_MODE_PIN 17
#endif

// <o> FEM_CONTROL_DEFAULT_ANTSEL_PIN - Pin number, selecting Antenna output of the FEM module
#ifndef FEM_CONTROL_DEFAULT_ANTSEL_PIN
#define FEM_CONTROL_DEFAULT_ANTSEL_PIN 20
#endif

// <o> FEM_CONTROL_DEFAULT_MOSI_PIN - Pin number, attached to MOSI pin of the FEM module
#ifndef FEM_CONTROL_DEFAULT_MOSI_PIN
#define FEM_CONTROL_DEFAULT_MOSI_PIN 45
#endif

// <o> FEM_CONTROL_DEFAULT_MISO_PIN - Pin number, attached to MISO pin of the FEM module
#ifndef FEM_CONTROL_DEFAULT_MISO_PIN
#define FEM_CONTROL_DEFAULT_MISO_PIN 46
#endif

// <o> FEM_CONTROL_DEFAULT_CLK_PIN - Pin number, attached to CLK pin of the FEM module
#ifndef FEM_CONTROL_DEFAULT_CLK_PIN
#define FEM_CONTROL_DEFAULT_CLK_PIN 47
#endif

// <o> FEM_CONTROL_DEFAULT_CSN_PIN - Pin number, attached to CS pin of the FEM module
#ifndef FEM_CONTROL_DEFAULT_CSN_PIN
#define FEM_CONTROL_DEFAULT_CSN_PIN 21
#endif

// <q> FEM_CONTROL_DEFAULT_ENABLE  - Enables FEM control on GPIO pins by default

#ifndef FEM_CONTROL_DEFAULT_ENABLE
#define FEM_CONTROL_DEFAULT_ENABLE 0
#endif

// </h>
//==========================================================

// </h>
//==========================================================


//==========================================================

// <h> nRF_SoftDevice

//==========================================================
// <e> NRF_SDH_ANT_ENABLED - nrf_sdh_ant - SoftDevice ANT event handler
//==========================================================
#ifndef NRF_SDH_ANT_ENABLED
#define NRF_SDH_ANT_ENABLED 0
#endif
// <h> ANT Channels

//==========================================================
// <o> NRF_SDH_ANT_TOTAL_CHANNELS_ALLOCATED - Allocated ANT channels.
#ifndef NRF_SDH_ANT_TOTAL_CHANNELS_ALLOCATED
#define NRF_SDH_ANT_TOTAL_CHANNELS_ALLOCATED 0
#endif

// <o> NRF_SDH_ANT_ENCRYPTED_CHANNELS - Encrypted ANT channels.
#ifndef NRF_SDH_ANT_ENCRYPTED_CHANNELS
#define NRF_SDH_ANT_ENCRYPTED_CHANNELS 0
#endif

// </h>
//==========================================================

// <h> ANT Queues

//==========================================================
// <o> NRF_SDH_ANT_EVENT_QUEUE_SIZE - Event queue size.
#ifndef NRF_SDH_ANT_EVENT_QUEUE_SIZE
#define NRF_SDH_ANT_EVENT_QUEUE_SIZE 32
#endif

// <o> NRF_SDH_ANT_BURST_QUEUE_SIZE - ANT burst queue size.
#ifndef NRF_SDH_ANT_BURST_QUEUE_SIZE
#define NRF_SDH_ANT_BURST_QUEUE_SIZE 128
#endif

// </h>
//==========================================================

// <h> ANT Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_ANT_OBSERVER_PRIO_LEVELS - Total number of priority levels for ANT observers.
// <i> This setting configures the number of priority levels available for the ANT event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_ANT_OBSERVER_PRIO_LEVELS
#define NRF_SDH_ANT_OBSERVER_PRIO_LEVELS 2
#endif

// <h> ANT Observers priorities - Invididual priorities

//==========================================================
// <o> ANT_BPWR_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Bicycle Power Profile.

#ifndef ANT_BPWR_ANT_OBSERVER_PRIO
#define ANT_BPWR_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_BSC_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Bicycle Speed and Cadence Profile.

#ifndef ANT_BSC_ANT_OBSERVER_PRIO
#define ANT_BSC_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_ENCRYPT_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Cryptographic ANT stack configuration module.

#ifndef ANT_ENCRYPT_ANT_OBSERVER_PRIO
#define ANT_ENCRYPT_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_HRM_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Heart Rate Monitor.

#ifndef ANT_HRM_ANT_OBSERVER_PRIO
#define ANT_HRM_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_SDM_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Stride Based Speed and Distance Monitor Profile.

#ifndef ANT_SDM_ANT_OBSERVER_PRIO
#define ANT_SDM_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_STATE_INDICATOR_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the ANT state indicator module.

#ifndef ANT_STATE_INDICATOR_ANT_OBSERVER_PRIO
#define ANT_STATE_INDICATOR_ANT_OBSERVER_PRIO 1
#endif

// <o> BSP_BTN_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Button Control module.

#ifndef BSP_BTN_ANT_OBSERVER_PRIO
#define BSP_BTN_ANT_OBSERVER_PRIO 1
#endif

// <o> NRF_DFU_ANT_EVT_HANDLER_PRIO - DFU ANT event handling priority.
// <i> Priority used to register the ANT event handler for DFU purposes. Change this if there is other ANT event handlers that should be higher priority.

#ifndef NRF_DFU_ANT_EVT_HANDLER_PRIO
#define NRF_DFU_ANT_EVT_HANDLER_PRIO 1
#endif

// </h>
//==========================================================

// </h>
//==========================================================


// </e>

// <e> NRF_SDH_BLE_ENABLED - nrf_sdh_ble - SoftDevice BLE event handler
//==========================================================
#ifndef NRF_SDH_BLE_ENABLED
#define NRF_SDH_BLE_ENABLED 0
#endif
// <h> BLE Stack configuration - Stack configuration parameters

// <i> The SoftDevice handler will configure the stack with these parameters when calling @ref nrf_sdh_ble_default_cfg_set.
// <i> Other libraries might depend on these values; keep them up-to-date even if you are not explicitely calling @ref nrf_sdh_ble_default_cfg_set.
//==========================================================
// <o> NRF_SDH_BLE_GAP_DATA_LENGTH   <27-251>


// <i> Requested BLE GAP data length to be negotiated.

#ifndef NRF_SDH_BLE_GAP_DATA_LENGTH
#define NRF_SDH_BLE_GAP_DATA_LENGTH 27
#endif

// <o> NRF_SDH_BLE_PERIPHERAL_LINK_COUNT - Maximum number of peripheral links.
#ifndef NRF_SDH_BLE_PERIPHERAL_LINK_COUNT
#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT 0
#endif

// <o> NRF_SDH_BLE_CENTRAL_LINK_COUNT - Maximum number of central links.
#ifndef NRF_SDH_BLE_CENTRAL_LINK_COUNT
#define NRF_SDH_BLE_CENTRAL_LINK_COUNT 0
#endif

// <o> NRF_SDH_BLE_TOTAL_LINK_COUNT - Total link count.
// <i> Maximum number of total concurrent connections using the default configuration.

#ifndef NRF_SDH_BLE_TOTAL_LINK_COUNT
#define NRF_SDH_BLE_TOTAL_LINK_COUNT 1
#endif

// <o> NRF_SDH_BLE_GAP_EVENT_LENGTH - GAP event length.
// <i> The time set aside for this connection on every connection interval in 1.25 ms units.

#ifndef NRF_SDH_BLE_GAP_EVENT_LENGTH
#define NRF_SDH_BLE_GAP_EVENT_LENGTH 6
#endif

// <o> NRF_SDH_BLE_GATT_MAX_MTU_SIZE - Static maximum MTU size.
#ifndef NRF_SDH_BLE_GATT_MAX_MTU_SIZE
#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE 23
#endif

// <o> NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE - Attribute Table size in bytes. The size must be a multiple of 4.
#ifndef NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE
#define NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE 1408
#endif

// <o> NRF_SDH_BLE_VS_UUID_COUNT - The number of vendor-specific UUIDs.
#ifndef NRF_SDH_BLE_VS_UUID_COUNT
#define NRF_SDH_BLE_VS_UUID_COUNT 0
#endif

// <q> NRF_SDH_BLE_SERVICE_CHANGED  - Include the Service Changed characteristic in the Attribute Table.


#ifndef NRF_SDH_BLE_SERVICE_CHANGED
#define NRF_SDH_BLE_SERVICE_CHANGED 0
#endif

// </h>
//==========================================================

// <h> BLE Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_BLE_OBSERVER_PRIO_LEVELS - Total number of priority levels for BLE observers.
// <i> This setting configures the number of priority levels available for BLE event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_BLE_OBSERVER_PRIO_LEVELS
#define NRF_SDH_BLE_OBSERVER_PRIO_LEVELS 4
#endif

// <h> BLE Observers priorities - Invididual priorities

//==========================================================
// <o> BLE_ADV_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Advertising module.

#ifndef BLE_ADV_BLE_OBSERVER_PRIO
#define BLE_ADV_BLE_OBSERVER_PRIO 1
#endif

// <o> BLE_ANCS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Apple Notification Service Client.

#ifndef BLE_ANCS_C_BLE_OBSERVER_PRIO
#define BLE_ANCS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_ANS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Alert Notification Service Client.

#ifndef BLE_ANS_C_BLE_OBSERVER_PRIO
#define BLE_ANS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_BAS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Battery Service.

#ifndef BLE_BAS_BLE_OBSERVER_PRIO
#define BLE_BAS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_BAS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Battery Service Client.

#ifndef BLE_BAS_C_BLE_OBSERVER_PRIO
#define BLE_BAS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_BPS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Blood Pressure Service.

#ifndef BLE_BPS_BLE_OBSERVER_PRIO
#define BLE_BPS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_CONN_PARAMS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Connection parameters module.

#ifndef BLE_CONN_PARAMS_BLE_OBSERVER_PRIO
#define BLE_CONN_PARAMS_BLE_OBSERVER_PRIO 1
#endif

// <o> BLE_CONN_STATE_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Connection State module.

#ifndef BLE_CONN_STATE_BLE_OBSERVER_PRIO
#define BLE_CONN_STATE_BLE_OBSERVER_PRIO 0
#endif

// <o> BLE_CSCS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Cycling Speed and Cadence Service.

#ifndef BLE_CSCS_BLE_OBSERVER_PRIO
#define BLE_CSCS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_CTS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Current Time Service Client.

#ifndef BLE_CTS_C_BLE_OBSERVER_PRIO
#define BLE_CTS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_DB_DISC_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Database Discovery module.

#ifndef BLE_DB_DISC_BLE_OBSERVER_PRIO
#define BLE_DB_DISC_BLE_OBSERVER_PRIO 1
#endif

// <o> BLE_DFU_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the DFU Service.

#ifndef BLE_DFU_BLE_OBSERVER_PRIO
#define BLE_DFU_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_DIS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Device Information Client.

#ifndef BLE_DIS_C_BLE_OBSERVER_PRIO
#define BLE_DIS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_GLS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Glucose Service.

#ifndef BLE_GLS_BLE_OBSERVER_PRIO
#define BLE_GLS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HIDS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Human Interface Device Service.

#ifndef BLE_HIDS_BLE_OBSERVER_PRIO
#define BLE_HIDS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HRS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Heart Rate Service.

#ifndef BLE_HRS_BLE_OBSERVER_PRIO
#define BLE_HRS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HRS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Heart Rate Service Client.

#ifndef BLE_HRS_C_BLE_OBSERVER_PRIO
#define BLE_HRS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HTS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Health Thermometer Service.

#ifndef BLE_HTS_BLE_OBSERVER_PRIO
#define BLE_HTS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_IAS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Immediate Alert Service.

#ifndef BLE_IAS_BLE_OBSERVER_PRIO
#define BLE_IAS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_IAS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Immediate Alert Service Client.

#ifndef BLE_IAS_C_BLE_OBSERVER_PRIO
#define BLE_IAS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LBS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the LED Button Service.

#ifndef BLE_LBS_BLE_OBSERVER_PRIO
#define BLE_LBS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LBS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the LED Button Service Client.

#ifndef BLE_LBS_C_BLE_OBSERVER_PRIO
#define BLE_LBS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LLS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Link Loss Service.

#ifndef BLE_LLS_BLE_OBSERVER_PRIO
#define BLE_LLS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LNS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Location Navigation Service.

#ifndef BLE_LNS_BLE_OBSERVER_PRIO
#define BLE_LNS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_NUS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the UART Service.

#ifndef BLE_NUS_BLE_OBSERVER_PRIO
#define BLE_NUS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_NUS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the UART Central Service.

#ifndef BLE_NUS_C_BLE_OBSERVER_PRIO
#define BLE_NUS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_OTS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Object transfer service.

#ifndef BLE_OTS_BLE_OBSERVER_PRIO
#define BLE_OTS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_OTS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Object transfer service client.

#ifndef BLE_OTS_C_BLE_OBSERVER_PRIO
#define BLE_OTS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_RSCS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Running Speed and Cadence Service.

#ifndef BLE_RSCS_BLE_OBSERVER_PRIO
#define BLE_RSCS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_RSCS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Running Speed and Cadence Client.

#ifndef BLE_RSCS_C_BLE_OBSERVER_PRIO
#define BLE_RSCS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_TPS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the TX Power Service.

#ifndef BLE_TPS_BLE_OBSERVER_PRIO
#define BLE_TPS_BLE_OBSERVER_PRIO 2
#endif

// <o> BSP_BTN_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Button Control module.

#ifndef BSP_BTN_BLE_OBSERVER_PRIO
#define BSP_BTN_BLE_OBSERVER_PRIO 1
#endif

// <o> NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the NFC pairing library.

#ifndef NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO
#define NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO 1
#endif

// <o> NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the NFC pairing library.

#ifndef NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO
#define NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO 1
#endif

// <o> NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the NFC pairing library.

#ifndef NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO
#define NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO 1
#endif

// <o> NRF_BLE_BMS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Bond Management Service.

#ifndef NRF_BLE_BMS_BLE_OBSERVER_PRIO
#define NRF_BLE_BMS_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_CGMS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Contiuon Glucose Monitoring Service.

#ifndef NRF_BLE_CGMS_BLE_OBSERVER_PRIO
#define NRF_BLE_CGMS_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_ES_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Eddystone module.

#ifndef NRF_BLE_ES_BLE_OBSERVER_PRIO
#define NRF_BLE_ES_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_GATTS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the GATT Service Client.

#ifndef NRF_BLE_GATTS_C_BLE_OBSERVER_PRIO
#define NRF_BLE_GATTS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_GATT_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the GATT module.

#ifndef NRF_BLE_GATT_BLE_OBSERVER_PRIO
#define NRF_BLE_GATT_BLE_OBSERVER_PRIO 1
#endif

// <o> NRF_BLE_GQ_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the GATT Queue module.

#ifndef NRF_BLE_GQ_BLE_OBSERVER_PRIO
#define NRF_BLE_GQ_BLE_OBSERVER_PRIO 1
#endif

// <o> NRF_BLE_QWR_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Queued writes module.

#ifndef NRF_BLE_QWR_BLE_OBSERVER_PRIO
#define NRF_BLE_QWR_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_SCAN_OBSERVER_PRIO
// <i> Priority for dispatching the BLE events to the Scanning Module.

#ifndef NRF_BLE_SCAN_OBSERVER_PRIO
#define NRF_BLE_SCAN_OBSERVER_PRIO 1
#endif

// <o> PM_BLE_OBSERVER_PRIO - Priority with which BLE events are dispatched to the Peer Manager module.
#ifndef PM_BLE_OBSERVER_PRIO
#define PM_BLE_OBSERVER_PRIO 1
#endif

// </h>
//==========================================================

// </h>
//==========================================================


// </e>

// <e> NRF_SDH_ENABLED - nrf_sdh - SoftDevice handler
//==========================================================
#ifndef NRF_SDH_ENABLED
#define NRF_SDH_ENABLED 0
#endif
// <h> Dispatch model

// <i> This setting configures how Stack events are dispatched to the application.
//==========================================================
// <o> NRF_SDH_DISPATCH_MODEL


// <i> NRF_SDH_DISPATCH_MODEL_INTERRUPT: SoftDevice events are passed to the application from the interrupt context.
// <i> NRF_SDH_DISPATCH_MODEL_APPSH: SoftDevice events are scheduled using @ref app_scheduler.
// <i> NRF_SDH_DISPATCH_MODEL_POLLING: SoftDevice events are to be fetched manually.
// <0=> NRF_SDH_DISPATCH_MODEL_INTERRUPT
// <1=> NRF_SDH_DISPATCH_MODEL_APPSH
// <2=> NRF_SDH_DISPATCH_MODEL_POLLING

#ifndef NRF_SDH_DISPATCH_MODEL
#define NRF_SDH_DISPATCH_MODEL 0
#endif

// </h>
//==========================================================

// <h> Clock - SoftDevice clock configuration

//==========================================================
// <o> NRF_SDH_CLOCK_LF_SRC  - SoftDevice clock source.

// <0=> NRF_CLOCK_LF_SRC_RC
// <1=> NRF_CLOCK_LF_SRC_XTAL
// <2=> NRF_CLOCK_LF_SRC_SYNTH

#ifndef NRF_SDH_CLOCK_LF_SRC
#define NRF_SDH_CLOCK_LF_SRC 1
#endif

// <o> NRF_SDH_CLOCK_LF_RC_CTIV - SoftDevice calibration timer interval.
#ifndef NRF_SDH_CLOCK_LF_RC_CTIV
#define NRF_SDH_CLOCK_LF_RC_CTIV 0
#endif

// <o> NRF_SDH_CLOCK_LF_RC_TEMP_CTIV - SoftDevice calibration timer interval under constant temperature.
// <i> How often (in number of calibration intervals) the RC oscillator shall be calibrated
// <i>  if the temperature has not changed.

#ifndef NRF_SDH_CLOCK_LF_RC_TEMP_CTIV
#define NRF_SDH_CLOCK_LF_RC_TEMP_CTIV 0
#endif

// <o> NRF_SDH_CLOCK_LF_ACCURACY  - External clock accuracy used in the LL to compute timing.

// <0=> NRF_CLOCK_LF_ACCURACY_250_PPM
// <1=> NRF_CLOCK_LF_ACCURACY_500_PPM
// <2=> NRF_CLOCK_LF_ACCURACY_150_PPM
// <3=> NRF_CLOCK_LF_ACCURACY_100_PPM
// <4=> NRF_CLOCK_LF_ACCURACY_75_PPM
// <5=> NRF_CLOCK_LF_ACCURACY_50_PPM
// <6=> NRF_CLOCK_LF_ACCURACY_30_PPM
// <7=> NRF_CLOCK_LF_ACCURACY_20_PPM
// <8=> NRF_CLOCK_LF_ACCURACY_10_PPM
// <9=> NRF_CLOCK_LF_ACCURACY_5_PPM
// <10=> NRF_CLOCK_LF_ACCURACY_2_PPM
// <11=> NRF_CLOCK_LF_ACCURACY_1_PPM

#ifndef NRF_SDH_CLOCK_LF_ACCURACY
#define NRF_SDH_CLOCK_LF_ACCURACY 7
#endif

// </h>
//==========================================================

// <h> SDH Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_REQ_OBSERVER_PRIO_LEVELS - Total number of priority levels for request observers.
// <i> This setting configures the number of priority levels available for the SoftDevice request event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_REQ_OBSERVER_PRIO_LEVELS
#define NRF_SDH_REQ_OBSERVER_PRIO_LEVELS 2
#endif

// <o> NRF_SDH_STATE_OBSERVER_PRIO_LEVELS - Total number of priority levels for state observers.
// <i> This setting configures the number of priority levels available for the SoftDevice state event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_STATE_OBSERVER_PRIO_LEVELS
#define NRF_SDH_STATE_OBSERVER_PRIO_LEVELS 2
#endif

// <o> NRF_SDH_STACK_OBSERVER_PRIO_LEVELS - Total number of priority levels for stack event observers.
// <i> This setting configures the number of priority levels available for the SoftDevice stack event handlers (ANT, BLE, SoC).
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_STACK_OBSERVER_PRIO_LEVELS
#define NRF_SDH_STACK_OBSERVER_PRIO_LEVELS 2
#endif


// <h> State Observers priorities - Invididual priorities

//==========================================================
// <o> CLOCK_CONFIG_STATE_OBSERVER_PRIO
// <i> Priority with which state events are dispatched to the Clock driver.

#ifndef CLOCK_CONFIG_STATE_OBSERVER_PRIO
#define CLOCK_CONFIG_STATE_OBSERVER_PRIO 0
#endif

// <o> POWER_CONFIG_STATE_OBSERVER_PRIO
// <i> Priority with which state events are dispatched to the Power driver.

#ifndef POWER_CONFIG_STATE_OBSERVER_PRIO
#define POWER_CONFIG_STATE_OBSERVER_PRIO 0
#endif

// <o> RNG_CONFIG_STATE_OBSERVER_PRIO
// <i> Priority with which state events are dispatched to this module.

#ifndef RNG_CONFIG_STATE_OBSERVER_PRIO
#define RNG_CONFIG_STATE_OBSERVER_PRIO 0
#endif

// </h>
//==========================================================

// <h> Stack Event Observers priorities - Invididual priorities

//==========================================================
// <o> NRF_SDH_ANT_STACK_OBSERVER_PRIO
// <i> This setting configures the priority with which ANT events are processed with respect to other events coming from the stack.
// <i> Modify this setting if you need to have ANT events dispatched before or after other stack events, such as BLE or SoC.
// <i> Zero is the highest priority.

#ifndef NRF_SDH_ANT_STACK_OBSERVER_PRIO
#define NRF_SDH_ANT_STACK_OBSERVER_PRIO 0
#endif

// <o> NRF_SDH_BLE_STACK_OBSERVER_PRIO
// <i> This setting configures the priority with which BLE events are processed with respect to other events coming from the stack.
// <i> Modify this setting if you need to have BLE events dispatched before or after other stack events, such as ANT or SoC.
// <i> Zero is the highest priority.

#ifndef NRF_SDH_BLE_STACK_OBSERVER_PRIO
#define NRF_SDH_BLE_STACK_OBSERVER_PRIO 0
#endif

// <o> NRF_SDH_SOC_STACK_OBSERVER_PRIO
// <i> This setting configures the priority with which SoC events are processed with respect to other events coming from the stack.
// <i> Modify this setting if you need to have SoC events dispatched before or after other stack events, such as ANT or BLE.
// <i> Zero is the highest priority.

#ifndef NRF_SDH_SOC_STACK_OBSERVER_PRIO
#define NRF_SDH_SOC_STACK_OBSERVER_PRIO 0
#endif

// </h>
//==========================================================

// </h>
//==========================================================


// </e>

// <e> NRF_SDH_SOC_ENABLED - nrf_sdh_soc - SoftDevice SoC event handler
//==========================================================
#ifndef NRF_SDH_SOC_ENABLED
#define NRF_SDH_SOC_ENABLED 0
#endif
// <h> SoC Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_SOC_OBSERVER_PRIO_LEVELS - Total number of priority levels for SoC observers.
// <i> This setting configures the number of priority levels available for the SoC event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_SOC_OBSERVER_PRIO_LEVELS
#define NRF_SDH_SOC_OBSERVER_PRIO_LEVELS 2
#endif

// <h> SoC Observers priorities - Invididual priorities

//==========================================================
// <o> BLE_DFU_SOC_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the DFU Service.

#ifndef BLE_DFU_SOC_OBSERVER_PRIO
#define BLE_DFU_SOC_OBSERVER_PRIO 1
#endif

// <o> CLOCK_CONFIG_SOC_OBSERVER_PRIO
// <i> Priority with which SoC events are dispatched to the Clock driver.

#ifndef CLOCK_CONFIG_SOC_OBSERVER_PRIO
#define CLOCK_CONFIG_SOC_OBSERVER_PRIO 0
#endif

// <o> POWER_CONFIG_SOC_OBSERVER_PRIO
// <i> Priority with which SoC events are dispatched to the Power driver.

#ifndef POWER_CONFIG_SOC_OBSERVER_PRIO
#define POWER_CONFIG_SOC_OBSERVER_PRIO 0
#endif

// </h>
//==========================================================

// </h>
//==========================================================


// <e> NRF_BLE_CONN_PARAMS_ENABLED - ble_conn_params - Initiating and executing a connection parameters negotiation procedure
//==========================================================
#ifndef NRF_BLE_CONN_PARAMS_ENABLED
#define NRF_BLE_CONN_PARAMS_ENABLED 0
#endif
// <o> NRF_BLE_CONN_PARAMS_MAX_SLAVE_LATENCY_DEVIATION - The largest acceptable deviation in slave latency.
// <i> The largest deviation (+ or -) from the requested slave latency that will not be renegotiated.

#ifndef NRF_BLE_CONN_PARAMS_MAX_SLAVE_LATENCY_DEVIATION
#define NRF_BLE_CONN_PARAMS_MAX_SLAVE_LATENCY_DEVIATION 499
#endif

// <o> NRF_BLE_CONN_PARAMS_MAX_SUPERVISION_TIMEOUT_DEVIATION - The largest acceptable deviation (in 10 ms units) in supervision timeout.
// <i> The largest deviation (+ or -, in 10 ms units) from the requested supervision timeout that will not be renegotiated.

#ifndef NRF_BLE_CONN_PARAMS_MAX_SUPERVISION_TIMEOUT_DEVIATION
#define NRF_BLE_CONN_PARAMS_MAX_SUPERVISION_TIMEOUT_DEVIATION 65535
#endif

// </e>

// <q> NRF_BLE_GATT_ENABLED  - nrf_ble_gatt - GATT module


#ifndef NRF_BLE_GATT_ENABLED
#define NRF_BLE_GATT_ENABLED 0
#endif

// <e> NRF_BLE_QWR_ENABLED - nrf_ble_qwr - Queued writes support module (prepare/execute write)
//==========================================================
#ifndef NRF_BLE_QWR_ENABLED
#define NRF_BLE_QWR_ENABLED 0
#endif
// <o> NRF_BLE_QWR_MAX_ATTR - Maximum number of attribute handles that can be registered. This number must be adjusted according to the number of attributes for which Queued Writes will be enabled. If it is zero, the module will reject all Queued Write requests.
#ifndef NRF_BLE_QWR_MAX_ATTR
#define NRF_BLE_QWR_MAX_ATTR 0
#endif

// </e>

// <e> PEER_MANAGER_ENABLED - peer_manager - Peer Manager
//==========================================================
#ifndef PEER_MANAGER_ENABLED
#define PEER_MANAGER_ENABLED 0
#endif
// <o> PM_MAX_REGISTRANTS - Number of event handlers that can be registered.
#ifndef PM_MAX_REGISTRANTS
#define PM_MAX_REGISTRANTS 3
#endif

// <o> PM_FLASH_BUFFERS - Number of internal buffers for flash operations.
// <i> Decrease this value to lower RAM usage.

#ifndef PM_FLASH_BUFFERS
#define PM_FLASH_BUFFERS 4
#endif

// <q> PM_CENTRAL_ENABLED  - Enable/disable central-specific Peer Manager functionality.


// <i> Enable/disable central-specific Peer Manager functionality.

#ifndef PM_CENTRAL_ENABLED
#define PM_CENTRAL_ENABLED 1
#endif

// <q> PM_SERVICE_CHANGED_ENABLED  - Enable/disable the service changed management for GATT server in Peer Manager.


// <i> If not using a GATT server, or using a server wihout a service changed characteristic,
// <i> disable this to save code space.

#ifndef PM_SERVICE_CHANGED_ENABLED
#define PM_SERVICE_CHANGED_ENABLED 1
#endif

// <q> PM_PEER_RANKS_ENABLED  - Enable/disable the peer rank management in Peer Manager.


// <i> Set this to false to save code space if not using the peer rank API.

#ifndef PM_PEER_RANKS_ENABLED
#define PM_PEER_RANKS_ENABLED 1
#endif

// <q> PM_LESC_ENABLED  - Enable/disable LESC support in Peer Manager.


// <i> If set to true, you need to call nrf_ble_lesc_request_handler() in the main loop to respond to LESC-related BLE events. If LESC support is not required, set this to false to save code space.

#ifndef PM_LESC_ENABLED
#define PM_LESC_ENABLED 0
#endif

// <e> PM_RA_PROTECTION_ENABLED - Enable/disable protection against repeated pairing attempts in Peer Manager.
//==========================================================
#ifndef PM_RA_PROTECTION_ENABLED
#define PM_RA_PROTECTION_ENABLED 0
#endif
// <o> PM_RA_PROTECTION_TRACKED_PEERS_NUM - Maximum number of peers whose authorization status can be tracked.
#ifndef PM_RA_PROTECTION_TRACKED_PEERS_NUM
#define PM_RA_PROTECTION_TRACKED_PEERS_NUM 8
#endif

// <o> PM_RA_PROTECTION_MIN_WAIT_INTERVAL - Minimum waiting interval (in ms) before a new pairing attempt can be initiated.
#ifndef PM_RA_PROTECTION_MIN_WAIT_INTERVAL
#define PM_RA_PROTECTION_MIN_WAIT_INTERVAL 4000
#endif

// <o> PM_RA_PROTECTION_MAX_WAIT_INTERVAL - Maximum waiting interval (in ms) before a new pairing attempt can be initiated.
#ifndef PM_RA_PROTECTION_MAX_WAIT_INTERVAL
#define PM_RA_PROTECTION_MAX_WAIT_INTERVAL 64000
#endif

// <o> PM_RA_PROTECTION_REWARD_PERIOD - Reward period (in ms).
// <i> The waiting interval is gradually decreased when no new failed pairing attempts are made during reward period.

#ifndef PM_RA_PROTECTION_REWARD_PERIOD
#define PM_RA_PROTECTION_REWARD_PERIOD 10000
#endif

// </e>

// <o> PM_HANDLER_SEC_DELAY_MS - Delay before starting security.
// <i>  This might be necessary for interoperability reasons, especially as peripheral.

#ifndef PM_HANDLER_SEC_DELAY_MS
#define PM_HANDLER_SEC_DELAY_MS 0
#endif

// </e>

// </h>
//==========================================================

// <h> nRF_BLE_Services

//==========================================================
// <q> BLE_ANCS_C_ENABLED  - ble_ancs_c - Apple Notification Service Client


#ifndef BLE_ANCS_C_ENABLED
#define BLE_ANCS_C_ENABLED 0
#endif

// <q> BLE_ANS_C_ENABLED  - ble_ans_c - Alert Notification Service Client


#ifndef BLE_ANS_C_ENABLED
#define BLE_ANS_C_ENABLED 0
#endif

// <q> BLE_BAS_C_ENABLED  - ble_bas_c - Battery Service Client


#ifndef BLE_BAS_C_ENABLED
#define BLE_BAS_C_ENABLED 0
#endif

// <e> BLE_BAS_ENABLED - ble_bas - Battery Service
//==========================================================
#ifndef BLE_BAS_ENABLED
#define BLE_BAS_ENABLED 0
#endif
// <e> BLE_BAS_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef BLE_BAS_CONFIG_LOG_ENABLED
#define BLE_BAS_CONFIG_LOG_ENABLED 0
#endif
// <o> BLE_BAS_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef BLE_BAS_CONFIG_LOG_LEVEL
#define BLE_BAS_CONFIG_LOG_LEVEL 3
#endif

// <o> BLE_BAS_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef BLE_BAS_CONFIG_INFO_COLOR
#define BLE_BAS_CONFIG_INFO_COLOR 0
#endif

// <o> BLE_BAS_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef BLE_BAS_CONFIG_DEBUG_COLOR
#define BLE_BAS_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <q> BLE_CSCS_ENABLED  - ble_cscs - Cycling Speed and Cadence Service


#ifndef BLE_CSCS_ENABLED
#define BLE_CSCS_ENABLED 0
#endif

// <q> BLE_CTS_C_ENABLED  - ble_cts_c - Current Time Service Client


#ifndef BLE_CTS_C_ENABLED
#define BLE_CTS_C_ENABLED 0
#endif

// <q> BLE_DIS_ENABLED  - ble_dis - Device Information Service


#ifndef BLE_DIS_ENABLED
#define BLE_DIS_ENABLED 0
#endif

// <q> BLE_GLS_ENABLED  - ble_gls - Glucose Service


#ifndef BLE_GLS_ENABLED
#define BLE_GLS_ENABLED 0
#endif

// <q> BLE_HIDS_ENABLED  - ble_hids - Human Interface Device Service


#ifndef BLE_HIDS_ENABLED
#define BLE_HIDS_ENABLED 0
#endif

// <q> BLE_HRS_C_ENABLED  - ble_hrs_c - Heart Rate Service Client


#ifndef BLE_HRS_C_ENABLED
#define BLE_HRS_C_ENABLED 0
#endif

// <q> BLE_HRS_ENABLED  - ble_hrs - Heart Rate Service


#ifndef BLE_HRS_ENABLED
#define BLE_HRS_ENABLED 0
#endif

// <q> BLE_HTS_ENABLED  - ble_hts - Health Thermometer Service


#ifndef BLE_HTS_ENABLED
#define BLE_HTS_ENABLED 0
#endif

// <q> BLE_IAS_C_ENABLED  - ble_ias_c - Immediate Alert Service Client


#ifndef BLE_IAS_C_ENABLED
#define BLE_IAS_C_ENABLED 0
#endif

// <e> BLE_IAS_ENABLED - ble_ias - Immediate Alert Service
//==========================================================
#ifndef BLE_IAS_ENABLED
#define BLE_IAS_ENABLED 0
#endif
// <e> BLE_IAS_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef BLE_IAS_CONFIG_LOG_ENABLED
#define BLE_IAS_CONFIG_LOG_ENABLED 0
#endif
// <o> BLE_IAS_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef BLE_IAS_CONFIG_LOG_LEVEL
#define BLE_IAS_CONFIG_LOG_LEVEL 3
#endif

// <o> BLE_IAS_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef BLE_IAS_CONFIG_INFO_COLOR
#define BLE_IAS_CONFIG_INFO_COLOR 0
#endif

// <o> BLE_IAS_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef BLE_IAS_CONFIG_DEBUG_COLOR
#define BLE_IAS_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <q> BLE_LBS_C_ENABLED  - ble_lbs_c - Nordic LED Button Service Client


#ifndef BLE_LBS_C_ENABLED
#define BLE_LBS_C_ENABLED 0
#endif

// <q> BLE_LBS_ENABLED  - ble_lbs - LED Button Service


#ifndef BLE_LBS_ENABLED
#define BLE_LBS_ENABLED 0
#endif

// <q> BLE_LLS_ENABLED  - ble_lls - Link Loss Service


#ifndef BLE_LLS_ENABLED
#define BLE_LLS_ENABLED 0
#endif

// <q> BLE_NUS_C_ENABLED  - ble_nus_c - Nordic UART Central Service


#ifndef BLE_NUS_C_ENABLED
#define BLE_NUS_C_ENABLED 0
#endif

// <e> BLE_NUS_ENABLED - ble_nus - Nordic UART Service
//==========================================================
#ifndef BLE_NUS_ENABLED
#define BLE_NUS_ENABLED 0
#endif
// <e> BLE_NUS_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef BLE_NUS_CONFIG_LOG_ENABLED
#define BLE_NUS_CONFIG_LOG_ENABLED 0
#endif
// <o> BLE_NUS_CONFIG_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef BLE_NUS_CONFIG_LOG_LEVEL
#define BLE_NUS_CONFIG_LOG_LEVEL 3
#endif

// <o> BLE_NUS_CONFIG_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef BLE_NUS_CONFIG_INFO_COLOR
#define BLE_NUS_CONFIG_INFO_COLOR 0
#endif

// <o> BLE_NUS_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef BLE_NUS_CONFIG_DEBUG_COLOR
#define BLE_NUS_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </e>

// <q> BLE_RSCS_C_ENABLED  - ble_rscs_c - Running Speed and Cadence Client


#ifndef BLE_RSCS_C_ENABLED
#define BLE_RSCS_C_ENABLED 0
#endif

// <q> BLE_RSCS_ENABLED  - ble_rscs - Running Speed and Cadence Service


#ifndef BLE_RSCS_ENABLED
#define BLE_RSCS_ENABLED 0
#endif

// <q> BLE_TPS_ENABLED  - ble_tps - TX Power Service


#ifndef BLE_TPS_ENABLED
#define BLE_TPS_ENABLED 0
#endif



// </h>
//==========================================================

// <<< end of configuration section >>>
#endif //SDK_CONFIG_H