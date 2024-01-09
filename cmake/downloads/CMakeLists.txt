CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

OPTION(CACHALOT_DOWNLOADS_SILENT "download silent" OFF)

PROJECT(downloads)

INCLUDE(../base_template.cmake)
INCLUDE(../project_template.cmake)
include(../download_template.cmake)

CACHALOT_GIT_CLONE(libevent https://github.com/libevent/libevent.git "release-2.1.12-stable")
CACHALOT_GIT_CLONE(json https://github.com/irov/json.git)
CACHALOT_GIT_CLONE(openssl https://github.com/janbar/openssl-cmake.git "1.1.1t-20230216")
CACHALOT_GIT_CLONE(hummingbird https://github.com/irov/hummingbird)