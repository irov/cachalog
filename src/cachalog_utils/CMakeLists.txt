PROJECT(cachalog_utils C)

IF(CACHALOG_TARGET_WIN32)
    SET(cachalog_time_platform cachalog_time_win32.c)
ELSEIF(CACHALOG_TARGET_UNIX)
    SET(cachalog_time_platform cachalog_time_unix.c)
ENDIF()

ADD_FILTER(
src
    cachalog_ring.h
    
    cachalog_getopt.h
    cachalog_getopt.c
    
    cachalog_date.h
    cachalog_date.c
    
    cachalog_clock.h
    cachalog_clock.c
    
    cachalog_file.h
    cachalog_file.c
    
    cachalog_time.h
    ${cachalog_time_platform}
    
    cachalog_rand.h
    cachalog_rand.c
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${CACHALOG_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} cachalog_memory)