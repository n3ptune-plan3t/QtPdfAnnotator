# CMake minimum version requirement
cmake_minimum_required(VERSION 3.16)

# Project name and language
project(QtPdfAnnotator LANGUAGES CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic tools (moc, uic, rcc)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find the required Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Pdf)

# Add the executable target
add_executable(QtPdfAnnotator main.cpp)

# Link the executable against the Qt6 libraries
target_link_libraries(QtPdfAnnotator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Pdf
)

# Install the executable to the standard binary directory
install(TARGETS QtPdfAnnotator DESTINATION bin)

