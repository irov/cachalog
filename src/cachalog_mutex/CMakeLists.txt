PROJECT(cachalog_mutex C)

IF(CACHALOG_TARGET_WIN32)
    set(cachalog_mutex_platform cachalog_mutex_win32.c)
    set(cachalog_mutex_platform_extra_lib)
ELSEIF(CACHALOG_TARGET_UNIX OR CACHALOG_TARGET_OSX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    
    set(cachalog_mutex_platform_extra_lib Threads::Threads)
    
    set(cachalog_mutex_platform cachalog_mutex_pthread.c)
ENDIF()

ADD_FILTER(
src
    cachalog_mutex.h
    ${cachalog_mutex_platform}
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${CACHALOG_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} cachalog_memory cachalog_log ${cachalog_mutex_platform_extra_lib})