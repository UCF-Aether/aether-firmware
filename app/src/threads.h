/*************************** Thread Definitions *******************************/
#define BME_STACK_SIZE		1024
#define ZMOD_STACK_SIZE		1024
#define PM_STACK_SIZE		1024
#define LORA_STACK_SIZE		1024
#define USB_STACK_SIZE		1024

#define NORMAL_PRIORITY		5
#define HIGH_PRIORITY		3

K_THREAD_STACK_DEFINE(bme_stack_area, BME_STACK_SIZE);
K_THREAD_STACK_DEFINE(zmod_stack_area, ZMOD_STACK_SIZE);
K_THREAD_STACK_DEFINE(pm_stack_area, PM_STACK_SIZE);
K_THREAD_STACK_DEFINE(lora_stack_area, LORA_STACK_SIZE);
K_THREAD_STACK_DEFINE(usb_stack_area, USB_STACK_SIZE);

extern void bme_entry_point(void *, void *, void *);
extern void zmod_entry_point(void *, void *, void *);
extern void pm_entry_point(void *, void *, void *);
extern void lora_entry_point(void *, void *, void *);
extern void usb_entry_point(void *, void *, void *);

struct k_thread bme_thread_data;
struct k_thread zmod_thread_data;
struct k_thread pm_thread_data;
struct k_thread lora_thread_data;
struct k_thread usb_thread_data;
