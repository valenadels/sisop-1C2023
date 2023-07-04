#ifndef TESTLIB_H
#define TESTLIB_H

#include "printfmt.h"

#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"
#define ASSERT_TRUE(test_title, predicate)                                     \
	do {                                                                   \
		if (predicate)                                                 \
			printfmt("%s: %sPASS%s\n",                             \
			         test_title,                                   \
			         COLOR_GREEN,                                  \
			         COLOR_RESET);                                 \
		else                                                           \
			printfmt("%s: %sFAIL%s\n",                             \
			         test_title,                                   \
			         COLOR_RED,                                    \
			         COLOR_RESET);                                 \
	} while (0)

typedef void (*test_case_t)(void);

void run_test(test_case_t test_case);

#endif  // TESTLIB_H
