set(BINARY ${CMAKE_PROJECT_NAME}_test)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cc)

set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME}lib gtest spdlog::spdlog liquid volk srsran_phy zmq)

add_custom_command(
  TARGET ${BINARY}
  POST_BUILD 
  COMMAND "${CMAKE_BINARY_DIR}/test/${BINARY}" 
  COMMENT "Running unit tests"
)
