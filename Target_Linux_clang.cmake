set(PROJECT_TARGET "TARGET_Linux" CACHE INTERNAL "")

set(TARGET_SOURCES CACHE INTERNAL "")

set(TARGET_DIR_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/board/linux  CACHE INTERNAL "")

set(TARGET_COMPILE_FEATURES CACHE INTERNAL "")

set(TARGET_COMPILE_DEFINITIONS CACHE INTERNAL "")
set(CMAKE_C_COMPILER "/home/niepiekm/work/staging/llvm-project/build/bin/templight")
set(CMAKE_CXX_COMPILER "/home/niepiekm/work/staging/llvm-project/build/bin/templight++")


add_compile_options(-funsigned-char -Wno-unknown-warning-option)

set(TARGET_LIBRARIES
    rt
    pthread
    portaudio
    CACHE INTERNAL "" )

option (LINUX_ENABLE_SANITIZER "Enable address sanitizer for Linux" OFF)
if (LINUX_ENABLE_SANITIZER)
    add_compile_options(-fsanitize=address -static-libasan)
    add_link_options(-fsanitize=address -static-libasan)
endif (LINUX_ENABLE_SANITIZER)

set(CMAKE_STRIP strip CACHE INTERNAL "")
set(CMAKE_OBJCOPY objcopy CACHE INTERNAL "")
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/sys)
file(CREATE_LINK ${CMAKE_BINARY_DIR}/user ${CMAKE_BINARY_DIR}/sys/user SYMBOLIC)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/sys/current)
file(CREATE_LINK ${CMAKE_BINARY_DIR}/assets ${CMAKE_BINARY_DIR}/sys/current/assets SYMBOLIC)

