#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/input.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/slab.h>
// include cai nay de cai de co the su dung cac bien
//trong ha Idtable trong ham probe tu dong nhan dien
// khong can phai tu tim vendor_id va usb_produc_id
#include <linux/hid.h>

#define DRIVER_NAME "usb_mouse_ioctl_driver"
#define DEVICE_NAME "usbmouse"

// Cai nay la ma chuot da dung de test
#define USB_MOUSE_VENDOR_ID    0x30FA
#define USB_MOUSE_PRODUCT_ID   0xc0400

// day se la bien dung de coppy yeu cau tu user xuong
#define MOUSE_IOCTL_BASE      'M'
#define IOCTL_GET_STATUS      _IOR(MOUSE_IOCTL_BASE, 1, struct mouse_data)

// day se la bien luu gia tri ngat, va coppy len user
struct mouse_data {
	__u8 left;
	__u8 right;
	__u8 wheel_click;
	__s8 x;
	__s8 y;
	__s8 wheel;
};

// cac bien dung de khoi tao driver device
static struct usb_device *mouse_dev;
static struct urb *mouse_urb;
static struct usb_interface *mouse_intf;

static struct mouse_data current_data;
static struct mutex data_lock;

static dev_t dev_number;
static struct cdev mouse_cdev;
static struct class *mouse_class;

static char *mouse_buf;

// bien nay se thay doi cac gia tri user se doc duoc neu khong co ngat
// tranh truong hop doc lien tuc cung 1 gia tri du khong co ngat
static int data_ready = 0;

// khi co ngat, kernel se vao ham nay
static void usb_mouse_irq(struct urb *urb)
{
    if (urb->status == 0) {
	//ham nay ko cho phep nhieu lenh truy cap vao vung du lieu cung 1 luc
	//vi du nhu ham IOCTL va ngat IRQ co the se vao cung 1 luc
        mutex_lock(&data_lock);
	// cap nhap cac gia tri ngat vao bien se duoc coppy len user
        current_data.left  = mouse_buf[0] & 0x01;
        current_data.right = mouse_buf[0] & 0x02;
	current_data.wheel_click = mouse_buf[0] & 0x04;
        current_data.x     = mouse_buf[1];
        current_data.y     = mouse_buf[2];
        current_data.wheel = mouse_buf[4];

	data_ready = 1;

	printk(KERN_INFO "USB Mouse X: %d, mouse Y: %d, mouse wheel : %d",mouse_buf[1],mouse_buf[2],mouse_buf[4]);
	// khi da ghi xong du lieu vao thi se cho phep cac lenh truy cap vao lai
        mutex_unlock(&data_lock);
    }

    usb_submit_urb(mouse_urb, GFP_ATOMIC);
}
// ham nay se giao tiep truc tiep voi user

static long mouse_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if (cmd == IOCTL_GET_STATUS) {
        struct mouse_data data;
	// tuong tuy nhu da giai thich o tren
        mutex_lock(&data_lock);
	// data duoc cap nhap thi co nghia la co gia tri ngat
	//thi se gui len user toan bo gia tri
	if (data_ready == 1){
        	data = current_data;
		data_ready = 0;}
	//con neu ko thi cac gia tri X va Y va wheel se la gia tri tuong doi
	// co nghia la hien tai khong co duy chuyen
	else{
	data.left = current_data.left;
	data.right = current_data.right;
	data.wheel_click = current_data.wheel_click;
	data.x = 0;
	data.y = 0;
	data.wheel = 0;
	}

        mutex_unlock(&data_lock);
	// ham nay coppy gia tri bien giao tiep len user
 	if (copy_to_user((void __user *)arg, &data, sizeof(data)))
		return -EFAULT;
        return 0;

    }
    return -EINVAL;
}
// 2 ham duoi co chuc nang thong bao
// khi user dong mo file 
static int mouse_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "file driver da duoc mo thanh cong\n");
	return 0;
}

static int mouse_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "file driver da duoc dong lai\n");
	return 0;
}
// la bang ham da tao voi charter device va user space
static const struct file_operations mouse_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = mouse_ioctl,
    .open           = mouse_open,
    .release        = mouse_release,

};
// ham nay se chay khi lan dau nhan dien USB Mouse
static int usb_mouse_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int pipe, error;

	iface_desc = interface->cur_altsetting;
	endpoint = &iface_desc->endpoint[0].desc;

	pipe = usb_rcvintpipe(interface_to_usbdev(interface), endpoint->bEndpointAddress);

	mouse_buf = kmalloc(endpoint->wMaxPacketSize, GFP_KERNEL);
	if (!mouse_buf)
		return -ENOMEM;

	mouse_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!mouse_urb) {
		kfree(mouse_buf);
		return -ENOMEM;
    	}

	usb_fill_int_urb(mouse_urb,
		interface_to_usbdev(interface),
		pipe,
		mouse_buf,
		endpoint->wMaxPacketSize,
		usb_mouse_irq,
		NULL,
		endpoint->bInterval);

 	mouse_dev = interface_to_usbdev(interface);
	mouse_intf = interface;

	usb_submit_urb(mouse_urb, GFP_KERNEL);
// phan nay de dang ki device driver
	alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
	cdev_init(&mouse_cdev, &mouse_fops);
	mouse_cdev.owner = THIS_MODULE;
	cdev_add(&mouse_cdev, dev_number, 1);
	mouse_class = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(mouse_class, NULL, dev_number, NULL, DEVICE_NAME);

	mutex_init(&data_lock);

	printk(KERN_INFO DRIVER_NAME ": USB mouse da ket noi thanh cong\n");
	return 0;
}
// ham nay se chay khi rut USB mouse ra
static void usb_mouse_disconnect(struct usb_interface *interface)
{
	usb_kill_urb(mouse_urb);
	usb_free_urb(mouse_urb);
	kfree(mouse_buf);

	device_destroy(mouse_class, dev_number);
	class_destroy(mouse_class);
	cdev_del(&mouse_cdev);
	unregister_chrdev_region(dev_number, 1);

    printk(KERN_INFO DRIVER_NAME ": USB mouse da bi mat ket noi\n");
}
// bien nay la danh sach cac USB mouse se duoc nhan dien tu dong
static struct usb_device_id usb_mouse_table[] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	{}
};
// cap nhap cac gia tri bien nay vao he thong
MODULE_DEVICE_TABLE(usb, usb_mouse_table);

// neu tra cuu VendorID va ProductID trung voi bien usb_mouse_table thi se chay ham probe
static struct usb_driver usb_mouse_driver = {
	.name = DRIVER_NAME,
	.id_table = usb_mouse_table,
	.probe = usb_mouse_probe,
	.disconnect = usb_mouse_disconnect,
};

module_usb_driver(usb_mouse_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tran Le Thien An");
MODULE_DESCRIPTION("USB Mouse Driver with IOCTL");
