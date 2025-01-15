cmake_minimum_required(VERSION 3.13)

project(MY_APP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 và SDL2_image
find_library(SDL2 SDL2)
find_library(SDL2_image SDL2_image)

# Tạo thư viện chia sẻ
add_library(main SHARED)

# target_compile_definitions(lua PRIVATE LUA_USE_C89)

# Tìm các thư viện cần thiết
find_library(log-lib log)
find_library(jnigraphics-lib jnigraphics)
find_library(android-lib android)
find_library(EGL-lib EGL)
find_library(GLESv1_CM-lib GLESv1_CM)
find_library(GLESv2-lib GLESv2)
find_library(OpenSLES-lib OpenSLES)

# Thêm các thư viện vào target
target_link_libraries(main PRIVATE
    ${log-lib} 
    ${jnigraphics-lib}
    ${android-lib}
    ${EGL-lib}
    ${GLESv1_CM-lib}
    ${GLESv2-lib}
    ${OpenSLES-lib}
    SDL2::SDL2
    SDL2_image::SDL2_image
)
# lua
# Thêm thư mục include
target_include_directories(main PRIVATE
    ${CMAKE_SOURCE_DIR}/SDL_image/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Chipset
    ${CMAKE_SOURCE_DIR}/src/Containers
    ${CMAKE_SOURCE_DIR}/src/Gui
    ${CMAKE_SOURCE_DIR}/src/Gui/imgui
    ${CMAKE_SOURCE_DIR}/src/Peripheral
    ${CMAKE_SOURCE_DIR}/src/StartupUi
    ${CMAKE_SOURCE_DIR}/SDL/include
    ${ANDROID_NDK}/sources/android/native_app_glue
)
#  ${CMAKE_SOURCE_DIR}/lua

# Tìm tất cả các file .cpp trong thư mục src và các thư mục con
file(GLOB_RECURSE SOURCES 
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*/*/*.cpp)

target_sources(main PRIVATE ${SOURCES})

# Thêm flags biên dịch
target_compile_options(main PRIVATE -O2)

# Thêm định nghĩa để sử dụng std::filesystem
target_compile_definitions(main PRIVATE USE_STD_FILESYSTEM)