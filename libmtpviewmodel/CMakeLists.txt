
set(LIB_NAME libmtpviewmodel)

add_library(${LIB_NAME} STATIC
    mtpdevice.cpp
    mtpdevice.h
    mtpviewmodel.cpp
    mtpviewmodel.h
    stubmtpdevice.h
    mtpdeviceadapter.h
    imtdevice.h
)


target_include_directories(${LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(${LIB_NAME} PRIVATE
    Qt6::Core
    Qt6::Concurrent
    Qt6::Gui
    Qt6::Widgets
    ${LIBMTP_LIBRARY}
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)
set_target_properties(${LIB_NAME} PROPERTIES AUTOMOC ON)
