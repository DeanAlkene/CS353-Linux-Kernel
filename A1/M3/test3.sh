sudo -S insmod module3.ko
cat /proc/M3_proc
echo 1 > /proc/M3_proc
sudo -S echo 1 > /proc/M3_proc
cat /proc/M3_proc
sudo -S rmmod module3
dmesg | tail -5 #specified number