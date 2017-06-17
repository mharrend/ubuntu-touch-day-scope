configure_file(
  "scope.ini.in.in"
  "${CMAKE_BINARY_DIR}/src/scope.ini"
)
#this is the bases for localation
configure_file(
  "scope.ini.in.in"
  "${CMAKE_SOURCE_DIR}/data/scope.ini.in"
)
file(GLOB INIS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.ini.in)
foreach(INI ${INIS})
    string(REPLACE ".ini.in" ".ini" INIFINAL ${INI})
    exec_program("intltool-merge -d -u -q ${CMAKE_SOURCE_DIR}/po/ ${CMAKE_CURRENT_SOURCE_DIR}/${INI} ${CMAKE_CURRENT_BINARY_DIR}/${INIFINAL}")
endforeach(INI)

INSTALL(
    FILES "scope.ini"
    DESTINATION "${SCOPE_INSTALLDIR}"
    RENAME "${PKG_PREFIX}.${PKG}_${APP}.ini"
)

INSTALL(DIRECTORY "images" DESTINATION "${SCOPE_INSTALLDIR}")
