PKG_CHECK_MODULES(SAMPLE_NEEDS REQUIRED glib-2.0 ${TIZEN_LOG_PKG})
INCLUDE_DIRECTORIES(${SAMPLE_NEEDS_INCLUDE_DIRS})
LINK_DIRECTORIES(${SAMPLE_NEEDS_LIBRARY_DIRS})

ADD_EXECUTABLE("aitt_tcp_test" tcp_test.cc)
TARGET_LINK_LIBRARIES("aitt_tcp_test" TCP_OBJ ${SAMPLE_NEEDS_LIBRARIES} ${AITT_TCP_NEEDS_LIBRARIES})
INSTALL(TARGETS "aitt_tcp_test" DESTINATION ${AITT_TEST_BINDIR})
