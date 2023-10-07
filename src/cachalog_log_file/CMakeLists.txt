PROJECT(cachalog_log_file C)

ADD_FILTER(
src
    cachalog_log_file.h
    cachalog_log_file.c
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${CACHALOG_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} cachalog_config)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} cachalog_log)