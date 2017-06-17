set(VERSION "1.5.2")

# Supress qDebug() output
ADD_DEFINITIONS( -DQT_NO_DEBUG_OUTPUT )

project(day CXX)
cmake_minimum_required(VERSION 2.8.12)

# This command figures out the target architecture and puts it into the manifest file
execute_process(
  COMMAND dpkg-architecture -qDEB_HOST_ARCH
  OUTPUT_VARIABLE ARCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -s")
set(UBUNTU_PROJECT_TYPE "Scope" CACHE INTERNAL "Tells QtCreator this is a Scope project")

set(SCOPE_NAME ${CMAKE_PROJECT_NAME})
set(SCOPE_INSTALLDIR /${CMAKE_PROJECT_NAME})

#used generally, for example in manifest
set(PKG_PREFIX "com.canonical.scopes")
set(PKG ${SCOPE_NAME})
set(APP ${SCOPE_NAME})

set(SO_NAME ${PKG_PREFIX}.${PKG}_${APP})

set(DISPLAY_NAME "Day Info")
set(DISPLAY_DESC "Info about this day")
set(SEARCH_HINT "")
set(AUTHOR "mharrend@github")
set(CARD_ICON "images/calendar-app_day-17.png")
set(GETTEXT_DOMAIN ${SCOPE_NAME})
set(AUTHOR "mharrend@github")
set(MAINTAINER "mharrend@github")

# AUTOPILOT GENERIC TESTS CONTROL ===============================
# These settings control whether generic autopilot tests are run
# and with what values

set(IS_GO_SCOPE False)
set(TEST_RESULTS True)
set(TEST_DEPARTMENTS False)
set(TEST_ALL_DEPARTMENTS_HAVE_RESULTS False)
set(TEST_LOCATIONDATANEEDED True)
set(TEST_SEARCHHINT False)
set(TEST_PAGEHEADER_LOGO False)
set(TEST_PAGEHEADER_BACKGROUND False)
set(TEST_PAGEHEADER_FOREGROUNDCOLOR False)
set(TEST_PAGEHEADER_DIVIDERCOLOR False)
set(TEST_PREVIEWBUTTONCOLOR False)
set(TEST_SEARCH False) #NOTE, if True, also set SEARCH_STRING
set(SEARCH_STRING "") #New Year's Eve at least
# Scopes are tested running with SearchMetadata.location set to this lat & lang
set(LAT "51.506") # London. DO NOT LEAVE BLANK
set(LONG "-0.099") # same
set(COUNTRY "es") # Germany. Needed in this case to query timeanddate.com api
# get target languages from po files
# NOTE: each po file must have DisplayName, Description, and SearchHint(if used and set above)-
# translated, or else set TEST_INI_TRANSLATIONS to False (only during development!)
file(GLOB POFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} po/*.po)
set(TEST_MO_FILES True)
set(TEST_INI_TRANSLATIONS True) # Only set to false during development
set(TEST_TRANSLATED_DISPLAYNAME True)
# END autopilot tests ===========================================


include(GNUInstallDirs)

include(FindPkgConfig)
pkg_check_modules(UNITY_SCOPES libunity-scopes>=0.6.0 REQUIRED)

include_directories(
  "${CMAKE_SOURCE_DIR}/include"
)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/manifest.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
)
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/security.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PKG_PREFIX}.${PKG}_${APP}.security.json"
)

add_subdirectory(tests)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifest.json DESTINATION "/")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PKG_PREFIX}.${PKG}_${APP}.security.json" DESTINATION "/")
