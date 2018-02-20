#!/bin/bash

echo -e "\nBackground Traffic"
tshark -r $1 -qz io,phs,"!ip.addr==$2 && !dns" | grep "ip "

echo -e "\nDNS Traffic"
tshark -r $1 -qz io,phs,dns | grep "ip "

echo -e "\nTotal Test Volume"
tshark -r $1 -qz io,phs,ip.addr==$2 | grep "ip "

echo -e "\nData Packet"
tshark -r $1 -qz io,phs,"ip.addr==$2 && ip.len>=100" | grep "ip "

echo -e "\nACK Packet"
tshark -r $1 -qz io,phs,"ip.addr==$2 && ip.len<100" | grep "ip "