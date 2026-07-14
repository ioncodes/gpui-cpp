add_executable(hello hello/main.cpp)
target_link_libraries(hello PRIVATE gpui::cpp)

if(MSVC)
    set_target_properties(hello PROPERTIES WIN32_EXECUTABLE ON)
    target_link_options(hello PRIVATE "/ENTRY:mainCRTStartup")
endif()
