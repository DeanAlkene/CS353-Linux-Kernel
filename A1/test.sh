cd M1
sudo -S insmod module1.ko
dmesg | tail -5
sudo -S rmmod module1
cd ../M2
sudo -S insmod module2.ko int_param=10 str_param=hello arr_param=1,2,3 arr_param_fake=1,2,3,4,5,6,7,8,9,10,11,12,13
dmesg | tail -20
sudo -S rmmod module2
cd ../M3
sudo -S insmod module3.ko
cat /proc/M3_proc
echo 1 > /proc/M3_proc
dmesg | tail -10
sudo -S rmmod module3
cd ../M4
sudo -S insmod module4.ko
cat /proc/M4_proc_dir/M4_proc
echo 1 > /proc/M4_proc_dir/M4_proc
dmesg | tail -10
sudo -S rmmod module4