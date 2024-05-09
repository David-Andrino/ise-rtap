
/*
 * Auto generated Run-Time-Environment Configuration File
 *      *** Do not modify ! ***
 *
 * Project: 'RTAP' 
 * Target:  'Target 1' 
 */

#ifndef RTE_COMPONENTS_H
#define RTE_COMPONENTS_H


/*
 * Define the Device Header File: 
 */
#define CMSIS_device_header "stm32f7xx.h"

/* ARM::CMSIS:RTOS2:Keil RTX5:Source:5.5.3 */
#define RTE_CMSIS_RTOS2                 /* CMSIS-RTOS2 */
        #define RTE_CMSIS_RTOS2_RTX5            /* CMSIS-RTOS2 Keil RTX5 */
        #define RTE_CMSIS_RTOS2_RTX5_SOURCE     /* CMSIS-RTOS2 Keil RTX5 Source */
/* Keil.ARM Compiler::Compiler:Event Recorder:DAP:1.5.1 */
#define RTE_Compiler_EventRecorder
          #define RTE_Compiler_EventRecorder_DAP
/* Keil.ARM Compiler::Compiler:I/O:File:File System:1.2.0 */
#define RTE_Compiler_IO_File            /* Compiler I/O: File */
          #define RTE_Compiler_IO_File_FS         /* Compiler I/O: File (File System) */
/* Keil.ARM Compiler::Compiler:I/O:STDOUT:ITM:1.2.0 */
#define RTE_Compiler_IO_STDOUT          /* Compiler I/O: STDOUT */
          #define RTE_Compiler_IO_STDOUT_ITM      /* Compiler I/O: STDOUT ITM */
/* Keil.MDK-Pro::File System:CORE:LFN Debug:6.14.1 */
#define RTE_FileSystem_Core             /* File System Core */
          #define RTE_FileSystem_LFN              /* File System with Long Filename support */
          #define RTE_FileSystem_Debug            /* File System Debug Version */
/* Keil.MDK-Pro::File System:Drive:Memory Card:6.14.1 */
#define RTE_FileSystem_Drive_MC_0       /* File System Memory Card Drive 0 */

/* Keil.MDK-Pro::Network:CORE:IPv4/IPv6 Release:7.15.0 */
#define RTE_Network_Core                /* Network Core */
          #define RTE_Network_IPv4                /* Network IPv4 Stack */
          #define RTE_Network_IPv6                /* Network IPv6 Stack */
          #define RTE_Network_Release             /* Network Release Version */
/* Keil.MDK-Pro::Network:Interface:ETH:7.15.0 */
#define RTE_Network_Interface_ETH_0     /* Network Interface ETH 0 */

/* Keil.MDK-Pro::Network:Service:Web Server Compact:HTTP:7.15.0 */
#define RTE_Network_Web_Server_RO       /* Network Web Server with Read-only Web Resources */
/* Keil.MDK-Pro::Network:Socket:TCP:7.15.0 */
#define RTE_Network_Socket_TCP          /* Network Socket TCP */
/* Keil.MDK-Pro::Network:Socket:UDP:7.15.0 */
#define RTE_Network_Socket_UDP          /* Network Socket UDP */
/* Keil::CMSIS Driver:Ethernet MAC:1.12.0 */
#define RTE_Drivers_ETH_MAC0            /* Driver ETH_MAC0 */
/* Keil::CMSIS Driver:Ethernet PHY:LAN8742A:1.3.0 */
#define RTE_Drivers_PHY_LAN8742A        /* Driver PHY LAN8742A */
/* Keil::CMSIS Driver:I2C:1.13.0 */
#define RTE_Drivers_I2C1                /* Driver I2C1 */
        #define RTE_Drivers_I2C2                /* Driver I2C2 */
        #define RTE_Drivers_I2C3                /* Driver I2C3 */
        #define RTE_Drivers_I2C4                /* Driver I2C4 */
/* Keil::CMSIS Driver:MCI:1.13.0 */
#define RTE_Drivers_MCI0                /* Driver MCI0 */
        #define RTE_Drivers_MCI1                /* Driver MCI1 */
