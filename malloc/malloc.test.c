#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "testlib.h"
#include "malloc.h"
#include <assert.h>
#include <stdbool.h>

#define TEST_STRING "malloc works correctly"


static void
successful_malloc_returns_non_null_pointer(void)
{
	char *var = malloc(100);

	ASSERT_TRUE("1- successful malloc returns non null pointer", var != NULL);

	free(var);
}

static void
correct_copied_value(void)
{
	char *var = malloc(100);

	strcpy(var, TEST_STRING);

	ASSERT_TRUE("2- allocated memory should contain the copied value",
	            strcmp(var, TEST_STRING) == 0);

	free(var);
}

static void
test_maximum_size_is_exceeded(void)
{
	struct malloc_stats stats;

	char *var = malloc(40000000);

	ASSERT_TRUE("3- test maximum size is exceeded error",
	            var == NULL && errno == ENOMEM);

	get_stats(&stats);
	ASSERT_TRUE("4- amount of mallocs should be zero", stats.mallocs == 0);
}

static void
test_malloc_size_is_zero(void)
{
	struct malloc_stats stats;
	char *var = malloc(0);
	ASSERT_TRUE("5- test malloc size is zero, then memory is allocated",
	            var != NULL);

	free(var);

	get_stats(&stats);
	ASSERT_TRUE("6-amount of mallocs should be one", stats.mallocs == 1);
	ASSERT_TRUE("7-amount of frees should be one", stats.frees == 1);
}

static void
test_malloc_size_is_less_than_minimum_size(void)
{
	struct malloc_stats stats;
	char *var = malloc(15);

	ASSERT_TRUE("8-test malloc size is less than minimum, then memory is "
	            "allocated",
	            var != NULL);

	free(var);

	get_stats(&stats);
	ASSERT_TRUE("9-amount of mallocs should be one", stats.mallocs == 1);
	ASSERT_TRUE("10-amount of frees should be one", stats.frees == 1);
}

static void
test_double_free_with_no_block_free(void)
{
	struct malloc_stats stats;
	char *var = malloc(1000);
	char *var_2 = malloc(1000);

	free(var);
	free(var);

	get_stats(&stats);
	ASSERT_TRUE("11- amount of frees should be one", stats.frees == 1);

	free(var_2);
}

static void
test_double_free_with_block_free(void)
{
	struct malloc_stats stats;
	char *var = malloc(2000);
	free(var);
	free(var);

	get_stats(&stats);
	ASSERT_TRUE("12 -amount of frees should be one", stats.frees == 1);
}

static void
test_invalid_free(void)
{
	struct malloc_stats stats;
	char *var = malloc(1000);

	strcpy(var, TEST_STRING);

	free(&(var[1]));

	get_stats(&stats);
	ASSERT_TRUE("13- amount of frees should be zero", stats.frees == 0);

	free(var);
}

static void
correct_amount_of_mallocs(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);

	get_stats(&stats);
	ASSERT_TRUE("14- amount of mallocs should be one", stats.mallocs == 1);
}

static void
correct_amount_of_frees(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);

	get_stats(&stats);

	ASSERT_TRUE("15- amount of frees should be one", stats.frees == 1);
}

static void
correct_amount_of_requested_memory(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);

	get_stats(&stats);
	ASSERT_TRUE("16- amount of requested memory should be 100",
	            stats.requested_memory == 100);
}

static void
correct_amount_of_requested_memory_with_2_mallocs(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);
	char *var_2 = malloc(500);
	free(var);
	free(var_2);

	get_stats(&stats);
	ASSERT_TRUE("17- amount of requested memory should be 600",
	            stats.requested_memory == 600);
}

static void
test_one_region_in_small_block(void)
{
	struct malloc_stats stats;
	char *var = malloc(500);

	strcpy(var, TEST_STRING);

	ASSERT_TRUE(
	        "18- allocated memory should contain the copied value for a "
	        "region in a small block",
	        strcmp(var, TEST_STRING) == 0);
	free(var);

	get_stats(&stats);
	ASSERT_TRUE("19- amount of small blocks should be 1",
	            stats.amount_small_blocks == 1);
}

static void
test_multiple_regions_in_small_block(void)
{
	struct malloc_stats stats;
	char *var = malloc(5000);
	char *var2 = malloc(700);

	strcpy(var, TEST_STRING);
	strcpy(var2, TEST_STRING);


	ASSERT_TRUE(
	        "20- allocated memory should contain the copied value of var "
	        "for region in a small block",
	        strcmp(var, TEST_STRING) == 0);

	ASSERT_TRUE(
	        "21- allocated memory should contain the copied value of var2 "
	        "for region in a small block",
	        strcmp(var2, TEST_STRING) == 0);

	free(var);
	free(var2);
	get_stats(&stats);
	ASSERT_TRUE("22- amount of small blocks should be 1",
	            stats.amount_small_blocks == 1);
}

