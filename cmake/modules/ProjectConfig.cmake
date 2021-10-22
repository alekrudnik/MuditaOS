# set LOG_USE_COLOR
if((${PROJECT_TARGET} STREQUAL "TARGET_RT1051"))
    set (LOG_USE_COLOR 0 CACHE INTERNAL "")
else()
    set (LOG_USE_COLOR 1 CACHE INTERNAL "")
endif()

# add LOG_SENSITIVE_DATA enable option
option(LOG_SENSITIVE_DATA "LOG_SENSITIVE_DATA" OFF)
if (${LOG_SENSITIVE_DATA} STREQUAL "ON")
    set (LOG_SENSITIVE_DATA_ENABLED 1 CACHE INTERNAL "")
else()
    set (LOG_SENSITIVE_DATA_ENABLED 0 CACHE INTERNAL "")
endif()

# add SystemView enable option
option(SYSTEMVIEW "SYSTEMVIEW" OFF)
if((${PROJECT_TARGET} STREQUAL "TARGET_RT1051") AND (${SYSTEMVIEW} STREQUAL "ON"))
    set (SYSTEM_VIEW_ENABLED 1 CACHE INTERNAL "")
    set (LOG_REDIRECT "RTT_SYSTEMVIEW" CACHE INTERNAL "")
else()
    set (SYSTEMVIEW 0FF CACHE INTERNAL "")
    set (SYSTEM_VIEW_ENABLED 0 CACHE INTERNAL "")
    set (LOG_REDIRECT "RTT_JLINK" CACHE INTERNAL "")
endif()

# add USB-CDC echo test enable option
option(USBCDC_ECHO "USBCDC_ECHO" OFF)
if (${USBCDC_ECHO} STREQUAL "ON")
    set (USBCDC_ECHO_ENABLED 1 CACHE INTERNAL "")
else()
    set (USBCDC_ECHO_ENABLED 0 CACHE INTERNAL "")
endif()

#add Debug LUART enable option
if (${LOG_REDIRECT} STREQUAL "RTT_LUART")
    set (LOG_LUART_ENABLED 1 CACHE INTERNAL "")
else()
    set (LOG_LUART_ENABLED 0 CACHE INTERNAL "")
endif()

# Config option for the lwext4
# LWEXT4 debug options
option(LWEXT4_DEBUG_PRINTF "LWEXT4 debug printf enable" ON)
option(LWEXT4_DEBUG_ASSERT "LWEXT4 assert printf enable" OFF)
# LWEXT4 sectors cache size
set(LWEXT4_CACHE_SIZE 256 CACHE INTERNAL "")

# add Development Configuration option
option(WITH_DEVELOPMENT_FEATURES "Include development features" OFF)
set(DEVELOPER_SETTINGS_OPTIONS_DEFAULT ${WITH_DEVELOPMENT_FEATURES} CACHE INTERNAL "")
set(ENABLE_DEVELOPER_MODE_ENDPOINT_DEFAULT ${WITH_DEVELOPMENT_FEATURES} CACHE INTERNAL "")
if (${WITH_DEVELOPMENT_FEATURES} STREQUAL "ON")
    set (LOG_SENSITIVE_DATA_ENABLED 1 CACHE INTERNAL "")
endif()

# Enable/disable USB MTP
option(ENABLE_USB_MTP "Enables usage of USB MTP" ON)

# add Mudita USB Vendor/Product IDs
option(MUDITA_USB_ID "Enables using Mudita registered USB Vendor ID and Pure Phone USB Product ID" ON)

#Config options described in README.md
set(PROJECT_CONFIG_DEFINITIONS
        LOG_USE_COLOR=${LOG_USE_COLOR}
        LOG_SENSITIVE_DATA_ENABLED=${LOG_SENSITIVE_DATA_ENABLED}
        LOG_REDIRECT=${LOG_REDIRECT}
        SYSTEM_VIEW_ENABLED=${SYSTEM_VIEW_ENABLED}
        USBCDC_ECHO_ENABLED=${USBCDC_ECHO_ENABLED}
        LOG_LUART_ENABLED=${LOG_LUART_ENABLED}
        MAGIC_ENUM_RANGE_MAX=256
        CACHE INTERNAL ""
        )

message(STATUS "BlueKitchen selected")
set(BT_STACK "BlueKitchen")
