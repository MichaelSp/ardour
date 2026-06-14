#pragma once

#include_next <glibmm/fileutils.h>

#ifndef FILE_TEST_EXISTS
#define FILE_TEST_EXISTS FileTest::EXISTS
#endif
#ifndef FILE_TEST_IS_REGULAR
#define FILE_TEST_IS_REGULAR FileTest::IS_REGULAR
#endif
#ifndef FILE_TEST_IS_DIR
#define FILE_TEST_IS_DIR FileTest::IS_DIR
#endif
#ifndef FILE_TEST_IS_SYMLINK
#define FILE_TEST_IS_SYMLINK FileTest::IS_SYMLINK
#endif
#ifndef FILE_TEST_IS_EXECUTABLE
#define FILE_TEST_IS_EXECUTABLE FileTest::IS_EXECUTABLE
#endif
