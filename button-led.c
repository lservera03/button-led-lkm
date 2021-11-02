#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code

MODULE_LICENSE("");
MODULE_AUTHOR("Luis Servera");
MODULE_DESCRIPTION("A Button/LED test driver for the BBB");
MODULE_VERSION("0.1");

static unsigned int gpioLED1 = 38;///< hard coding the LED gpio for this example to P9_23 (GPIO49)
static unsigned int gpioLED2 = 36;
static unsigned int gpioButton1 = 37;   ///< hard coding the button gpio for this example to P9_27 (GPIO115)
static unsigned int gpioButton2 = 35;
static unsigned int gpioButton3 = 33;
static unsigned int gpioButton4 = 40;
static unsigned int irqNumberButton1;          ///< Used to share the IRQ number within this file
static unsigned int irqNumberButton2;
static unsigned int irqNumberButton3;
static unsigned int irqNumberButton4;
static unsigned int button1Presses = 0;  ///< For information, store the number of button presses
static unsigned int button2Presses = 0;
static unsigned int button3Presses = 0;
static unsigned int button4Presses = 0;
static bool	    led1On = 0;          ///< Is the LED on or off? Used to invert its state (off by default)
static bool 	    led2On = 0;

/// Function prototype for the custom IRQ handler function -- see below for the implementation
static irq_handler_t  button1_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

static irq_handler_t  button2_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

static irq_handler_t  button3_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

static irq_handler_t  button4_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);


/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point. In this example this
 *  function sets up the GPIOs and the IRQ
 *  @return returns 0 if successful
 */
