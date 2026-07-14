cmake_minimum_required(VERSION 3.16)

project(
    classical_rng
    VERSION 2.0.0
    DESCRIPTION "Deterministic and operating-system-backed randomness for C11"
    LANGUAGES C
)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

option(BUILD_SHARED_LIBS "Build classical_rng as a shared library" OFF)
option(CRNG_BUILD_EXAMPLES "Build educational examples" ON)
option(CRNG_BUILD_TOOLS "Build game_rng and crypto_rng command-line tools" ON)
option(CRNG_BUILD_TESTS "Build the test suite" ${BUILD_TESTING})
option(CRNG_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(CRNG_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_library(classical_rng
    src/game_rng/game_rng.c
    src/crypto_rng/crypto_rng.c
    src/common/status.c
)
add_library(classical_rng::classical_rng ALIAS classical_rng)

target_compile_features(classical_rng PUBLIC c_std_11)
target_include_directories(classical_rng
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_definitions(classical_rng PRIVATE CRNG_BUILDING_LIBRARY)
set_target_properties(classical_rng PROPERTIES
    C_EXTENSIONS OFF
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(classical_rng PUBLIC CRNG_SHARED)
endif()

if(WIN32)
    target_link_libraries(classical_rng PRIVATE bcrypt)
endif()

function(crng_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(CRNG_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
        if(CRNG_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

crng_set_warnings(classical_rng)

if(CRNG_ENABLE_SANITIZERS)
    if(MSVC)
        message(FATAL_ERROR "CRNG_ENABLE_SANITIZERS is supported with GCC and Clang")
    endif()
    target_compile_options(classical_rng PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    # Static-library instrumentation needs the final consumer to link runtimes.
    target_link_options(classical_rng PUBLIC -fsanitize=address,undefined)
endif()

if(CRNG_BUILD_EXAMPLES)
    foreach(example IN ITEMS constant_roundoff dice shuffle secure_token)
        add_executable(crng_${example} examples/${example}.c)
        target_link_libraries(crng_${example} PRIVATE classical_rng::classical_rng)
        crng_set_warnings(crng_${example})
    endforeach()
    foreach(example IN ITEMS particle_system terrain_generation token_generation key_derivation)
        if(example STREQUAL "particle_system" OR example STREQUAL "terrain_generation")
            set(example_source examples/game/${example}.c)
        else()
            set(example_source examples/crypto/${example}.c)
        endif()
        add_executable(crng_${example} ${example_source})
        target_link_libraries(crng_${example} PRIVATE classical_rng::classical_rng)
        crng_set_warnings(crng_${example})
    endforeach()
endif()

if(CRNG_BUILD_TOOLS)
    add_executable(game_rng_cli src/game_rng/game_rng_cli.c)
    target_link_libraries(game_rng_cli PRIVATE classical_rng::classical_rng)
    crng_set_warnings(game_rng_cli)

    add_executable(crypto_rng_cli src/crypto_rng/crypto_rng_cli.c)
    target_link_libraries(crypto_rng_cli PRIVATE classical_rng::classical_rng)
    crng_set_warnings(crypto_rng_cli)
endif()

if(CRNG_BUILD_TESTS)
    enable_testing()
    add_executable(crng_tests tests/test_classical_rng.c)
    target_link_libraries(crng_tests PRIVATE classical_rng::classical_rng)
    crng_set_warnings(crng_tests)
    if(CRNG_ENABLE_SANITIZERS)
        target_compile_options(crng_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(crng_tests PRIVATE -fsanitize=address,undefined)
    endif()
    add_test(NAME classical_rng.unit COMMAND crng_tests)

    add_executable(crng_game_module_test
        tests/test_game_rng.c
        tests/test_utils/statistical_tests.c
    )
    target_include_directories(crng_game_module_test PRIVATE tests)
    target_link_libraries(crng_game_module_test PRIVATE classical_rng::classical_rng)
    crng_set_warnings(crng_game_module_test)
    add_test(NAME classical_rng.game_module COMMAND crng_game_module_test)

    add_executable(crng_crypto_module_test
        tests/test_crypto_rng.c
        tests/test_utils/statistical_tests.c
    )
    target_include_directories(crng_crypto_module_test PRIVATE tests)
    target_link_libraries(crng_crypto_module_test PRIVATE classical_rng::classical_rng)
    crng_set_warnings(crng_crypto_module_test)
    add_test(NAME classical_rng.crypto_module COMMAND crng_crypto_module_test)

    add_executable(crng_v1_game_compat_test tests/test_v1_game_compat.c)
    target_link_libraries(crng_v1_game_compat_test PRIVATE classical_rng::classical_rng)
    crng_set_warnings(crng_v1_game_compat_test)
    add_test(NAME classical_rng.v1_game_compat COMMAND crng_v1_game_compat_test)

    add_executable(crng_v1_crypto_compat_test tests/test_v1_crypto_compat.c)
    target_link_libraries(crng_v1_crypto_compat_test PRIVATE classical_rng::classical_rng)
    crng_set_warnings(crng_v1_crypto_compat_test)
    add_test(NAME classical_rng.v1_crypto_compat COMMAND crng_v1_crypto_compat_test)

    include(CheckLanguage)
    check_language(CXX)
    if(CMAKE_CXX_COMPILER)
        enable_language(CXX)
        add_executable(crng_cpp_header_test tests/test_cpp_header.cpp)
        target_link_libraries(crng_cpp_header_test PRIVATE classical_rng::classical_rng)
        target_compile_features(crng_cpp_header_test PRIVATE cxx_std_11)
        crng_set_warnings(crng_cpp_header_test)
        add_test(NAME classical_rng.cpp_header COMMAND crng_cpp_header_test)
    endif()
endif()

install(TARGETS classical_rng
    EXPORT classical_rngTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES include/classical_rng.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/classical_rng DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT classical_rngTargets
    FILE classical_rngTargets.cmake
    NAMESPACE classical_rng::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/classical_rng
)

configure_package_config_file(
    cmake/classical_rngConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/classical_rngConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/classical_rng
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/classical_rngConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/classical_rngConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/classical_rngConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/classical_rng
)
