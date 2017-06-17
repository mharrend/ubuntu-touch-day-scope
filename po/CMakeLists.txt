include(FindGettext)
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
find_program(INTLTOOL_UPDATE intltool-update)

set(POT_FILE ${GETTEXT_DOMAIN}.pot)
file(GLOB POFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.po)

# Creates POTFILES
configure_file(
    "make-pot.sh.in"
    "${CMAKE_SOURCE_DIR}/po/make-pot.sh"
)
configure_file(
    "update-pos-from-pot.sh.in"
    "${CMAKE_SOURCE_DIR}/po/update-pos-from-pot.sh"
)

# Builds the binary translations catalog for each language
# it finds source translations (*.po) for
foreach(POFILE ${POFILES})
    string(REPLACE ".po" "" LANG ${POFILE})
    list(APPEND PO_FILES "${POFILE}")
    set(INSTALL_DIR "${CMAKE_BINARY_DIR}/po/built_mos/locale/${LANG}/LC_MESSAGES" )
    exec_program("mkdir -p ${INSTALL_DIR}")
    exec_program("msgfmt -o  ${INSTALL_DIR}/${GETTEXT_DOMAIN}.mo ${CMAKE_SOURCE_DIR}/po/${POFILE}")
endforeach(POFILE)

INSTALL(
    DIRECTORY "built_mos/"
    DESTINATION "${SCOPE_INSTALLDIR}"
)
