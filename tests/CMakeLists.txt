# Скачиваем Catch2, если нет
include(FetchContent)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(catch2)

# Добавляем исполняемый файл тестов
add_executable(vector_tests
    vector_public_test.cpp
)

# Подключаем нашу библиотеку и Catch2
target_link_libraries(vector_tests
    PRIVATE
    custom_vector
    Catch2::Catch2WithMain
)

# Добавляем тест в CTest
add_test(NAME VectorTests COMMAND vector_tests)