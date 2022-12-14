cmake_minimum_required(VERSION 3.8)
include(cmake_sources.cmake)
include(configs.cmake)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_HOME_DIRECTORY}/sam4e.cmake)
message(DEBUG "toolchain file is  ${CMAKE_TOOLCHAIN_FILE}")

set(SOURCES_DIRECTORY src)

project("ClampMeter" LANGUAGES C CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/archive")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(ARM_CPU cortex-m4)
set(MCU_TYPE SAM4E8C)
string(TOLOWER ${MCU_TYPE} MCU_TYPE_LOWERCASE)

set(ATMEL_ARCHITECTURE SAM4E)

set(EXECUTABLE_NAME ${PROJECT_NAME})


set(C_CXX_COMPILE_FLAGS "${COMPILER_SHOW_HEADERS_CMD} -mthumb -D__${MCU_TYPE}__  -DBOARD=USER_BOARD -DARM_MATH_CM4=true -Dprintf=iprintf -ffast-math --param max-inline-insns-single=500 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -DDEBUG -O3 -ffunction-sections -mlong-calls -g3 -Wall -mcpu=${ARM_CPU} -c")
set(C_COMPILE_FLAGS "-x c -std=gnu11")
set(CXX_COMPILE_FLAGS "-fno-rtti -fno-exceptions -std=c++14")

#set(LINKER_SCRIPT_DIR "C:/Program Files (x86)/Atmel/Studio/7.0/packs/atmel/SAM4E_DFP/1.1.57/ic.sam4e/gcc/gcc")
set(LINKER_SCRIPT_DIR "${CMAKE_SOURCE_DIR}/src/asf_lib/src/ASF/sam/utils/linker_scripts/sam4e/sam4e8/gcc")
#set(LINKER_SCRIPT_FILENAME ${MCU_TYPE_LOWERCASE}_flash.ld)
set(LINKER_SCRIPT_FILENAME flash.ld)
set(LINKER_SCRIPT_COMMAND "-L\"${LINKER_SCRIPT_DIR}\" -T${LINKER_SCRIPT_FILENAME}")
set(LINKER_MAP_GEN_COMMAND "-Wl,-Map=\"${PROJECT_NAME}.map\"")

set(MY_LINK_FLAGS "${LINKER_VERBOSE_CMD} --specs=nosys.specs -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -mthumb -mcpu=cortex-m4 ${LINKER_MAP_GEN_COMMAND} -Wl,--start-group -lm -Wl,--end-group -Wl,--gc-sections ${LINKER_SCRIPT_COMMAND}")
message(DEBUG "link flags: ${MY_LINK_FLAGS}")

set(CXX_SOURCES ${SOURCES_DIRECTORY}/main.cpp)

set_source_files_properties(${CXX_SOURCES} PROPERTIES COMPILE_FLAGS "${C_CXX_COMPILE_FLAGS} ${CXX_COMPILE_FLAGS}")


add_sam_executable(${EXECUTABLE_NAME} ${CXX_SOURCES})

add_subdirectory(src/asf_lib)
target_link_libraries(${EXECUTABLE_NAME} PUBLIC asf)

add_subdirectory(src/clamp_meter)
target_link_libraries(${EXECUTABLE_NAME} PUBLIC clamp_meter)

add_subdirectory(src/test)
target_link_libraries(${EXECUTABLE_NAME} PUBLIC mytestlib)
#link_libraries(clamp_meter PUBLIC asf)