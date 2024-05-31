cmake_minimum_required(VERSION 3.27)
project(ProjektGrafika)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ustawienie zmiennej środowiskowej wxWidgets_CONFIG_EXECUTABLE na lokalizację pliku konfiguracyjnego wx-config
# dla zainstalowanej wersji wxWidgets
set(wxWidgets_CONFIG_EXECUTABLE "/opt/homebrew/Cellar/wxwidgets/3.2.4/bin/wx-config")

# Znajdowanie pakietu wxWidgets
find_package(wxWidgets REQUIRED core base OPTIONAL_COMPONENTS net)
include(${wxWidgets_USE_FILE})

# Dodanie plików źródłowych projektu
add_executable(ProjektGrafika main.cpp
        ../src/MyFrame.cpp
        ../src/GUIMyFrame.cpp


)


include_directories(${PROJECT_SOURCE_DIR}/include)

# Linkowanie aplikacji z bibliotekami wxWidgets
target_link_libraries(ProjektGrafika ${wxWidgets_LIBRARIES})