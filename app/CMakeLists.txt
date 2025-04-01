set(APP_NAME mtp_app)


add_executable(${APP_NAME}
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)


set_target_properties(${APP_NAME} PROPERTIES AUTOMOC ON AUTOUIC ON)

target_link_libraries(${APP_NAME} PRIVATE
    Qt6::Widgets
    libmtpviewmodel
)

