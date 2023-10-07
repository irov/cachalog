PROJECT(cachalog_thread C)

IF(CACHALOG_TARGET_WIN32)
    SET(cachalog_thread_platform cachalog_thread_win32.c)
    set(cachalog_thread_platform_extra_lib)
ELSEIF(CACHALOG_TARGET_UNIX OR CACHALOG_TARGET_OSX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    
    SET(cachalog_thread_platform cachalog_thread_pthread.c)
    set(cachalog_thread_platform_extra_lib Threads::Threads)
ENDIF()

ADD_FILTER(
src
    cachalog_thread.h
    ${cachalog_thread_platform}
)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${CACHALOG_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} cachalog_memory cachalog_log ${cachalog_thread_platform_extra_lib})