project (tutorial2)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include_directories(${KDE4_INCLUDES})
set(tutorial1_SRCS main.cpp)
kde4_add_executable(tutorial2 ${tutorial1_SRCS})
target_link_libraries(tutorial2 ${KDE4_KDEUI_LIBS} phonon)
install(TARGETS tutorial2  ${INSTALL_TARGETS_DEFAULT_ARGS})
