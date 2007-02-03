/*
 * vport is an own build multi I/O converter 
 * rs232, i2c,p arallel I/O port
 *
 *  Copyright (C) 2006 Benedikt Sauter (sauter@ixbat.de)
 *  
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 *	WITHOUT ANY WARRANTY
 *
 * This driver is based on the 2.6.3 version of drivers/usb/usb-vport.c 
 * but has been rewritten to be easy to read and use, as no locks are now
 * needed anymore.
 * 
 * Also the module ark3116 from Stefan Schulz helped me much.
 * Thanks!!
 *
 * digital i/o port
 * http://www.oreilly.de/german/freebooks/linuxdrive2ger/harddigio.html
 *
 * port part is based of
 * http://www.freesoftwaremagazine.com/articles/drivers_linux?page=0%2C0
 *
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kref.h>
#include <asm/uaccess.h>
#include <linux/usb.h>
#include <linux/fs.h> 
#include <linux/types.h> /* size_t */
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/io.h> /* inb, outb */

#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/tty_driver.h>
#include <linux/usb/serial.h>
#include <linux/serial.h>

#include <linux/ioport.h>


static int debug;

/**************************************************************/

/* serial port */

static int vport_serial_attach(struct usb_serial *serial);
static void vport_serial_set_termios(struct usb_serial_port *port,struct termios * old);
static int vport_serial_ioctl(struct usb_serial_port *port, struct file * file, unsigned int cmd, unsigned long arg);
static int vport_serial_tiocmget(struct usb_serial_port *port, struct file * file);
static int vport_serial_write (struct usb_serial_port *port, const unsigned char *buf, int count);
static void vport_serial_read_bulk_callback (struct urb *urb, struct pt_regs *regs);
static int vport_serial_open(struct usb_serial_port *port,struct file *filp);
void vport_serial_exit(void);
int vport_serial_init(void);




/* digitial IO Ports */
int vport_port_open(struct inode *inode, struct file *filp); 
int vport_port_release(struct inode *inode, struct file *filp); 
ssize_t vport_port_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t vport_port_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);
void vport_port_exit(void);
int vport_port_init(void);

/* i2c */

//static int vport_i2c_attach();
//static int vport_i2c_open();
//static int vport_i2c_write();
//static int vport_i2c_read();




/**************************************************************/
/* usb serial adapter */

static struct usb_device_id id_table [] = {
    { USB_DEVICE(0x0400,0x0000) },
    { },
};

MODULE_DEVICE_TABLE(usb,id_table);

static struct usb_driver vport_serial_driver = {
    .name = "vport",
    .probe = usb_serial_probe,
    .disconnect = usb_serial_disconnect,
    .id_table = id_table,
};


static struct usb_serial_driver vport_serial_device = {
    .driver = {
      .owner = THIS_MODULE,
      .name = "vport",
    },
    .id_table = id_table,
    .num_interrupt_in = 0,
    .num_bulk_in = 1,
    .num_bulk_out = 1,
    .num_ports = 1,
    .attach = vport_serial_attach,
    .set_termios = vport_serial_set_termios,
    .ioctl = vport_serial_ioctl,
    .tiocmget = vport_serial_tiocmget,
    .open = vport_serial_open,
    .write = vport_serial_write,
    .read_bulk_callback = vport_serial_read_bulk_callback,
};

struct vport_serial_private {
    spinlock_t lock;
    u8 termios_initialized;
};




/**************************************************************/
/* digital io ports */
/* Structure that declares the common */
/* file access functions */
struct file_operations vport_port_fops = { 
	read: vport_port_read,
	write: vport_port_write,
	open: vport_port_open,
	release: vport_port_release
};

/* Driver global variables */
/* Major number */
int vport_port_major = 61;	//experimental local use

/* Control variable for memory */ 
/* reservation of the parallel port*/
int vport_port_port;


/**************************************************************/



static int vport_serial_attach(struct usb_serial *serial)
{
    struct vport_serial_private *priv;
    int i;

    dbg("vport Device found and attach");
    
    for(i=0; i<serial->num_ports; ++i) {
		priv = kmalloc(sizeof (struct vport_serial_private), GFP_KERNEL);
	
		if(!priv)
	    	goto cleanup;

		memset(priv,0x00,sizeof(struct vport_serial_private));
		spin_lock_init(&priv->lock);
	
		usb_set_serial_port_data(serial->port[i],priv);
    }

    return 0;

    cleanup:
	for(--i;i>=0;--i) 
	{
	    usb_set_serial_port_data(serial->port[i],NULL);
	}
    return -ENOMEM;
}

