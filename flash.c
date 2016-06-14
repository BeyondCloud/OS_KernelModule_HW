#include <linux/module.h>
#include <linux/configfs.h>
#include <linux/init.h>
#include <linux/tty.h> /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h> /* For KDSETLED */
#include <linux/vt.h>
#include <linux/console_struct.h> /* For vc_cons */
#include <linux/vt_kern.h>
MODULE_DESCRIPTION("This module will blink three Keyboard LEDs periodically");
MODULE_LICENSE("GPL");
struct timer_list my_timer;
struct tty_driver *my_driver;
char led_status = 0;
#define BLINK_PERIOD HZ/5
#define ALL_LEDS_ON 0x07
#define RESTORE_LEDS 0xFF
static void timer_func(unsigned long ptr)
{
	int *led_stat = (int *)ptr;  
	if (*led_stat == ALL_LEDS_ON)
		*led_stat = RESTORE_LEDS;
	else
		*led_stat = ALL_LEDS_ON;
	(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,*led_stat);
	my_timer.expires = jiffies + BLINK_PERIOD;
	add_timer(&my_timer);
}
static int __init flash_driver_init(void)
{
	int i;
	printk(KERN_INFO "flash_driver: loading\n");
	printk(KERN_INFO "flash_driver: fgconsole is %x\n", fg_console);
	for (i = 0; i < MAX_NR_CONSOLES; i++) {
		if (!vc_cons[i].d)
		break;
		printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n", i
		,MAX_NR_CONSOLES, vc_cons[i].d->vc_num
		,(unsigned long)vc_cons[i].d->port.tty);
	}
	printk(KERN_INFO "flash_driver: finished scanning consoles\n");
	my_driver = vc_cons[fg_console].d->port.tty->driver;
	printk(KERN_INFO "flash_driver: tty driver magic %x\n", my_driver->magic);
	/*
	* Set up the LED blink timer the first time
	*/
	init_timer(&my_timer);
	my_timer.function = timer_func;
	my_timer.data = (unsigned long)&led_status;
	my_timer.expires = jiffies + BLINK_PERIOD;
	add_timer(&my_timer);
	return 0;
}
static void __exit flash_driver_cleanup(void)
{
	printk(KERN_INFO "flash_driver: unloading...\n");
	del_timer(&my_timer);
	(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,RESTORE_LEDS);
}
module_init(flash_driver_init);
module_exit(flash_driver_cleanup);
