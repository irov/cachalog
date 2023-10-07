CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

OPTION(CACHALOG_DOWNLOADS_SILENT "download silent" OFF)

PROJECT(downloads)

INCLUDE(../base_template.cmake)
INCLUDE(../project_template.cmake)
include(../download_template.cmake)

CACHALOG_GIT_CLONE(libevent https://github.com/libevent/libevent.git "release-2.1.12-stable")
CACHALOG_GIT_CLONE(yyjson https://github.com/ibireme/yyjson.git "0.8.0")
CACHALOG_GIT_CLONE(openssl https://github.com/janbar/openssl-cmake.git "1.1.1t-20230216")