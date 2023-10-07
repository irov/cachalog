PROJECT(cachalog_memory C)

ADD_FILTER(
src
    cachalog_memory.h
    cachalog_memory.c
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${CACHALOG_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} cachalog_config)