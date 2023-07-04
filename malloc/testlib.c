#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "testlib.h"

void
run_test(test_case_t test_case)
{
	pid_t p;

	if ((p = fork()) == 0) {
		test_case();
		exit(EXIT_SUCCESS);
	}

	assert(wait(NULL) > 0);
}
