# sudo dd if=2021-01-11-raspios-buster-armhf.img of=/dev/mmcblk0 status=progress
# vim /etc/apt/sources.list
# deb http://mirror.vnet.sk/raspbian/raspbian/
apt update && sudo apt upgrade
apt install nginx haproxy php vim apache2 libapache2-mod-php libapache2-mod-security2 keepalived isc-dhcp-server -y
systemctl disable nginx apache2 nginx haproxy keepalived bind9 isc-dhcp-server
systemctl stop nginx apache2 nginx haproxy keepalived bind9 isc-dhcp-server

a2enmod mpm_prefork
a2enmod ssl
apache2ctl configtest
vim /etc/apache2/apache2.conf

Miroslav:MalinovyServer
pi:MalinovyServer

# openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -subj '/CN=localhost'
# wait on network on boot