/* Keil::CMSIS Driver:USART:1.21.0 */
#define RTE_Drivers_USART1              /* Driver USART1 */
        #define RTE_Drivers_USART2              /* Driver USART2 */
        #define RTE_Drivers_USART3              /* Driver USART3 */
        #define RTE_Drivers_USART4              /* Driver USART4 */
        #define RTE_Drivers_USART5              /* Driver USART5 */
        #define RTE_Drivers_USART6              /* Driver USART6 */
        #define RTE_Drivers_USART7              /* Driver USART7 */
        #define RTE_Drivers_USART8              /* Driver USART8 */
/* Keil::Device:STM32Cube Framework:Classic:1.3.0 */
#define RTE_DEVICE_FRAMEWORK_CLASSIC
/* Keil::Device:STM32Cube HAL:ADC:1.3.0 */
#define RTE_DEVICE_HAL_ADC
/* Keil::Device:STM32Cube HAL:Common:1.3.0 */
#define RTE_DEVICE_HAL_COMMON
/* Keil::Device:STM32Cube HAL:Cortex:1.3.0 */
#define RTE_DEVICE_HAL_CORTEX
/* Keil::Device:STM32Cube HAL:DAC:1.3.0 */
#define RTE_DEVICE_HAL_DAC
/* Keil::Device:STM32Cube HAL:DMA2D:1.3.0 */
#define RTE_DEVICE_HAL_DMA2D
/* Keil::Device:STM32Cube HAL:DMA:1.3.0 */
#define RTE_DEVICE_HAL_DMA
/* Keil::Device:STM32Cube HAL:DSI:1.3.0 */
#define RTE_DEVICE_HAL_DSI
/* Keil::Device:STM32Cube HAL:GPIO:1.3.0 */
#define RTE_DEVICE_HAL_GPIO
/* Keil::Device:STM32Cube HAL:I2C:1.3.0 */
#define RTE_DEVICE_HAL_I2C
/* Keil::Device:STM32Cube HAL:LTDC:1.3.0 */
#define RTE_DEVICE_HAL_LTDC
/* Keil::Device:STM32Cube HAL:MMC:1.3.0 */
#define RTE_DEVICE_HAL_MMC
/* Keil::Device:STM32Cube HAL:PWR:1.3.0 */
#define RTE_DEVICE_HAL_PWR
/* Keil::Device:STM32Cube HAL:RCC:1.3.0 */
#define RTE_DEVICE_HAL_RCC
/* Keil::Device:STM32Cube HAL:RTC:1.3.0 */
#define RTE_DEVICE_HAL_RTC
/* Keil::Device:STM32Cube HAL:SD:1.3.0 */
#define RTE_DEVICE_HAL_SD
/* Keil::Device:STM32Cube HAL:SDRAM:1.3.0 */
#define RTE_DEVICE_HAL_SDRAM
/* Keil::Device:STM32Cube HAL:TIM:1.3.0 */
#define RTE_DEVICE_HAL_TIM
/* Keil::Device:STM32Cube LL:Common:1.3.0 */
#define RTE_DEVICE_LL_COMMON
/* Keil::Device:STM32Cube LL:PWR:1.3.0 */
#define RTE_DEVICE_LL_PWR
/* Keil::Device:STM32Cube LL:RCC:1.3.0 */
#define RTE_DEVICE_LL_RCC
/* Keil::Device:STM32Cube LL:UTILS:1.3.0 */
#define RTE_DEVICE_LL_UTILS
/* Keil::Device:Startup:1.2.5 */
#define RTE_DEVICE_STARTUP_STM32F7XX    /* Device Startup for STM32F7 */
/* LVGL.LVGL9::LVGL:Display:ILI9341:9.1.0 */
/* use display driver for ILI9341 */
#define LV_USE_ILI9341	    1
/* LVGL.LVGL9::LVGL:Essential:9.1.0 */
/*! \brief Enable LVGL */
#define RTE_GRAPHICS_LVGL


#endif /* RTE_COMPONENTS_H */
