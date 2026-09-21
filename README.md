# !!!!DISCLAIMER!!!!  
this is just for educational purposes. IMEI modification may be ILLEGAL depending on where you live. DO NOT use IMEIs you don't own.  

Tested only on `PIXEL 10 PRO` and comes with ABSOLUTELY NO WARRANTY. I am not responsible for you bricking your device and starting any termonuclear wars.
YOU HAVE BEEN WARNED  

hi everyone.  

i finally found a way to run deprecated pixel 9 umts_router commands on pixel 10 series of devices  

It may work for other pixel devices but i only tested it on pixel 10 pro (blazer), pixel 10 series has literally the same chip as the pixel 9 series, but because google decided to remove some functionality with A16, some commands do not work on stock modem.img.  

## requirements
- rooted device with some way to run shell commands (termux or adb shell).
- oldest modem.img from pixel 9 devices. you can get it from [here] or extract it from Google's official OTA archives.
- [this](https://github.com/rektstarsceosu/pixel10-imei-repair/blob/main/bin/cp_crash) modem restart binary (or any other way to restart the modem, explained below)  


### 1. get the original devinfo.img and fix it

```
adb shell # or your preferred way to get a shell
su
dd if=/dev/block/by-name/devinfo of=/sdcard/devinfo_original.img
```
you need to replace the IMEI number strings in this image. They are located at offset  
413:imei1xxxxxxxxxx  
447:imei2xxxxxxxxxx  

you can use any hex editor or `sed` if you prefer, they are ASCII encoded strings:  
```
# LC_ALL=C stops multibyte encoding from mangling arbitrary bytes.
LC_ALL=C sed -i "s/imei1xxxxxxxxxx/imei1yyyyyyyyyy/g" devinfo.img
LC_ALL=C sed -i "s/imei2xxxxxxxxxx/imei2yyyyyyyyyy/g" devinfo.img
```

### 2. replace it on the device

send the devinfo_modified.img to the device
```
dd if=/sdcard/devinfo_modified.img of=/dev/block/by-name/devinfo  

```

### 3. reboot the device and also enter factory mode

reboot your device into fastboot mode  
```
adb reboot bootloader
# set it to factory mode
fastboot oem set_config bootmode factory
fastboot reboot

```
you will see a red 'Factory' warning when the device powers on.  

### 4. hotfix the modem partition to use deprecated commands

put your old modem image from pixel 9 to `/sdcard/modem9.img`
i used 14.0.0 [(AD1A.240530.030, Aug 2024) for caiman](https://dl.google.com/dl/android/aosp/caiman-ota-ad1a.240530.030-0fc923fd.zip)  
you can extract the modem with [payload-dumper-go](https://github.com/ssut/payload-dumper-go)  

create a loopback device for modem.img  
```
# /sdcard/modem9.img
mkdir -p /data/local/tmp/mountp
losetup -f
# on pixel10 pro it starts from loop55, NUMBER MAY BE DIFFERENT, ADJUST ACCORDINGLY!!!
losetup /dev/block/loop55 /sdcard/modem9.img
```
mount the old image and replace the modem.img inside modem9.img (i don't know why there is another modem.img inside modem9.img but this modem.img is where commands are executed from.)  
```
# adjust loop device accordingly
mount -t ext4 -o rw,context=u:object_r:modem_img_file:s0 /dev/block/loop55 /data/local/tmp/mountp
mount --bind /data/local/tmp/mountp/images/default/modem.bin /mnt/vendor/modem_img/images/default/modem.bin
```

### 5. crash the modem device so it reads from our new modem.img
you can use the program i included here, or you can build it from source with any arm64 compiler  
or even better you can write a better wrapper  
basically you need to send IOCTL_TRIGGER_CP_CRASH to the modem  
```c
// this is the core idea
int fd = open("/dev/umts_ipc0", O_RDWR);
ioctl(fd, IOCTL_TRIGGER_CP_CRASH, 0);   // _IO('o', 0x34)
```
using the binary i provide  
```
# put it to /data/local/tmp/cp_crash
/data/local/tmp/cp_crash # execute it
```
the modem will crash and restart  


### 6. continue from any other pixel9 imei repair guide

get your imei cp sha  
after modem restarts AT+GOOGGETIMEISHA will be available  
```
exec 3<>/dev/umts_router
echo -e 'AT+GOOGGETIMEISHA\r' >&3
timeout 2 cat <&3
exec 3<&-
```
copy the output (just the sha256sum)  
```
echo EXAMPLEsha256SUMc0e33cb4917083839b7add4cf9d100c8bad17b7 > /mnt/vendor/persist/modem/cpsha
echo ‘AT+GOOGBACKUPNV\r’ > /dev/umts_router
```

### 7. exit factory mode and check your imei

enter fastboot again and exit the factory mode  
```
fastboot oem rm_config bootmode
fastboot reboot
```
check your imei using the dialer,  
dial: *#06#  

### Sources:
```
- claude.ai for writing the wrapper for restarting the modem for me
- extract payload.bin: https://github.com/ssut/payload-dumper-go
- pixel 9 imei restore: https://xdaforums.com/t/restore-imei-numbers-for-pixel-9-pro-and-pixel-9-pro-xl.4714671/
- my hatred against corpos taking away our right to modify/repair our devices and anyone who sells this knowledge for money


```
Thanks for reading :<