static int __init ebbgpio_init(void){
   int result = 0;
   printk(KERN_INFO "GPIO_TEST: Initializing the GPIO_TEST LKM\n");
   // Is the GPIO a valid GPIO number (e.g., the BBB has 4x32 but not all available)
   if (!gpio_is_valid(gpioLED1)){
      printk(KERN_INFO "GPIO_TEST: invalid LED 1 GPIO\n");
      return -ENODEV;
   }
   if (!gpio_is_valid(gpioLED2)){
      printk(KERN_INFO "GPIO_TEST: invalid LED 2 GPIO\n");
      return -ENODEV;
   }
   // Going to set up the LED. It is a GPIO in output mode and will be on by default
   led1On = true;
   led2On = true;
   
   gpio_request(gpioLED1, "sysfs");          // gpioLED is hardcoded to 49, request it
   gpio_direction_output(gpioLED1, ledOn);   // Set the gpio to be in output mode and on
   gpio_export(gpioLED1, false);             // Causes gpio49 to appear in /sys/class/gpio
			                    // the bool argument prevents the direction from being changed
   gpio_request(gpioLED2, "sysfs");          // gpioLED is hardcoded to 49, request it
   gpio_direction_output(gpioLED2, ledOn);   // Set the gpio to be in output mode and on
   gpio_export(gpioLED2, false);             // Causes gpio49 to appear in /sys/class/gpio
   
   gpio_request(gpioButton1, "sysfs");       // Set up the gpioButton
   gpio_direction_input(gpioButton1);        // Set the button GPIO to be an input
   gpio_set_debounce(gpioButton1, 200);      // Debounce the button with a delay of 200ms
   gpio_export(gpioButton1, false);          // Causes gpio115 to appear in /sys/class/gpio
			                    // the bool argument prevents the direction from being changed
   gpio_request(gpioButton2, "sysfs");       // Set up the gpioButton
   gpio_direction_input(gpioButton2);        // Set the button GPIO to be an input
   gpio_set_debounce(gpioButton2, 200);      // Debounce the button with a delay of 200ms
   gpio_export(gpioButton2, false);          // Causes gpio115 to appear in /sys/class/gpio

   gpio_request(gpioButton3, "sysfs");       // Set up the gpioButton
   gpio_direction_input(gpioButton3);        // Set the button GPIO to be an input
   gpio_set_debounce(gpioButton3, 200);      // Debounce the button with a delay of 200ms
   gpio_export(gpioButton3, false);          // Causes gpio115 to appear in /sys/class/gpio

   gpio_request(gpioButton4, "sysfs");       // Set up the gpioButton
   gpio_direction_input(gpioButton4);        // Set the button GPIO to be an input
   gpio_set_debounce(gpioButton4, 200);      // Debounce the button with a delay of 200ms
   gpio_export(gpioButton4, false);          // Causes gpio115 to appear in /sys/class/gpio
   
   // Perform a quick test to see that the button is working as expected on LKM load
   printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", gpio_get_value(gpioButton1));

   // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
   irqNumberButton1 = gpio_to_irq(gpioButton1);
   printk(KERN_INFO "GPIO_TEST: The button 1 is mapped to IRQ: %d\n", irqNumberButton1);

   // This next call requests an interrupt line
   result = request_irq(irqNumberButton1,             // The interrupt number requested
                        (irq_handler_t) button1_irq_handler, // The pointer to the handler function below
                        IRQF_TRIGGER_RISING,   // Interrupt on rising edge (button press, not release)
                        "ebb_gpio_handler",    // Used in /proc/interrupts to identify the owner
                        NULL);                 // The *dev_id for shared interrupt lines, NULL is okay


   // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
   irqNumberButton2 = gpio_to_irq(gpioButton2);
   printk(KERN_INFO "GPIO_TEST: The button 2 is mapped to IRQ: %d\n", irqNumberButton2);

   // This next call requests an interrupt line
   result = request_irq(irqNumberButton2,             // The interrupt number requested
                        (irq_handler_t) button2_irq_handler, // The pointer to the handler function below
                        IRQF_TRIGGER_RISING,   // Interrupt on rising edge (button press, not release)
                        "ebb_gpio_handler",    // Used in /proc/interrupts to identify the owner
                        NULL);                 // The *dev_id for shared interrupt lines, NULL is okay


   // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
   irqNumberButton3 = gpio_to_irq(gpioButton3);
   printk(KERN_INFO "GPIO_TEST: The button 3 is mapped to IRQ: %d\n", irqNumberButton3);

   // This next call requests an interrupt line
   result = request_irq(irqNumberButton3,             // The interrupt number requested
                        (irq_handler_t) button3_irq_handler, // The pointer to the handler function below
                        IRQF_TRIGGER_RISING,   // Interrupt on rising edge (button press, not release)
                        "ebb_gpio_handler",    // Used in /proc/interrupts to identify the owner
                        NULL);                 // The *dev_id for shared interrupt lines, NULL is okay

   // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
   irqNumberButton4 = gpio_to_irq(gpioButton4);
   printk(KERN_INFO "GPIO_TEST: The button 4 is mapped to IRQ: %d\n", irqNumberButton4);

   // This next call requests an interrupt line
   result = request_irq(irqNumberButton4,             // The interrupt number requested
                        (irq_handler_t) button4_irq_handler, // The pointer to the handler function below
                        IRQF_TRIGGER_RISING,   // Interrupt on rising edge (button press, not release)
                        "ebb_gpio_handler",    // Used in /proc/interrupts to identify the owner
                        NULL);                 // The *dev_id for shared interrupt lines, NULL is okay
   
   printk(KERN_INFO "GPIO_TEST: The interrupt request result is: %d\n", result);
   return result;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required. Used to release the
 *  GPIOs and display cleanup messages.
 */
static void __exit ebbgpio_exit(void){
   printk(KERN_INFO "GPIO_TEST: The button 1 state is currently: %d\n", gpio_get_value(gpioButton));
   printk(KERN_INFO "GPIO_TEST: The button was pressed %d times\n", numberPresses);
   gpio_set_value(gpioLED, 0);              // Turn the LED off, makes it clear the device was unloaded
   gpio_unexport(gpioLED);                  // Unexport the LED GPIO
   free_irq(irqNumber, NULL);               // Free the IRQ number, no *dev_id required in this case
   gpio_unexport(gpioButton);               // Unexport the Button GPIO
   gpio_free(gpioLED);                      // Free the LED GPIO
   gpio_free(gpioButton);                   // Free the Button GPIO
   printk(KERN_INFO "GPIO_TEST: Goodbye from the LKM!\n");
}

/** @brief The GPIO IRQ Handler function
 *  This function is a custom interrupt handler that is attached to the GPIO above. The same interrupt
 *  handler cannot be invoked concurrently as the interrupt line is masked out until the function is complete.
 *  This function is static as it should not be invoked directly from outside of this file.
 *  @param irq    the IRQ number that is associated with the GPIO -- useful for logging.
 *  @param dev_id the *dev_id that is provided -- can be used to identify which device caused the interrupt
 *  Not used in this example as NULL is passed.
 *  @param regs   h/w specific register values -- only really ever used for debugging.
 *  return returns IRQ_HANDLED if successful -- should return IRQ_NONE otherwise.
 */
static irq_handler_t button1_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   led1On = !led1On;                          // Invert the LED state on each button press
   gpio_set_value(gpioLED1, led1On);          // Set the physical LED accordingly
   printk(KERN_INFO "GPIO_TEST: Interrupt! (button 1 state is %d)\n", gpio_get_value(gpioButton1));
   button1Presses++;                         // Global counter, will be outputted when the module is unloaded
   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}


static irq_handler_t button2_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   led1On = !led1On;                          // Invert the LED state on each button press
   gpio_set_value(gpioLED1, led1On);          // Set the physical LED accordingly
   printk(KERN_INFO "GPIO_TEST: Interrupt! (button 2 state is %d)\n", gpio_get_value(gpioButton2));
   button2Presses++;                         // Global counter, will be outputted when the module is unloaded
   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}


static irq_handler_t button3_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   led2On = !led2On;                          // Invert the LED state on each button press
   gpio_set_value(gpioLED2, led2On);          // Set the physical LED accordingly
   printk(KERN_INFO "GPIO_TEST: Interrupt! (button 3 state is %d)\n", gpio_get_value(gpioButton3));
   button3Presses++;                         // Global counter, will be outputted when the module is unloaded
   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}


static irq_handler_t button4_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   led2On = !led2On;                          // Invert the LED state on each button press
   gpio_set_value(gpioLED2, led2On);          // Set the physical LED accordingly
   printk(KERN_INFO "GPIO_TEST: Interrupt! (button 1 state is %d)\n", gpio_get_value(gpioButton4));
   button4Presses++;                         // Global counter, will be outputted when the module is unloaded
   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}

/// This next calls are  mandatory -- they identify the initialization function
/// and the cleanup function (as above).
module_init(ebbgpio_init);
module_exit(ebbgpio_exit);
