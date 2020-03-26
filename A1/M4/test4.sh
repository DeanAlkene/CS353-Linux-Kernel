sudo -S insmod module4.ko
cat /proc/M4_proc_dir/M4_proc
echo 1 > /proc/M4_proc_dir/M4_proc
cat /proc/M4_proc_dir/M4_proc
echo helloworldhelloworld > /proc/M4_proc_dir/M4_proc
cat /proc/M4_proc_dir/M4_proc
echo helloworldhelloworldhelloworldhelloworld > /proc/M4_proc_dir/M4_proc
cat /proc/M4_proc_dir/M4_proc
sudo -S rmmod module4.ko
dmesg | tail -17 #specified number