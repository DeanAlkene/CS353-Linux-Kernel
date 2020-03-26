sudo -S insmod module2.ko int_param=10 str_param=hello arr_param=1,2,3 arr_param_fake=1,2,3,4,5,6,7,8,9,10,11,12,13
sudo -S rmmod module2
dmesg | tail -23 #specified number
sudo -S insmod module2.ko int_param=20 arr_param=4,5,6,7,8
sudo -S rmmod module2
dmesg | tail -10 #specified number