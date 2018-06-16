# Service Control Panel

Simple logs collector and service manager. 

Functions
------------
* monitoring services states
* install/uninstall services
* start, stop, restart services
* collect logs from redis lists (you may use https://bitbucket.org/Mexanik/rlog_async for logging)
* filter, sort and save logs

Usage
------------
Control services works only for Windows with NSSE (nsse.exe mast be in same folder with SCP)

**Supported Redis versions**: 3.2+
For log`s refrashing in real time notification mast be enabled in redis.conf (notify-keyspace-events El). 

Build
------------
Qt 5 (msvc15+ on Windows)
