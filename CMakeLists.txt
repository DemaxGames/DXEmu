# CMakeList.txt: проект CMake для DXEmu; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("DXEmu")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (DXEmu "DXEmu.cpp" "DXEmu.h")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
