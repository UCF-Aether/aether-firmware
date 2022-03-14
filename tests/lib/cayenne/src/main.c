#include <kernel.h>
#include <cayenne.h>

static void test_cayenne_packetize() {

}

static void test_get_reading_size() {

}

void test_main() {
  ztest_test_suite(cayenne,
     ztest_unit_test(test_cayenne_packetize),
     ztest_unit_test(test_get_reading_size));
	ztest_run_test_suite(cayenne);
}
