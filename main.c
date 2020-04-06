#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/usb.h>


#define SANDDISK_VID  0x0781
#define SANDDISK_PID  0x558a

#define B(x) (((x)!=0)?1:0)
#define READ_CAPACITY_LENGTH          0x08
#define be_to_int32(buf) (((buf)[0]<<24)|((buf)[1]<<16)|((buf)[2]<<8)|(buf)[3])
void send_mass_storage_command(struct usb_device *, uint8_t, uint8_t ,uint8_t *, uint8_t , int , uint32_t *);
 void test_mass_storage(struct usb_device *, uint8_t , uint8_t );
static void display_buffer_hex(unsigned char *, unsigned );

static uint8_t cdb_length[256] = {
//	 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  0
	06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  1
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  2
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  3
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  4
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  5
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  6
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  7
	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  8
	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  9
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  A
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  B
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  C
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  D
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  E
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  F
};
struct usbdev_private
{
	struct usb_device *udev;
	unsigned char class;
	unsigned char subclass;
	unsigned char protocol;
	unsigned char ep_in;
	unsigned char ep_out;
};
struct command_block_wrapper {
	uint8_t dCBWSignature[4];
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;
	uint8_t bmCBWFlags;
	uint8_t bCBWLUN;
	uint8_t bCBWCBLength;
	uint8_t CBWCB[16];
};
struct usbdev_private *p_usbdev_info;

static void usbdev_disconnect(struct usb_interface *interface)
{
	printk(KERN_INFO "USBDEV Device Removed\n");
	return;
}

static struct usb_device_id usbdev_table [] = {
	{USB_DEVICE(SANDDISK_VID, SANDDISK_PID)},
	
	{} /*terminating entry*/	
};

static int usbdev_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int i=1;
	unsigned char epAddr, epAttr;
	//struct usb_host_interface *if_desc;
	struct usb_endpoint_descriptor *ep_desc;
	struct usb_device *udev = interface_to_usbdev(interface);
        uint8_t endpoint_in = 0, endpoint_out = 0;
        
 
      if(id->idVendor == SANDDISK_VID)
	{
		printk(KERN_ALERT "known Usb drive detected\n");
                printk(KERN_INFO "vendor_id %x" ,udev->descriptor.idVendor);
               printk(KERN_INFO "product_id %x" ,udev->descriptor.idProduct);
               printk(KERN_INFO "DEVICE CLASS %x", interface->cur_altsetting->desc.bInterfaceClass);
                printk(KERN_INFO "interface subCLASS %x", interface->cur_altsetting->desc.bInterfaceSubClass);
                printk(KERN_INFO "No. of Endpoints = %d\n", interface->cur_altsetting->desc.bNumEndpoints);


        for(i=0;i<interface->cur_altsetting->desc.bNumEndpoints;i++)

           {
		ep_desc = &interface->cur_altsetting->endpoint[i].desc;
		epAddr = ep_desc->bEndpointAddress;
		epAttr = ep_desc->bmAttributes;


            if((epAttr & 0x01)==0x01)
		{
			if(epAddr & 0x80)

				printk(KERN_INFO "EP %d is Isochronous IN\n", i);
			else
				printk(KERN_INFO "EP %d is Isochronous OUT\n", i);
	
		}

             else if((epAttr & 0x2)==0x2)
		{
			if((epAddr & 0x80))
{
				printk(KERN_INFO "EP %d is BULK IN\n", i);

                                endpoint_in=ep_desc->bEndpointAddress;

}
			else
{
				printk(KERN_INFO "EP %d is BULK OUT\n", i);

                                endpoint_out=ep_desc->bEndpointAddress;
}	
		}


             else if((epAttr & 0x3)==0x3)
		{
			if(epAddr & 0x80)
				printk(KERN_INFO "EP %d is Interrupt IN\n", i);
			else
				printk(KERN_INFO "EP %d is Interrupt OUT\n", i);
	
		}
         }	
	}
test_mass_storage(udev,  endpoint_in,  endpoint_out);
return 0;
}


