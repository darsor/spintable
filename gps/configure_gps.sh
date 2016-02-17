#!/bin/bash
sudo gpsd /dev/ttyAMA0 -F /var/run/gpsd.sock -S 2948
echo gps daemon running on poart 2948
