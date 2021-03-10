# sudo dd if=2021-01-11-raspios-buster-armhf.img of=/dev/mmcblk0 status=progress
# vim /etc/apt/sources.list
# deb http://mirror.vnet.sk/raspbian/raspbian/
apt update && sudo apt upgrade
apt install nginx haproxy php vim apache2 libapache2-mod-php libapache2-mod-security2 keepalived -y
systemctl disable nginx apache2 nginx haproxy keepalived
systemctl stop nginx apache2 nginx haproxy keepalived

apache2ctl configtest
vim /etc/apache2/apache2.conf

Miroslav:MalinovyServer
pi:MalinovyServer