void test_mass_storage(struct usb_device *udev, uint8_t endpoint_in, uint8_t endpoint_out)
{
	int r, size;
	uint8_t lun;
	uint32_t expected_tag;
	uint32_t i, max_lba, block_size;
	int32_t device_size;
	uint8_t cdb[16];	
	uint8_t buffer[64];
	char vid[9], pid[9], rev[5];
	unsigned char *data;
	
		lun = 0;
	

	memset(buffer, 0, sizeof(buffer));
	memset(cdb, 0, sizeof(cdb));
	cdb[0] = 0x12;	
	cdb[4] = 0x24;
 
	send_mass_storage_command(udev, endpoint_out, lun, cdb, 0x80, 0x24, &expected_tag);
	usb_bulk_msg(udev, usb_rcvbulkpipe(udev,endpoint_in), (unsigned char*)&buffer, 0x24, &size, 1000);
	printk(KERN_ALERT "   received %d bytes\n", size);
	// The following strings are not zero terminated
	for (i=0; i<8; i++) {
		vid[i] = buffer[8+i];
		pid[i] = buffer[16+i];
		rev[i/2] = buffer[32+i/2];	
	}
	vid[8] = 0;
	pid[8] = 0;
	rev[4] = 0;
	printk(KERN_ALERT "   VID:PID:REV \"%8s\":\"%8s\":\"%4s\"\n", vid, pid, rev); 
	

	// Read capacity
	printk(KERN_ALERT "Reading Capacity:\n");
	memset(buffer, 0, sizeof(buffer));
	memset(cdb, 0, sizeof(cdb));
	cdb[0] = 0x25;	// Read Capacity
       // cdb[4] = 0x24;
	send_mass_storage_command(udev, endpoint_out, 0, cdb, 0x80, 0x08, &expected_tag);
	usb_bulk_msg(udev, usb_rcvbulkpipe(udev,endpoint_in), (unsigned char*)&buffer, 0x08, &size, 5000);
	printk(KERN_ALERT "received %d bytes \n", size);
	max_lba = be_to_int32(&buffer[0]);
	block_size = be_to_int32(&buffer[4]);
	//device_size = (int)((max_lba+1)*block_size/(1024*1024*1024));
	printk(KERN_ALERT "   Max LBA: %08X, Block Size: %08X", max_lba, block_size);
        //printk(KERN_ALERT " LBA and BLocksize should be multiplied in decimal format then it should be divided by 1024^3 for getting size in gb");
	//printk(KERN_ALERT "received %d bytes \n", size);
	//return 0;
}

void send_mass_storage_command(struct usb_device *udev, uint8_t endpoint, uint8_t lun,
	uint8_t *cdb, uint8_t direction, int data_length, uint32_t *ret_tag)
{
	static uint32_t tag = 1;
	uint8_t cdb_len;
	int i, r, size;
	struct command_block_wrapper cbw;



	cdb_len = cdb_length[cdb[0]];
	memset(&cbw, 0, sizeof(cbw));
	cbw.dCBWSignature[0] = 'U';
	cbw.dCBWSignature[1] = 'S';
	cbw.dCBWSignature[2] = 'B';
	cbw.dCBWSignature[3] = 'C';
	*ret_tag = tag;
	cbw.dCBWTag = tag++;
	cbw.dCBWDataTransferLength = data_length;
	cbw.bmCBWFlags = direction;
	cbw.bCBWLUN = lun;
	// Subclass is 1 or 6 => cdb_len
	cbw.bCBWCBLength = cdb_len;
	memcpy(cbw.CBWCB, cdb, cdb_len);

	i = 0;
	do {
		// The transfer length must always be exactly 31 bytes.
		usb_bulk_msg(udev, usb_sndbulkpipe(udev,endpoint), (unsigned char*)&cbw, 31, &size, 10000);
		
		i++;
	} while ( (i<5));
	
	printk(KERN_ALERT "sent %d CDB bytes\n", cdb_len);
	//return 0;
}

/*Operations structure*/
static struct usb_driver usbdev_driver = {
	name: "usbdev",  
	probe: usbdev_probe, 
	disconnect: usbdev_disconnect, 
	id_table: usbdev_table,
};

int device_init(void)
{
                printk(KERN_ALERT "UAS READ capacity driver inserted");
        	usb_register(&usbdev_driver);
                return 0;
}

int device_exit(void)
{
	usb_deregister(&usbdev_driver);
	printk(KERN_NOTICE "Leaving Kernel\n");
	return 0;
}

module_init(device_init);
module_exit(device_exit);
MODULE_LICENSE("GPL");


