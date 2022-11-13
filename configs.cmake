############### DEVICE SPECIFIC CONFIGURATIONS #####################
set(ARM_CPU cortex-m4)
set(MCU_TYPE SAM4E8C)
string(TOLOWER ${MCU_TYPE} MCU_TYPE_LOWERCASE)
set(ATMEL_ARCHITECTURE SAM4E)

################### DIRECTORIES CONFIGURATIONS #################
set(SOURCES_DIRECTORY src)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/archive")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

############## TOOLSET CONFIGURATIONS ####################
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_HOME_DIRECTORY}/sam4e.cmake)
message(DEBUG "toolchain file is  ${CMAKE_TOOLCHAIN_FILE}")
set(USE_EXTERNAL_TOOLSET TRUE)
set(USE_COMPILER_EXECUTABLE_AS_LINKER TRUE)
set(LINKER_VERBOSE_LIBRARY_INFO FALSE)
set(COMPILER_SHOW_INCLUDED_HEADERS FALSE)

if (LINKER_VERBOSE_LIBRARY_INFO)
    set(LINKER_VERBOSE_CMD "-Wl,--trace")
else ()
    set(LINKER_VERBOSE_CMD "")
endif ()

if (LINKER_VERBOSE_LIBRARY_INFO)
    set(COMPILER_SHOW_HEADERS_CMD "-H")
else ()
    set(COMPILER_SHOW_HEADERS_CMD "")
endif ()

################# COMPILER FLAGS ####################
set(C_CXX_COMPILER_OPTIONS_DEBUG "-O0 -g3 -DDEBUG")
set(C_CXX_COMPILER_OPTIONS_RELEASE "-Ofast")
set(CXX_COMPILER_OPTIONS_RELEASE "-fno-rtti")
set(COMPILER_WARNINGS_SETTINGS "-Wall -Werror=return-type -Wreturn-local-addr -Wno-volatile -Wno-unused-function -Wno-unused-variable -Wno-unused-local-typedefs ")
set(BOARD_CONFIGS_COMPILE_FLAGS "-mcpu=${ARM_CPU} -Dprintf=iprintf -D__${MCU_TYPE}__  -DBOARD=USER_BOARD  -DBOARD_FREQ_SLCK_XTAL=32768UL -DBOARD_FREQ_SLCK_BYPASS=32768UL -DBOARD_FREQ_MAINCK_XTAL=20000000UL -DBOARD_FREQ_MAINCK_BYPASS=20000000UL -DBOARD_OSC_STARTUP_US=15625UL")
set(FPU_SETTINGS_COMPILE_FLAGS "-DARM_MATH_CM4=true -mfloat-abi=softfp -mfpu=fpv4-sp-d16")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(C_CXX_CONDITIONAL_COMPILE_FLAGS ${C_CXX_COMPILER_OPTIONS_DEBUG})
    set(CXX_CONDITIONAL_COMPILE_FLAGS ${CXX_COMPILER_OPTIONS_DEBUG})
else ()
    set(C_CXX_CONDITIONAL_COMPILE_FLAGS ${C_CXX_COMPILER_OPTIONS_RELEASE})
endif ()

set(C_CXX_COMPILE_FLAGS "${COMPILER_WARNINGS_SETTINGS} ${BOARD_CONFIGS_COMPILE_FLAGS} ${COMPILER_SHOW_HEADERS_CMD} ${C_CXX_CONDITIONAL_COMPILE_FLAGS} ${FPU_SETTINGS_COMPILE_FLAGS} -mthumb --param max-inline-insns-single=500 -ffunction-sections -mlong-calls -c")
set(C_COMPILE_FLAGS "-x c -std=gnu17")
set(CXX_COMPILE_FLAGS "${CXX_CONDITIONAL_COMPILE_FLAGS} -D__FPU_PRESENT -mthumb -fconcepts -fconcepts-diagnostics-depth=2 -fno-exceptions -std=gnu++20")

####################### LINKER SCRIPT ##########################
set(LINKER_SCRIPT_DIR "${CMAKE_SOURCE_DIR}/src/asf_lib/src/ASF/sam/utils/linker_scripts/sam4e/sam4e8/gcc")
set(LINKER_SCRIPT_FILENAME flash.ld)
set(LINKER_SCRIPT_COMMAND "-L\"${LINKER_SCRIPT_DIR}\" -T${LINKER_SCRIPT_FILENAME}")
set(LINKER_MAP_GEN_COMMAND "-Wl,-Map=\"${PROJECT_NAME}.map\"")
set(MY_LINK_FLAGS "${LINKER_VERBOSE_CMD} --specs=nosys.specs -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -mthumb -mcpu=cortex-m4 ${LINKER_MAP_GEN_COMMAND} -Wl,--start-group -lm -Wl,--end-group -Wl,--gc-sections ${LINKER_SCRIPT_COMMAND}")
message(DEBUG "link flags: ${MY_LINK_FLAGS}")
