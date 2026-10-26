cmake_minimum_required(VERSION 3.16)
project(FOV LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# 查找 Qt5 和 OpenGL（Qt 建议用 find_package）
find_package(Qt5 5.12.11 REQUIRED COMPONENTS Core Gui Widgets Svg)

# 预编译库的路径（假设位于项目根目录下的 x64/Release）
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/x64/Release")
 
# 导入特定的库文件（非 VTK 部分）
set(QWT_LIB          ${LIB_DIR}/qwt.lib)
set(PROPERTY_LIB     ${LIB_DIR}/property.lib)
set(QT5_LIB     ${LIB_DIR}/Qt5Core.lib
    ${LIB_DIR}/Qt5Core.lib
    ${LIB_DIR}/Qt5Gui.lib
    ${LIB_DIR}/Qt5Widgets.lib
    ${LIB_DIR}/Qt5Svg.lib
)

# ---- 源文件（位于 fov 目录下）----
set(SOURCES
    fov/FieldViewWidget.cpp
    fov/PlotBase.cpp
    fov/PlotSetting.cpp
    fov/QwtPropertyBrowser.cpp
    fov/StringEditorFactory.cpp
    fov/StringEditWithButton.cpp
	fov/CircleViewWidget.cpp
	fov/ConeViewWidget.cpp
	fov/FieldGlyphWidget.cpp
	fov/LineViewWidget.cpp
	fov/mainwindow.cpp	
	fov/main.cpp
	fov/JonesViewWidget.cpp
	fov/PolarizationItem.cpp
)

# ---- 头文件（位于 fov 目录下）----
set(HEADERS
    fov/FieldViewWidget.h
    fov/PlotBase.h
    fov/PlotSetting.h
    fov/QwtPropertyBrowser.h
    fov/StringEditorFactory.h
    fov/StringEditWithButton.h
	fov/mainwindow.h
	fov/CircleViewWidget.h
	fov/ConeViewWidget.h
	fov/FieldGlyphWidget.h
	fov/LineViewWidget.h
	fov/main.h
	fov/JonesViewWidget.h
	fov/PolarizationItem.h
)

# ---- UI 文件（位于 fov 目录下）----
set(UI_FILES

)

# ---- 资源文件（假设位于项目根目录）----
set(RESOURCES

)


add_executable(fov WIN32 ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCES})

set_target_properties(fov PROPERTIES AUTOGEN_SOURCE_GROUP "moc")

# 将 .qrc 文件放到 "Resource Files"
foreach(qrc_file ${RESOURCES})
    source_group("Resource Files" FILES ${qrc_file})
endforeach()

# 将 .ui 文件放到 "Form Files"
foreach(ui_file ${UI_FILES})
    source_group("Form Files" FILES ${ui_file})
endforeach()


# 包含头文件路径（请根据实际目录调整）
target_include_directories(fov PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/fov
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qwt
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/PropertyBrowser
)
include_directories(${QT5_INCLUDE_DIR})
target_compile_definitions(fov PRIVATE
    QUARTER_DLL
    COIN_DLL
    USE_QT
    NOMINMAX
    QWT_DLL
    _WINDOWS
    UNICODE
    _UNICODE
    WIN32
    _ENABLE_EXTENDED_ALIGNED_STORAGE
    WIN64
    QT_DISABLE_DEPRECATED_BEFORE=0
    NDEBUG
    QT_NO_DEBUG
    QT_QTPROPERTYBROWSER_IMPORT
)

# ---- 编译选项 ----
target_compile_options(fov PRIVATE
    /openmp
    /utf-8
    /wd4577
    /wd4467
)

# 设置子系统为控制台
target_link_options(fov PRIVATE /SUBSYSTEM:CONSOLE)
# 针对 Release 模式添加优化链接选项
target_link_options(fov PRIVATE
    $<$<CONFIG:Release>:/OPT:REF>
    $<$<CONFIG:Release>:/OPT:ICF>
)

# 链接库
target_link_libraries(fov
        Qt5::Core
        Qt5::Widgets
        Qt5::Svg
    ${QWT_LIB}
    ${PROPERTY_LIB}
)

# 设置输出目录
set_target_properties(fov PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)



set(QT_BIN_DIR ${LIB_DIR} CACHE PATH "Directory containing qt.dll")
set(QWT_DLL_DIR ${LIB_DIR} CACHE PATH "Directory containing qwt.dll")
set(PROPERTY_DLL_DIR ${LIB_DIR} CACHE PATH "Directory containing property.dll")

if(WIN32)
    # ------------------- Qt5 DLL -------------------
if(EXISTS ${QT_BIN_DIR})
    install(FILES
        ${QT_BIN_DIR}/Qt5Core.dll
        ${QT_BIN_DIR}/Qt5Gui.dll
        ${QT_BIN_DIR}/Qt5OpenGL.dll
        ${QT_BIN_DIR}/Qt5Widgets.dll
        ${QT_BIN_DIR}/Qt5Svg.dll
        DESTINATION bin
    )
endif()

install(TARGETS fov
    RUNTIME DESTINATION bin   # 安装到 ${CMAKE_INSTALL_PREFIX}/bin
)



    # ------------------- Qwt DLL -------------------
    # 假设 Qwt 的 DLL 位于项目 x64/Release 目录（或 qwt 安装目录）
    if(EXISTS ${QWT_DLL_DIR}/qwt.dll)
        install(FILES ${QWT_DLL_DIR}/qwt.dll DESTINATION bin)
    else()
        message(WARNING "qwt.dll not found in ${QWT_BIN_DIR}")
    endif()
    

    # ------------------- 第三方库 DLL 可选 -------------------
    # 例如 property.dll 等（如果它们也是动态库）
    if(EXISTS ${PROPERTY_DLL_DIR}/property.dll)
        install(FILES ${PROPERTY_DLL_DIR}/property.dll DESTINATION bin)
    endif()


    set(QT_PLUGIN_SUBDIRS iconengines imageformats platforms styles)

    foreach(subdir ${QT_PLUGIN_SUBDIRS})
        set(src_dir "${LIB_DIR}/${subdir}")
        if(EXISTS ${src_dir})
            install(DIRECTORY ${src_dir} DESTINATION bin)
        else()
            message(WARNING "Qt plugin directory not found: ${src_dir}")
        endif()
    endforeach()
endif()

set(THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty")
# 安装 PropertyBrowser 头文件
if(EXISTS "${THIRDPARTY_DIR}/PropertyBrowser/")
    install(DIRECTORY "${THIRDPARTY_DIR}/PropertyBrowser/"
        DESTINATION include/PropertyBrowser
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl"
    )
endif()

# 安装 qwt 头文件
if(EXISTS "${THIRDPARTY_DIR}/qwt/")
    install(DIRECTORY "${THIRDPARTY_DIR}/qwt/"
        DESTINATION include/qwt
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl"
    )
endif()



install(FILES ${QWT_LIB} ${PROPERTY_LIB} DESTINATION lib)


