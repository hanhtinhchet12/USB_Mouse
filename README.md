# RaspberryPi USB Mouse IOCTL

## Authors:
- Trần Lê Thiên Ân (22146265)
- Đỗ Đức Tuấn (22146440)
- Đoàn Mạnh Kha (22146326)
- Đào Chí Quân (22146380)

---

## Overview
This project demonstrates how to manually connect and control a USB mouse on a Raspberry Pi using IOCTL. It involves uninstalling the default mouse driver, installing a custom driver, and testing mouse input.

---

## Usage

### Step 1: Connect the Mouse
1. **Connect the USB mouse to the Raspberry Pi board.**

---

### Step 2: Delete the Default Mouse Driver
1. Use the `lsusb` command to find the mouse's device ID:
    ```bash
    admin@raspberrypi:~ $ lsusb
    Bus 001 Device 005: ID 30fa:0400  USB Optical Mouse
    Bus 001 Device 004: ID 0424:7800 Microchip Technology, Inc. (formerly SMSC)
    Bus 001 Device 003: ID 0424:2514 Microchip Technology, Inc. (formerly SMSC) USB 2.0 Hub
    Bus 001 Device 002: ID 0424:2514 Microchip Technology, Inc. (formerly SMSC) USB 2.0 Hub
    Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
    ```

2. Use `lsusb -t` to find the correct port of the mouse:
    ```bash
    admin@raspberrypi:~ $ lsusb -t
    /:  Bus 01.Port 1: Dev 1, Class=root_hub, Driver=dwc_otg/1p, 480M
        |__ Port 1: Dev 2, If 0, Class=Hub, Driver=hub/4p, 480M
            |__ Port 1: Dev 3, If 0, Class=Hub, Driver=hub/3p, 480M
                |__ Port 1: Dev 4, If 0, Class=Vendor Specific Class, Driver=lan78xx, 480M
            |__ Port 2: Dev 5, If 0, Class=Human Interface Device, Driver=usbhid, 1.5M
    ```

    - Example: The address is `1-1.2:1.0` (Bus 1 - Port 1. Port 2 : SubDevice (usually 1:0)).

3. Run the `bind.sh` script with the found address:
    ```bash
    admin@raspberrypi:~/test_mouse $ sudo bash bind.sh 1-1.2:1.0
    Rebinding 1-1.2:1.0...
    ```

---

### Step 3: Install the Module
1. Compile the driver:
    ```bash
    admin@raspberrypi:~/test_mouse $ make
    ```

2. Install the driver module:
    ```bash
    admin@raspberrypi:~/test_mouse $ sudo insmod mouse_device_driver.ko
    ```

---

### Step 4: Verify Installation
1. Check the `dmesg` command for successful installation:
    ```bash
    admin@raspberrypi:~/test_mouse $ dmesg
    ```

    - If successful, the following messages will appear:
        ```text
        [ 1091.583291] usb_mouse_ioctl_driver: USB mouse da ket noi thanh cong
        [ 1091.583462] usbcore: registered new interface driver usb_mouse_ioctl_driver
        ```

---

### Step 5: Run the Demo Code
1. Compile the test program:
    ```bash
    admin@raspberrypi:~/test_mouse $ gcc test_user.c -o run
    ```

2. Run the test program:
    ```bash
    admin@raspberrypi:~/test_mouse $ sudo ./run
    ```

3. Move the mouse and observe the changes in the output data values.

---

## Troubleshooting
- Ensure the mouse is connected properly to the Raspberry Pi.
- Verify the correct address using `lsusb` and `lsusb -t`.
- If the driver installation fails, check the `dmesg` logs for error messages.
- Ensure all dependencies for building the module are installed.

---

## License
This project is licensed under the MIT License. See `LICENSE` for details.
