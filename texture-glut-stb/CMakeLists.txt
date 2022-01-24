cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# 設定變數
set(MY_PROJECT "texture-glut-stb")
set(MY_EXECUTABLE "texture-glut-stb")

set(INCLUDE_DIRS "include")
set(SOURCE_DIRS "src/*.cpp")

# 定義專案屬性
project (${MY_PROJECT}
    VERSION 1.0
)

# 建立二進位執行檔目標
add_executable(${MY_EXECUTABLE})

# 設定目標屬性: C++ 語言
set_target_properties(${MY_EXECUTABLE}
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# 根據不同的編譯器以及組態檔來設定編譯器 flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND CompileOptions -Wall -Wextra -pedantic-errors)
    list(APPEND CompileOptions_Debug -ggdb)
    list(APPEND CompileOptions_Release)
    target_compile_definitions(${MY_EXECUTABLE} PRIVATE $<$<CONFIG:Debug>:_GLIBCXX_DEBUG>)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    list(APPEND CompileOptions -Wall -Wextra -pedantic-errors)
    list(APPEND CompileOptions_Debug)
    list(APPEND CompileOptions_Release)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    list(APPEND CompileOptions /W3 /permissive-)
    list(APPEND CompileOptions_Debug)
    list(APPEND CompileOptions_Release)
endif()
target_compile_options(${MY_EXECUTABLE} PRIVATE ${CompileOptions})
target_compile_options(${MY_EXECUTABLE} PRIVATE $<$<CONFIG:Debug>:${CompileOptions_Debug}>)
target_compile_options(${MY_EXECUTABLE} PRIVATE $<$<CONFIG:Release>:${CompileOptions_Release}>)

# 顯示相關資訊
message(STATUS)
message(STATUS "========== Build Information ==========")
message(STATUS "\tHost System Name:         ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "\tHost System Version:      ${CMAKE_HOST_SYSTEM_VERSION}")
message(STATUS "\tHost System Processor:    ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS)
message(STATUS "\tC++ Compiler:             ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "\tC++ Compiler Path:        ${CMAKE_CXX_COMPILER}")
message(STATUS)
message(STATUS "\tVCPKG Path:               ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS)

if(MSVC)
    message(STATUS "\tMSVC Version: ${MSVC_VERSION}")
    message(STATUS "\tMSVC Toolset: ${MSVC_TOOLSET_VERSION}")
endif()

if(CMAKE_GENERATOR MATCHES "Xcode")
    message(STATUS "\tXcode Version: ${XCODE_VERSION}")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(STATUS "\tMacOS SDK: ${CMAKE_OSX_SYSROOT}")
endif()

message(STATUS "========================================")

# 引入 vcpkg 套件
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)

# 找尋所有原始檔並將其加入到【執行檔目標】中
file(GLOB_RECURSE MY_SOURCE CONFIGURE_DEPENDS ${SOURCE_DIRS})
target_sources(${MY_EXECUTABLE} PRIVATE ${MY_SOURCE})

# 將 vcpkg 的套件（函式庫）連結到【執行檔目標】
target_link_libraries(${MY_EXECUTABLE} PRIVATE
    OpenGL::GL
    OpenGL::GLU
    GLUT::GLUT
    glm::glm
    imgui::imgui
)

# 針對不同的編譯器有不同的引入設定
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${MY_EXECUTABLE} PRIVATE X11)
endif()

# 建立 Symlink 到 assets 資料夾
add_custom_command(TARGET ${MY_EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${MY_EXECUTABLE}>/assets"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    COMMENT
        "Creating symlinks to project resources..."
    VERBATIM
)