APP_NAME := lab1-io

# paths
# DE2-35 sources
CORE_FILE := ../../hardware/DE2-pre-built/DE2_Nios2System.sopcinfo
SOF_FILE  := ../../hardware/DE2-pre-built/IL2206_DE2_Nios2.sof
JDI_FILE  := ../../hardware/DE2-pre-built/IL2206_DE2_Nios2.jdi
# DE2-115 sources
# CORE_FILE := ../../hardware/DE2-115-pre-built/DE2_115_Nios2System.sopcinfo
# SOF_FILE  := ../../hardware/DE2-115-pre-built/IL2206_DE2_115_Nios2.sof
# JDI_FILE  := ../../hardware/DE2-115-pre-built/IL2206_DE2_115_Nios2.jdi

BSP_PATH := ../../bsp/il2206-pre-built
SRC_PATH := src
GEN_PATH := gen
BIN_PATH := bin


# component names
CPU_NAME := nios2
BSP_TYPE := hal

# commands
NIOS2_BSP_COMMANDS := --set hal.make.bsp_cflags_debug -g \
		  --set hal.make.bsp_cflags_optimization -O0 \
		  --set hal.enable_sopc_sysid_check 1 \
		  --set hal.max_file_descriptors 4

MAKEFILE_COMMANDS := --set APP_CFLAGS_OPTIMIZATION -O0

include ../Makefile.in
