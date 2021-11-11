#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 
#include <linux/interrupt.h>             

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luis Servera");
MODULE_DESCRIPTION("A Button/LED module");
MODULE_VERSION("0.1");

//LEDs GPIOs
static unsigned int gpioLED1 = 21;
static unsigned int gpioLED2 = 20;
//Buttons GPIOs
static unsigned int gpioButton1 = 19;   
static unsigned int gpioButton2 = 12;
static unsigned int gpioButton3 = 13;
static unsigned int gpioButton4 = 16;
//Buttons interruptions request numbers
static unsigned int irqNumberButton1;          
static unsigned int irqNumberButton2;
static unsigned int irqNumberButton3;
static unsigned int irqNumberButton4;
//Counter of presses for every button
static unsigned int button1Presses = 0;  
static unsigned int button2Presses = 0;
static unsigned int button3Presses = 0;
static unsigned int button4Presses = 0;
//Booleans to control if the leds are on or not
static bool	    led1On = false;          
static bool 	    led2On = false;


// Functions of interruption handler for every button
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
   printk(KERN_INFO "BUTTON_LED: Initializing the BUTTON_LED LKM\n");
   
   // Check if all GPIOs are valid
   if (!gpio_is_valid(gpioLED1)){
      printk(KERN_INFO "BUTTON_LED: invalid LED 1 GPIO\n");
      return -ENODEV;
   }
   if (!gpio_is_valid(gpioLED2)){
      printk(KERN_INFO "BUTTON_LED: invalid LED 2 GPIO\n");
      return -ENODEV;
   }
   if (!gpio_is_valid(gpioButton1)){
      printk(KERN_INFO "BUTTON_LED: invalid Button1 GPIO\n");
      return -ENODEV;
   }
   if (!gpio_is_valid(gpioButton2)){
      printk(KERN_INFO "BUTTON_LED: invalid Button2 GPIO\n");
      return -ENODEV;
   }
   if (!gpio_is_valid(gpioButton3)){
      printk(KERN_INFO "BUTTON_LED: invalid Button3 GPIO\n");
      return -ENODEV;
   }
   if (!gpio_is_valid(gpioButton4)){
      printk(KERN_INFO "BUTTON_LED: invalid Button 4 GPIO\n");
      return -ENODEV;
   }
   
   // LED off by default
   led1On = false;
   led2On = false;
   
   // Request to the OS the GPIO of both LEDs
   gpio_request(gpioLED1, "sysfs");          
   gpio_direction_output(gpioLED1, led1On);   // Set the gpio to be in output mode 
   gpio_export(gpioLED1, false);             
			                    
   gpio_request(gpioLED2, "sysfs");          
   gpio_direction_output(gpioLED2, led2On);   
   gpio_export(gpioLED2, false);             
   
   // Request to the OS the GPIO of all the buttons
   gpio_request(gpioButton1, "sysfs");       // Set up the gpioButton
   gpio_direction_input(gpioButton1);        // Set the button GPIO to be an input
   gpio_set_debounce(gpioButton1, 300);      // Debounce the button with a delay of 300ms
   gpio_export(gpioButton1, false);          
			                    
   gpio_request(gpioButton2, "sysfs");       // Set up the gpioButton
   gpio_direction_input(gpioButton2);        // Set the button GPIO to be an input
   gpio_set_debounce(gpioButton2, 300);      // Debounce the button with a delay of 300ms
   gpio_export(gpioButton2, false);          

   gpio_request(gpioButton3, "sysfs");       // Set up the gpioButton
   gpio_direction_input(gpioButton3);        // Set the button GPIO to be an input
   gpio_set_debounce(gpioButton3, 300);      // Debounce the button with a delay of 300ms
   gpio_export(gpioButton3, false);          

   gpio_request(gpioButton4, "sysfs");       // Set up the gpioButton
   gpio_direction_input(gpioButton4);        // Set the button GPIO to be an input
   gpio_set_debounce(gpioButton4, 300);      // Debounce the button with a delay of 300ms
   gpio_export(gpioButton4, false);         
   
   // Perform a quick test to see that the button is working as expected on LKM load
   printk(KERN_INFO "BUTTON_LED: The button 1 state is currently: %d\n", gpio_get_value(gpioButton1));

   // Map GPIO number to IRQ
   irqNumberButton1 = gpio_to_irq(gpioButton1);
   printk(KERN_INFO "BUTTON_LED: The button 1 is mapped to IRQ: %d\n", irqNumberButton1);

   // Request an interrupt line
   result = request_irq(irqNumberButton1,             // The interrupt number requested
                        (irq_handler_t) button1_irq_handler, // The pointer to the handler function below
                        IRQF_TRIGGER_RISING,   // Interrupt on rising edge (button press, not release)
                        "ebb_gpio_handler",    // Used in /proc/interrupts to identify the owner
                        NULL);                 // The *dev_id for shared interrupt lines, NULL is okay


  
   irqNumberButton2 = gpio_to_irq(gpioButton2);
   printk(KERN_INFO "GPIO_TEST: The button 2 is mapped to IRQ: %d\n", irqNumberButton2);

   result = request_irq(irqNumberButton2,             
                        (irq_handler_t) button2_irq_handler, 
                        IRQF_TRIGGER_RISING,   
                        "ebb_gpio_handler",    
                        NULL);                 


   irqNumberButton3 = gpio_to_irq(gpioButton3);
   printk(KERN_INFO "GPIO_TEST: The button 3 is mapped to IRQ: %d\n", irqNumberButton3);

   result = request_irq(irqNumberButton3,             
                        (irq_handler_t) button3_irq_handler, 
                        IRQF_TRIGGER_RISING,   
                        "ebb_gpio_handler",    
                        NULL);                 

   irqNumberButton4 = gpio_to_irq(gpioButton4);
   printk(KERN_INFO "GPIO_TEST: The button 4 is mapped to IRQ: %d\n", irqNumberButton4);

   result = request_irq(irqNumberButton4,             
                        (irq_handler_t) button4_irq_handler, 
                        IRQF_TRIGGER_RISING,   
                        "ebb_gpio_handler",    
                        NULL);                 
   
   printk(KERN_INFO "BUTTON_LED: The interrupt request result is: %d\n", result);
   return result;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required. Used to release the
 *  GPIOs and display cleanup messages.
 */
