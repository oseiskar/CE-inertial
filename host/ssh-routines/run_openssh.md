
# config netplan interface, check config file
sudo netplan generate && sudo netplan apply

# install (apt install squid) and config squid (etc/squid/squid.conf -> http_access deny all to http_access allow all)
sudo service squid start

# run proxy
sudo ssh -R 3129:localhost:3128 jetson@192.168.2.112

# on jetson add proxy to etc/environment and source environment
http_proxy="http://127.0.0.1:3129"
https_proxy="https://127.0.0.1:3129"

# install netplan (apt install netplan.io), check config file


# optional: set ssh keys, ssh-keygen & ssh-copy-id user@addr

# optional: pulling files 
scp jetson@192.168.2.112:/home/jetson/Desktop/config.yaml .
