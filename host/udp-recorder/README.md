# UDP Server / Recorder

Server / Recorder for high-speed IMU data over ethernet, communication between IMU device using Universal Datagram Protocol (UPD)

## Dependencies 
* jsonl-recorder (https://github.com/AaltoVision/jsonl-recorder)

## Network settings and Connection
Depends on host device. Network needs to be set up on master machine for Wiznet to be able to connect. Connect ethernet device and IP address with
```bash
sudo ip addr add 192.168.2.2/24 dev enp0s31f6
```

ifconfig can be used to check if the operation succeeded
```bash
ifconfig
cd / && cd etc/netplan
sudo nano 01-netcfg.yaml
sudo netplan try
```

```console
GNU nano 4.8                                                01-netcfg.yaml                                                          
This file describes the network interfaces available on your system
For more information, see netplan(5).
network:
  version: 2
  renderer: networkd
  ethernets:
    enp0s31f6:
      dhcp4: no
      addresses: [192.168.2.2/24]
      gateway4: 192.168.0.255 
```
