# PIB FIIT STU: Semestrálna práca o DDoS

Princípy Informačnej Bezpečnosti (AS 2020 / 2021). Fakulta informatiky a informačných technológií. Článok: **"Zvýšenie odolnosti webových aplikácií proti útokom typu DDoS, za pomoci horizontálneho škálovania"**.

### Rozčlenenie

- **Documents** - obsahuje špecifikáciu projektu v rozsahu 500 - 900 slov, dva progress reporty vo forme prezentácií na 15 minút a záverečnú správu o projekte o rozsahu minimálne 20 normostrán.
- **Infrastructure** - uchováva konfiguračné súbory pre softvér analyzovaný v semestrálnej práci
  - apache - webový server Apache 2.4
  - bind - BIND9 DNS server s doménou website.home na lokálnej sieti
  - dhcpd - ISC DHCP server so centralizovaním pridelovaním IP adries uzlom v experimentálnej izolovanej sieti
  - docker-compose - Kontainerizované vyvažovače záťaže NGINX a HAProxy pre pozorovanie horizontálneho škálovania inštancií Apache kontajnera.
  - experiments -  nespracované záznamy z priebehu sieťových útokov a Jupyter Notebooky s ich detailným rozborom.
  - keepalived - Dva failover uzly v roliach master a slave.
  - loadbalancer - Konfigurácia pre loadbalancery vystavované DDoS útokom: NGINX a HAProxy 
  - website - webová stránka fotogalérie so uchovávaním obľúbených fotiek v session bez overenie statefull loadbalancing
  - DoS.c - záplavové útoky implementované v jazyku C: UDP flood, TCP SYN flood, Slowloris
- **Writing** -  LaTeX zdrojový kód seminárnej práce a prezentácie progress reportov v OpenDocument Format