static void __exit ebbgpio_exit(void){
   printk(KERN_INFO "BUTTON_LED: The button 1 state is currently: %d\n", gpio_get_value(gpioButton1));
   printk(KERN_INFO "BUTTON_LED: The button 1 was pressed %d times\n", button1Presses);

   printk(KERN_INFO "BUTTON_LED: The button 2 was pressed %d times\n", button2Presses);
   printk(KERN_INFO "BUTTON_LED: The button 3 was pressed %d times\n", button3Presses);
   printk(KERN_INFO "BUTTON_LED: The button 4 was pressed %d times\n", button4Presses);
   
   gpio_set_value(gpioLED1, 0);              // Turn the LED off
   gpio_unexport(gpioLED1);   // Unexport the LED GPIO
   gpio_free(gpioLED1);  //Free GPIO

   gpio_set_value(gpioLED2, 0);
   gpio_unexport(gpioLED2);
   gpio_free(gpioLED2);

   free_irq(irqNumberButton1, NULL);               // Free the IRQ number, no *dev_id required in this case
   gpio_unexport(gpioButton1);               // Unexport the Button GPIO
   gpio_free(gpioButton1);                   // Free the Button GPIO

   free_irq(irqNumberButton2, NULL);              
   gpio_unexport(gpioButton2);               
   gpio_free(gpioButton2);                   


   free_irq(irqNumberButton3, NULL);              
   gpio_unexport(gpioButton3);               
   gpio_free(gpioButton3);                   

   free_irq(irqNumberButton4, NULL);               
   gpio_unexport(gpioButton4);               
   gpio_free(gpioButton4);                   
   
   printk(KERN_INFO "BUTTON_LED: Goodbye from the LKM!\n");
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
   /*
   char *argv[] = {"/home/pi/button1.sh", NULL};

   char *envp[] = {"HOME=/", NULL};
   */
   led1On = true;   // LED state to on
   gpio_set_value(gpioLED1, led1On);   // Set the physical LED accordingly
   printk(KERN_INFO "BUTTON_LED: Interrupt! (button 1 state is %d)\n", gpio_get_value(gpioButton1));
   button1Presses++;                         // Global counter, will be outputted when the module is unloaded

   //call_usermodehelper(argv[0], argv, envp, UMH_NO_WAIT);

   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}


static irq_handler_t button2_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   led1On = false;                          
   gpio_set_value(gpioLED1, led1On);          
   printk(KERN_INFO "BUTTON_LED: Interrupt! (button 2 state is %d)\n", gpio_get_value(gpioButton2));
   button2Presses++;                         
   return (irq_handler_t) IRQ_HANDLED;      
}


static irq_handler_t button3_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   led2On = true;                          
   gpio_set_value(gpioLED2, led2On);          
   printk(KERN_INFO "GPIO_TEST: Interrupt! (button 3 state is %d)\n", gpio_get_value(gpioButton3));
   button3Presses++;                         
   return (irq_handler_t) IRQ_HANDLED;      
}


static irq_handler_t button4_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   led2On = false;                          
   gpio_set_value(gpioLED2, led2On);          
   printk(KERN_INFO "GPIO_TEST: Interrupt! (button 4 state is %d)\n", gpio_get_value(gpioButton4));
   button4Presses++;                         
   return (irq_handler_t) IRQ_HANDLED;      
}

/// This next calls are  mandatory -- they identify the initialization function
/// and the cleanup function (as above).
module_init(ebbgpio_init);
module_exit(ebbgpio_exit);
