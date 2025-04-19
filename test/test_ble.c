#include "unity.h"
#include "utils/ble_format_utils.h"

void setUp(void) {}
void tearDown(void) {}

void test_format_notify_data(void) {
    int value = 520;   // 0x0208
    uint8_t out[4];
    ble_format_notify_data(value, out);
    TEST_ASSERT_EQUAL_UINT8(0x08, out[0]);
    TEST_ASSERT_EQUAL_UINT8(0x02, out[1]);
    TEST_ASSERT_EQUAL_UINT8(0x00, out[2]);
    TEST_ASSERT_EQUAL_UINT8(0x00, out[3]);
}
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_format_notify_data);
    return UNITY_END();
}
