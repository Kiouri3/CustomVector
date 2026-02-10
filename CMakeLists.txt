cmake_minimum_required(VERSION 3.15)
project(CustomVector VERSION 1.0.0 LANGUAGES CXX)

# Настройки компилятора
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Политики
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

# Опции проекта
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Основная библиотека (header-only)
add_library(custom_vector INTERFACE)
target_include_directories(custom_vector INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Тесты
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Установка (опционально)
install(DIRECTORY include/ DESTINATION include)
install(TARGETS custom_vector EXPORT CustomVectorTargets)