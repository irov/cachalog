PROJECT(cachalog_log C)

ADD_FILTER(
src
	cachalog_log.h
	cachalog_log.c
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${CACHALOG_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} cachalog_mutex cachalog_config)