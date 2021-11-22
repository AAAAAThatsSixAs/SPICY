/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */
uint8_t full_mask=0xFF; 
static uint8_t master_bound=8; //1 more than max for master
static uint8_t slave_bound=15; //max of slave

/* Initialize the 8259 PIC */

/* i8259_init
DESC:  Initializes the keyboard
INPUT: none
OUTPUT: none
SIDE EFFECTS:Sends IRQ to PIC.
*/
void
i8259_init(void)
{
    //clear interrupts
    cli();
    master_mask=full_mask;
    slave_mask=full_mask;
    outb(master_mask, MASTER_8259_PORT + 1);
    outb(slave_mask, SLAVE_8259_PORT + 1);
    

     // Initialize master
    outb(ICW1, MASTER_8259_PORT);  
    outb(ICW2_MASTER, MASTER_8259_PORT + 1);
    outb(ICW3_MASTER, MASTER_8259_PORT + 1);
    outb(ICW4, MASTER_8259_PORT + 1);

    // Initialize slave 
    outb(ICW1, SLAVE_8259_PORT); 
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
    outb(ICW4, SLAVE_8259_PORT + 1);
    outb(master_mask, MASTER_8259_PORT + 1);
    outb(slave_mask, SLAVE_8259_PORT + 1);

    /*set masks again in case they have changed*/
    master_mask=full_mask;
    slave_mask=full_mask;
    outb(master_mask, MASTER_8259_PORT + 1);
    outb(slave_mask, SLAVE_8259_PORT + 1);
    //restore interrupts
    sti();

}

/* Enable (unmask) the specified IRQ */

/* enable_irq
DESC:  opens up specific ports on the PIC
INPUT: irq_num specifying specific port
OUTPUT: none
SIDE EFFECTS:opens up ports on PIC
*/
void
enable_irq(uint32_t irq_num)
{
    //check irq number is less than master
    if(irq_num < master_bound) {
        //check master mask
        master_mask &= ~(1<< irq_num);
    } 
    else {
        //check mask slave
        slave_mask &= ~(1<< (irq_num-master_bound));
        master_mask &= ~(1<< irq_num);  //reinitializing the master irq2(where the slave is)
    }

    //send mask to ports
    outb(master_mask, MASTER_8259_PORT + 1);
    outb(slave_mask, SLAVE_8259_PORT + 1);
 

}

/* Disable (mask) the specified IRQ */

/* disable_irq
DESC:  closes ports on the PIC
INPUT: irq_num specifying speific ports
OUTPUT: none
SIDE EFFECTS:Sends IRQ to PIC.
*/
void
disable_irq(uint32_t irq_num)
{
    //check irq number is less than master
    if(irq_num < master_bound) {
        //set master mask
        master_mask |= (1<< irq_num);
    } 
    else {
        //set slave and master mask
        slave_mask |= (1<< (irq_num-master_bound));
        master_mask |= (1<< irq_num);
    }

    //output to PIC
    outb(master_mask, MASTER_8259_PORT + 1);
    outb(slave_mask, SLAVE_8259_PORT + 1);
    

}

/* Send end-of-interrupt signal for the specified IRQ */

/* send_eoi
DESC:  sends eoi signal to the PIC
INPUT: specific irq num
OUTPUT: none
SIDE EFFECTS:ends master or slave processes
*/
void
send_eoi(uint32_t irq_num)
{
    /*check if valid irq_num*/
    if(irq_num<0||irq_num>slave_bound)
        return;
    /*slave bounds*/
    if(irq_num > (master_bound-1))
    {
        outb(ICW3_SLAVE|EOI, MASTER_8259_PORT);
        outb(EOI | (irq_num-master_bound),SLAVE_8259_PORT);
        return;
    }
    
    /*master bounds*/
    else {
       outb(EOI | irq_num,MASTER_8259_PORT);
       return;
   }
    
}