static void
test_multiple_regions_in_multiple_small_blocks(void)
{
	struct malloc_stats stats;
	char *var = malloc(8000);
	char *var2 = malloc(1000);

	// deberia crear un nuevo bloque
	char *var3 = malloc(15000);
	char *var4 = malloc(500);
	char *var5 = malloc(12000);

	strcpy(var, TEST_STRING);
	strcpy(var2, TEST_STRING);
	strcpy(var3, TEST_STRING);
	strcpy(var4, TEST_STRING);
	strcpy(var5, TEST_STRING);

	ASSERT_TRUE(
	        "23- allocated memory should contain the copied value of var "
	        "for region in small block",
	        strcmp(var, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "24- allocated memory should contain the copied value of var2 "
	        "for region in small block",
	        strcmp(var2, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "25- allocated memory should contain the copied value of var3 "
	        "for region in small block",
	        strcmp(var3, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "26- allocated memory should contain the copied value of var4 "
	        "for region in small block",
	        strcmp(var4, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "27- allocated memory should contain the copied value of var5 "
	        "for region in small block",
	        strcmp(var5, TEST_STRING) == 0);

	free(var);
	free(var2);
	free(var3);
	free(var4);
	free(var5);
	get_stats(&stats);
	ASSERT_TRUE("28- amount of small blocks should be 3",
	            stats.amount_small_blocks == 3);
}

static void
test_one_region_in_medium_block(void)
{
	struct malloc_stats stats;
	char *var = malloc(20000);

	strcpy(var, TEST_STRING);

	ASSERT_TRUE(
	        "29- allocated memory should contain the copied value for a "
	        "region in a medium block",
	        strcmp(var, TEST_STRING) == 0);
	free(var);
	get_stats(&stats);
	ASSERT_TRUE("30- amount of medium blocks should be 1",
	            stats.amount_medium_blocks == 1);
}

static void
test_multiple_regions_in_medium_block(void)
{
	struct malloc_stats stats;
	char *var = malloc(20000);
	char *var2 = malloc(25000);

	strcpy(var, TEST_STRING);
	strcpy(var2, TEST_STRING);

	ASSERT_TRUE(
	        "31- allocated memory should contain the copied value of var "
	        "for region in a medium block",
	        strcmp(var, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "32- allocated memory should contain the copied value of var2 "
	        "for region in a medium block",
	        strcmp(var2, TEST_STRING) == 0);

	free(var);
	free(var2);
	get_stats(&stats);
	ASSERT_TRUE("33- amount of medium blocks should be 1",
	            stats.amount_medium_blocks == 1);
}

static void
test_multiple_regions_in_multiple_medium_blocks(void)
{
	struct malloc_stats stats;
	char *var = malloc(900000);
	char *var2 = malloc(400000);
	char *var3 = malloc(500000);
	char *var4 = malloc(200000);

	strcpy(var, TEST_STRING);
	strcpy(var2, TEST_STRING);
	strcpy(var3, TEST_STRING);
	strcpy(var4, TEST_STRING);

	ASSERT_TRUE(
	        "34- allocated memory should contain the copied value of var "
	        "for region in medium block",
	        strcmp(var, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "35- allocated memory should contain the copied value of var2 "
	        "for region in medium block",
	        strcmp(var2, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "36- allocated memory should contain the copied value of var3 "
	        "for region in medium block",
	        strcmp(var3, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "37- allocated memory should contain the copied value of var4 "
	        "for region in medium block",
	        strcmp(var4, TEST_STRING) == 0);

	free(var);
	free(var2);
	free(var3);
	free(var4);
	get_stats(&stats);
	ASSERT_TRUE("38- amount of medium blocks should be 3",
	            stats.amount_medium_blocks == 3);
}

static void
test_one_region_in_large_block(void)
{
	struct malloc_stats stats;
	char *var = malloc(3000000);

	strcpy(var, TEST_STRING);

	ASSERT_TRUE(
	        "39- allocated memory should contain the copied value for a "
	        "region in a large block",
	        strcmp(var, TEST_STRING) == 0);
	free(var);
	get_stats(&stats);
	ASSERT_TRUE("40- amount of large blocks should be 1",
	            stats.amount_large_blocks == 1);
}

static void
test_multiple_regions_in_large_block(void)
{
	struct malloc_stats stats;
	char *var = malloc(1200000);
	char *var2 = malloc(1200000);

	strcpy(var, TEST_STRING);
	strcpy(var2, TEST_STRING);

	ASSERT_TRUE(
	        "41- allocated memory should contain the copied value of var "
	        "for region in a large block",
	        strcmp(var, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "42- allocated memory should contain the copied value of var2 "
	        "for region in a large block",
	        strcmp(var2, TEST_STRING) == 0);

	free(var);
	free(var2);
	get_stats(&stats);
	ASSERT_TRUE("43- amount of large blocks should be 1",
	            stats.amount_large_blocks == 1);
}

static void
test_multiple_regions_in_multiple_large_blocks(void)
{
	struct malloc_stats stats;
	char *var = malloc(3200000);
	char *var2 = malloc(30000000);
	char *var3 = malloc(32000000);
	char *var4 = malloc(8000000);

	strcpy(var, TEST_STRING);
	strcpy(var2, TEST_STRING);
	strcpy(var3, TEST_STRING);
	strcpy(var4, TEST_STRING);

	ASSERT_TRUE(
	        "44- allocated memory should contain the copied value of var "
	        "for region in large block",
	        strcmp(var, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "45- allocated memory should contain the copied value of var2 "
	        "for region in large block",
	        strcmp(var2, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "46- allocated memory should contain the copied value of var3 "
	        "for region in large block",
	        strcmp(var3, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "47- allocated memory should contain the copied value of var4 "
	        "for region in large block",
	        strcmp(var4, TEST_STRING) == 0);

	free(var);
	free(var2);
	free(var3);
	free(var4);
	get_stats(&stats);
	ASSERT_TRUE("48- amount of large blocks should be 3",
	            stats.amount_large_blocks == 3);
}

static void
test_multiple_regions_in_multiple_block_sizes(void)
{
	struct malloc_stats stats;
	char *var = malloc(500000);
	char *var2 = malloc(400);
	char *var3 = malloc(3000000);

	strcpy(var, TEST_STRING);
	strcpy(var2, TEST_STRING);
	strcpy(var3, TEST_STRING);

	ASSERT_TRUE(
	        "49- allocated memory should contain the copied value of var "
	        "for region in medium block with multiple blocks",
	        strcmp(var, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "50- allocated memory should contain the copied value of var2 "
	        "for region in small block with multiple blocks",
	        strcmp(var2, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "51- allocated memory should contain the copied value of var3 "
	        "for region in large block with multiple blocks",
	        strcmp(var3, TEST_STRING) == 0);

	free(var);
	free(var2);
	free(var3);
	get_stats(&stats);
	ASSERT_TRUE("52- amount of small blocks should be 1",
	            stats.amount_small_blocks == 1);
	ASSERT_TRUE("53- amount of medium blocks should be 1",
	            stats.amount_medium_blocks == 1);
	ASSERT_TRUE("54- amount of large blocks should be 1",
	            stats.amount_large_blocks == 1);
}

static void
test_calloc_correct_copied_value(void)
{
	char *var = calloc(50, 10);
	strcpy(var, TEST_STRING);

	ASSERT_TRUE(
	        "55- allocated memory with calloc should contain the copied "
	        "value of var",
	        strcmp(var, TEST_STRING) == 0);
	free(var);
}

void
test_calloc_correct_initialization_of_zeros()
{
	int *ptr = (int *) calloc(10, sizeof(int));
	bool correct_initialization_of_zeros = true;

	for (int i = 0; i < 10; i++) {
		correct_initialization_of_zeros &= ptr[i] == 0;
	}

	ASSERT_TRUE("56 - correct initialization of zeros",
	            correct_initialization_of_zeros);

	free(ptr);
}

static void
test_realloc_to_smaller_size(void)
{
	char *var = malloc(500);

	strcpy(var, TEST_STRING);

	char *var2 = realloc(var, 300);

	ASSERT_TRUE("57 - reallocated memory should contain the copied value "
	            "of var2",
	            strcmp(var2, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "58 - reallocated memory does not change initial position when "
	        "reallocating to a smaller size",
	        var == var2);

	free(var);
}


static void
test_realloc_to_bigger_size(void)
{
	char *var = malloc(500);

	strcpy(var, TEST_STRING);

	char *var2 = realloc(var, 1000);

	ASSERT_TRUE("59 - reallocated memory should contain the copied value "
	            "of var2",
	            strcmp(var2, TEST_STRING) == 0);
	ASSERT_TRUE(
	        "60-reallocated memory does not change initial position when "
	        "reallocating to a bigger size with enough space",
	        var == var2);

	free(var2);
}

static void
test_realloc_to_a_new_block_size(void)
{
	char *var = malloc(500);

	strcpy(var, TEST_STRING);

	char *var2 = realloc(var, 25000);

	ASSERT_TRUE("61- reallocated memory should contain the copied value of "
	            "var2 when reallocating to a new block size",
	            strcmp(var2, TEST_STRING) == 0);
	ASSERT_TRUE("62 - reallocated memory changes initial position when "
	            "reallocating to a new block size",
	            var != var2);

	free(var);
	free(var2);
}

static void
test_realloc_ptr_is_null_behaviour_is_as_malloc(void)
{
	struct malloc_stats stats;

	char *var = realloc(NULL, 25000);

	strcpy(var, TEST_STRING);

	ASSERT_TRUE(
	        "63 -reallocated memory should contain the copied value of var",
	        strcmp(var, TEST_STRING) == 0);

	free(var);

	get_stats(&stats);

	ASSERT_TRUE(
	        "64 -amount of mallocs should be one when reallocating with "
	        "null pointer",
	        stats.mallocs == 1);
}

static void
test_realloc_to_size_zero_behaviour_is_as_free(void)
{
	struct malloc_stats stats;

	char *var = malloc(500);
	var = realloc(var, 0);

	get_stats(&stats);

	ASSERT_TRUE("65 -amount of frees should be one", stats.frees == 1);
}

static void
test_realloc_to_invalid_position(void)
{
	char *var = malloc(500);

	strcpy(var, TEST_STRING);

	char *var2 = realloc(&(var[1]), 12000);

	ASSERT_TRUE("66 -test realloc to invalid position",
	            var2 == NULL && errno == ENOMEM);
	free(var);
}

static void
test_maximum_size_is_exceeded_in_realloc(void)
{
	char *var = malloc(3000000);

	char *var2 = realloc(var, 5000000000000);

	ASSERT_TRUE("67-test maximum size while reallocating is exceeded error",
	            var2 == NULL && errno == ENOMEM);

	free(var);
}


int
main(void)
{
	printfmt("\n######### PRUEBAS MALLOC Y FREE #########\n");

	printfmt("\n######### PRUEBAS CATEDRA #########\n");
	run_test(successful_malloc_returns_non_null_pointer);
	run_test(correct_copied_value);
	printfmt("\n######### PRUEBAS MANEJO DE ERRORES #########\n");
	run_test(test_maximum_size_is_exceeded);
	run_test(test_malloc_size_is_zero);
	run_test(test_malloc_size_is_less_than_minimum_size);
	run_test(test_double_free_with_block_free);
	run_test(test_double_free_with_no_block_free);
	run_test(test_invalid_free);
	printfmt("\n######### PRUEBAS CANTIDAD #########\n");
	run_test(correct_amount_of_mallocs);
	run_test(correct_amount_of_frees);
	run_test(correct_amount_of_requested_memory);
	run_test(correct_amount_of_requested_memory_with_2_mallocs);
	printfmt("\n######### PRUEBAS BLOQUE DE TAMAÑO PEQUEÑO #########\n");
	run_test(test_one_region_in_small_block);
	run_test(test_multiple_regions_in_small_block);
	run_test(test_multiple_regions_in_multiple_small_blocks);
	printfmt("\n######### PRUEBAS BLOQUE DE TAMAÑO MEDIO #########\n");
	run_test(test_one_region_in_medium_block);
	run_test(test_multiple_regions_in_medium_block);
	run_test(test_multiple_regions_in_multiple_medium_blocks);
	printfmt("\n######### PRUEBAS BLOQUE DE TAMAÑO GRANDE #########\n");
	run_test(test_one_region_in_large_block);
	run_test(test_multiple_regions_in_large_block);
	run_test(test_multiple_regions_in_multiple_large_blocks);
	printfmt("\n######### PRUEBAS BLOQUES DE VARIOS TAMAÑOS #########\n");
	run_test(test_multiple_regions_in_multiple_block_sizes);

	printfmt("\n######### PRUEBAS CALLOC Y FREE #########\n");
	run_test(test_calloc_correct_copied_value);
	run_test(test_calloc_correct_initialization_of_zeros);

	printfmt("\n######### PRUEBAS REALLOC Y FREE #########\n");
	run_test(test_realloc_to_smaller_size);
	run_test(test_realloc_to_bigger_size);
	run_test(test_realloc_to_a_new_block_size);
	run_test(test_realloc_ptr_is_null_behaviour_is_as_malloc);
	run_test(test_realloc_to_size_zero_behaviour_is_as_free);
	run_test(test_realloc_to_invalid_position);
	run_test(test_maximum_size_is_exceeded_in_realloc);


	return 0;
}
