echo "Rebinding $1..."
echo -n "$1" > /sys/bus/usb/drivers/usbhid/unbind
