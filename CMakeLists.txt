# You can call cmake from command line by toolchain file argument   use `-DCMAKE_TOOLCHAIN_FILE=cmake/gcc.cmake`

# Project minimum required CMake version
cmake_minimum_required(VERSION 3.16.4)

# Project configuration
project(cnc_controller
	VERSION 1.0.0
	LANGUAGES C CXX ASM)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/GeneratedSrc.cmake)
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
set(LST_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.lst)

### generate hex files
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${CMAKE_CURRENT_SOURCE_DIR}/build/CMakeFiles/${RASC_PROJECT_NAME}.elf.dir/${RASC_PROJECT_NAME}.elf ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${CMAKE_CURRENT_SOURCE_DIR}/build/CMakeFiles/${RASC_PROJECT_NAME}.elf.dir/${RASC_PROJECT_NAME}.elf ${BIN_FILE}
        COMMAND ${CMAKE_OBJDUMP} -h -s -S ${CMAKE_CURRENT_SOURCE_DIR}/build/CMakeFiles/${RASC_PROJECT_NAME}.elf.dir/${RASC_PROJECT_NAME}.elf >${LST_FILE}
        COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")