static int vport_serial_open(struct usb_serial_port *port,struct file *fp)
{
	//struct termios tmp_termios;
	struct usb_serial *serial = port->serial;
	struct vport_serial_private *priv = usb_get_serial_port_data(port);
	int result;
	unsigned long flags;
		
	dbg("%s - port %d", __FUNCTION__,port->number);

	//result = usb_serial_generic_open(port, fp);
	//if (result)
	//	return result;


	
#if 0
	/* ATTENTION THIS CAUSES A DEADLOCK *
	 * http://www.gatago.com/linux/kernel/15460975.html */
	// TODO workarround?
	if(port->tty)
		port->tty->low_latency = 1;
#endif

    /* Start reading from the device */
	usb_fill_bulk_urb (port->read_urb, serial->dev,
		usb_rcvbulkpipe (serial->dev,3),
		port->read_urb->transfer_buffer,
		port->read_urb->transfer_buffer_length,
		((serial->type->read_bulk_callback) ?
		 serial->type->read_bulk_callback : 
		 vport_serial_read_bulk_callback), port);

	result = usb_submit_urb(port->read_urb, GFP_KERNEL);
	if (result) {
		return -1;	
	}

	
	if(port->tty) {
		spin_lock_irqsave(&priv->lock,flags);
		if(!priv->termios_initialized) {
			*(port->tty->termios) = tty_std_termios;
			port->tty->termios->c_cflag = B75 | CS8 | CREAD | HUPCL | CLOCAL;
			//port->tty->termios->c_cflag = B75 | CS8 | CLOCAL;	
			// transmit received data from avr without CR to ttyUSB
			port->tty->termios->c_iflag = 0;
			port->tty->termios->c_lflag = 0;
			port->tty->termios->c_oflag = 0;
			priv->termios_initialized = 1;
		}
		spin_unlock_irqrestore(&priv->lock, flags);
	}

    return 0;
}

static void vport_serial_set_termios(struct usb_serial_port *port,struct termios * old)
{
    dbg("term");
}

static int vport_serial_ioctl(struct usb_serial_port *port, struct file * file, unsigned int cmd, unsigned long arg)
{
    dbg("ioctl");
    return 0;
}

static int vport_serial_tiocmget(struct usb_serial_port *port, struct file * file)
{
    return 0;
}

static int vport_serial_write (struct usb_serial_port *port, const unsigned char *buf, int count)
{
    //struct visor_private *priv = usb_get_serial_port_data(port);
	struct usb_serial *serial = port->serial;
	//struct urb *urb;
	unsigned char *msg;
	int actlen;
	int *actual_length = &actlen;
	int i;

	dbg("wert: %s count: %i",buf,count);

	if(count<=0) {
		dbg("length of data <=0");
		return -1;		
	}

	

	msg = kmalloc(sizeof (char)*(count+4), GFP_KERNEL);
	actlen = count+4;

	/* prepare message in vport format */
	
	msg[0] = 0x01;		//command 0x01 = uart/rs232
	msg[1] = actlen;	// len
	msg[2] = 1;     	//port
	msg[3] = 0x01;  	//sending command

	for(i=0;i<count;i++){
		msg[i+4] = buf[i];
	}
	
	usb_bulk_msg(serial->dev, usb_sndbulkpipe(serial->dev,2),
				    msg, actlen, actual_length,
					100);

	kfree(msg);
	return count;
}

