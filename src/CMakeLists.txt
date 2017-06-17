add_library(${SO_NAME} SHARED
    preview.cpp
    query.cpp
    scope.cpp
)

add_definitions(
  -DGETTEXT_PACKAGE=\"${PROJECT_NAME}\"
  -DGETTEXT_LOCALEDIR=\"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LOCALEDIR}\"
)

include_directories(${UNITY_SCOPES_INCLUDE_DIRS})

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
qt5_use_modules(${SO_NAME} Core Network)

target_link_libraries(${SO_NAME} ${UNITY_SCOPES_LDFLAGS})

install(
  TARGETS ${SO_NAME}
  LIBRARY DESTINATION "${SCOPE_INSTALLDIR}"
)
