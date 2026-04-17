cmake_minimum_required(VERSION 4.2)
project(Tufan)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TUFAN_BUILD_TESTS    "Собирать тесты"          ON)
option(TUFAN_BUILD_TOOLS    "Собирать CLI инструменты" ON)
option(TUFAN_ENABLE_TLS     "Включить OpenSSL/TLS"     ON)
option(TUFAN_SANITIZERS     "Включить ASan/UBSan"      OFF)

set(ENGINE_DIR  ${CMAKE_SOURCE_DIR}/engine)
set(KERNEL_DIR  ${CMAKE_SOURCE_DIR}/kernel)
set(CONFIG_DIR  ${CMAKE_SOURCE_DIR}/config)
set(TESTS_DIR   ${CMAKE_SOURCE_DIR}/tests)

if(TUFAN_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

find_package(Boost REQUIRED COMPONENTS
        system
        filesystem
        thread
        regex
        log
        log_setup
        program_options
        json
        url
)

if(TUFAN_ENABLE_TLS)
    find_package(OpenSSL REQUIRED)
endif()

find_package(Threads REQUIRED)

if(TUFAN_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_library(TufanCore STATIC
        src/core/scanner/PortScanner.cpp
        src/core/net/AsyncClient.cpp
        src/core/crypto/HashUtils.cpp
        src/core/log/Logger.cpp
)

target_include_directories(TufanCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(TufanCore PUBLIC
        Boost::system
        Boost::filesystem
        Boost::thread
        Boost::regex
        Boost::log
        Boost::log_setup
        Boost::json
        Threads::Threads
        $<$<BOOL:${TUFAN_ENABLE_TLS}>:OpenSSL::SSL>
        $<$<BOOL:${TUFAN_ENABLE_TLS}>:OpenSSL::Crypto>
)
add_executable(Tufan main.cc
        engine/Tufan.hh
        engine/foundation/IContainer.hh
        engine/exceptions/foundation_exceptions.hh
        engine/exceptions/config_exceptions.hh
        engine/foundation/IProvider.hh
        engine/foundation/Binding.hh
        engine/foundation/Container.hh
        engine/foundation/Container.cc
        engine/configuration/EnvParser.hh
        engine/configuration/EnvParser.cc
        engine/configuration/Repository.hh
        engine/foundation/Application.hh
        engine/foundation/Application.cc
        config/app.cc
        config/security.cc
        config/logging.cc
        engine/testing/unit/configuration/RepositoryTest.cc
        engine/testing/unit/foundation/ApplicationTest.cc
        engine/foundation/ServiceRegistry.hh
        engine/foundation/ServiceRegistry.hh
        engine/logger/Logger.hh
        engine/logger/Logger.cc
        engine/exceptions/Exceptions.hh
        engine/exceptions/Handler.hh
        engine/exceptions/Handler.cc
        engine/console/Command.hh
        engine/console/Kernel.hh
        engine/console/Parser.hh
        engine/console/Parser.cc
        engine/console/Kernel.cc
        engine/facades/Facade.hh
        engine/facades/Config.hh
        engine/facades/Registry.hh
        engine/facades/Logger.hh
        engine/testing/TestCase.hh
        engine/testing/unit/foundation/RegistryTest.cc
        engine/services/ServiceProvider.hh
        engine/services/ServiceProvider.cc
        engine/services/ServiceManager.hh
        engine/services/ServiceManager.cc
        engine/logger/LoggerServiceProvider.hh
        engine/foundation/ScannerServiceProvider.hh
        engine/foundation/ReportServiceProvider.hh
        engine/testing/unit/foundation/ServiceProviderTest.cc)
target_link_libraries(Tufan PRIVATE TufanCore Boost::program_options)
add_subdirectory(src/modules/scanner)
add_subdirectory(src/modules/analyzer)

if(TUFAN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()









