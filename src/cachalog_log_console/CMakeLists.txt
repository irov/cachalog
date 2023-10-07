PROJECT(cachalog_log_console C)

ADD_FILTER(
src
    cachalog_log_console.h
    cachalog_log_console.c
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${CACHALOG_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} cachalog_config)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} cachalog_log)