static void vport_serial_read_bulk_callback (struct urb *urb, struct pt_regs *regs)
{
	struct usb_serial_port *port = (struct usb_serial_port *)urb->context;
	struct usb_serial *serial = port->serial;
	struct tty_struct *tty = port->tty;
	unsigned char *data = urb->transfer_buffer;
	int result;
	int actual_length;

	dbg("read bulk callback %c",data[0]);

	if(urb->status) {
		dbg("%s - nonzero read bulk status received: %d",__FUNCTION__,urb->status);
	}
	
	usb_serial_debug_data(debug,&port->dev,__FUNCTION__,urb->actual_length,data);

	actual_length = urb->actual_length;
	if(tty && actual_length) {
		tty_buffer_request_room(tty,actual_length);
		dbg("data %c",data[0]);
		//dbg("tty %s index %i magic %i",tty->name, tty->index,tty->magic);
		//dbg("tty major %i num %i ",tty->driver->major,tty->driver->num);
		//tty_insert_flip_string(tty,data,actual_length);
		tty_insert_flip_char(tty,data[0],0);
		tty_flip_buffer_push(tty);
	}

    /* Start reading from the device */
	usb_fill_bulk_urb (port->read_urb, serial->dev,
		usb_rcvbulkpipe (serial->dev,3),
		port->read_urb->transfer_buffer,
		port->read_urb->transfer_buffer_length,
		((serial->type->read_bulk_callback) ?
		 serial->type->read_bulk_callback : 
		 vport_serial_read_bulk_callback), port);

	result = usb_submit_urb(port->read_urb, GFP_ATOMIC);

}




int vport_serial_init(void)
{
    int retval;

	/* serial part */

	/**
	 * create /dev/ttyUSB0
     */
    retval = usb_serial_register(&vport_serial_device);
    if(retval)
      return retval;
    retval = usb_register(&vport_serial_driver);
	
    //if(retval)
      //usb_serial_deregister(&vport_serial_device);
    return retval;
}



void vport_serial_exit(void)
{
	usb_deregister(&vport_serial_driver);
  	usb_serial_deregister(&vport_serial_device);
}


/**************************************************************/

int vport_port_init(void) { 
  	int result;

  	/* Registering device */
  	result = register_chrdev(vport_port_major, "vport_port", 
      	&vport_port_fops);
  	if (result < 0) { 
    	dbg(
      		"<1>vport_port: cannot obtain major number %d\n",
      		vport_port_major); 
    	return result; 
  	} 
   
	/* Registering port */
	//vport_port_port = check_region(0x278, 1);
  	//if (vport_port_port) { 
	//	dbg("<1>vport_port: cannot reserve 0x278\n"); 
	//	result = vport_port_port; 
	//	goto fail;
	//} 
	//request_region(0x278, 1, "vport_port");


  	dbg("<1>Inserting vport_port module\n"); 
  	return 0;

  	fail: 
    	vport_port_exit(); 
    	return result;
}

void vport_port_exit(void) {

	/* Make major number free! */
  	unregister_chrdev(vport_port_major, "vport_port");

	/* Make port free! */ 
	if (!vport_port_port) { 
		release_region(0x278,1);
	}

  	dbg("<1>Removing vport_port module\n");
}

int vport_port_open(struct inode *inode, struct file *filp) 
{

    dbg("vport port open");
  /* Success */
  return 0;

}

int vport_port_release(struct inode *inode, struct file *filp) {

  /* Success */
  return 0; 
}

ssize_t vport_port_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) 
{ 
  	/* Buffer to read the device */
  	//char vport_port_buffer;
	dbg("read from parport");

 	/* Reading port */
  	//vport_port_buffer = inb(0x278);

  	/* We transfer data to user space */
  	//copy_to_user(buf,&vport_port_buffer,1); 
  
  	/* We change the reading position as best suits */
	
  	//if (*f_pos == 0) { 
    	//*f_pos+=1; 
    	return 1; 
  	//} else { 
    	//return 0; 
  	//}
}


ssize_t vport_port_write( struct file *filp, char *buf, size_t count, loff_t *f_pos) 
{

  	//char *tmp;

  	/* Buffer writing to the device */
  	//char vport_port_buffer;

  	//tmp=buf+count-1;
  	//copy_from_user(&vport_port_buffer,tmp,1);

 	/* Writing to the port */
	//outb(vport_port_buffer,0x278);

  return 1; 
}









/**************************************************************/


static int __init vport_init(void)
{
    int retval;
    dbg("vport Module starting");
	
	/* serial usb converter */
	retval = vport_serial_init();

	/* digital io port */
	retval = vport_port_init();

    return retval;
}


static void __exit vport_exit(void)
{
  	dbg("vport Module stopped");


	vport_serial_exit();
	vport_port_exit();
}


module_param(debug,bool,S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug enable or not");


module_init (vport_init);
module_exit (vport_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Benedikt Sauter sauter@ixbat.de");
MODULE_DESCRIPTION("vPort multi IO Interface Expander");
