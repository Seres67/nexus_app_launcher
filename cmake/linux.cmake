if(UNIX)
    set(CMAKE_SYSTEM_NAME Windows)
    set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

    find_program(CMAKE_C_COMPILER NAMES ${TOOLCHAIN_PREFIX}-gcc ${TOOLCHAIN_PREFIX}-clang)
    find_program(CMAKE_CXX_COMPILER NAMES ${TOOLCHAIN_PREFIX}-g++ ${TOOLCHAIN_PREFIX}-c++ ${TOOLCHAIN_PREFIX}-clang++)
    find_program(CMAKE_Fortran_COMPILER NAMES ${TOOLCHAIN_PREFIX}-gfortran)
    find_program(CMAKE_RC_COMPILER NAMES ${TOOLCHAIN_PREFIX}-windres)
endif()
