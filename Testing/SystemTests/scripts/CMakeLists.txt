# Write out a files containing - the data search directories - the default save directory These will be read by the
# MantidConfig class to configure the properties file

# Data search directories
set(DATA_DIRS "")
# Input data
set(SYSTEM_TEST_DATA_DIR "${ExternalData_BINARY_ROOT}/Testing/Data/SystemTest")
list(APPEND DATA_DIRS ${SYSTEM_TEST_DATA_DIR})
list(APPEND DATA_DIRS ${SYSTEM_TEST_DATA_DIR}/INTER)
list(APPEND DATA_DIRS ${SYSTEM_TEST_DATA_DIR}/ILL/D2B)
list(APPEND DATA_DIRS ${SYSTEM_TEST_DATA_DIR}/ILL/D20)
list(APPEND DATA_DIRS ${SYSTEM_TEST_DATA_DIR}/ILL/D11)
list(APPEND DATA_DIRS ${SYSTEM_TEST_DATA_DIR}/ILL/D11B)
list(APPEND DATA_DIRS ${SYSTEM_TEST_DATA_DIR}/ILL/D16)
list(APPEND DATA_DIRS ${SYSTEM_TEST_DATA_DIR}/ILL/D33)
list(APPEND DATA_DIRS ${SYSTEM_TEST_DATA_DIR}/ILL/D22)
# Reference results
list(APPEND DATA_DIRS ${ExternalData_BINARY_ROOT}/Testing/SystemTests/tests/framework/reference)
# Unit tests
list(APPEND DATA_DIRS ${ExternalData_BINARY_ROOT}/Testing/Data/UnitTest)
# Doc tests
list(APPEND DATA_DIRS ${ExternalData_BINARY_ROOT}/Testing/Data/DocTest)

# Output to file If this is changed, make sure to update the .gitignore in this directory
set(DATA_DIRS_FILE ${CMAKE_CURRENT_LIST_DIR}/datasearch-directories.txt)
# The quotes are important as CMake translates the list to a semicolon-separated string
file(WRITE ${DATA_DIRS_FILE} "${DATA_DIRS}")

# Default save directory
set(SAVE_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(SAVE_DIR_FILE ${CMAKE_CURRENT_LIST_DIR}/defaultsave-directory.txt)
file(WRITE ${SAVE_DIR_FILE} "${SAVE_DIR}")
