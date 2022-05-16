# oncrpc4-win32

ONCRPC v4 for windows as described in [RFC 5531](https://tools.ietf.org/html/rfc5531) for distributing applications over one or more computers.

ONC/RPC was originally developed by SUN, this code is based on RPCSRC4.0 bundled with current NetBSD distributions.

````
$ ./rpcinfo.exe
   program version netid     address                service    owner
   100000    4    tcp       0.0.0.0.0.111          portmapper superuser
   100000    3    tcp       0.0.0.0.0.111          portmapper superuser
   100000    2    tcp       0.0.0.0.0.111          portmapper superuser
   100000    4    udp       0.0.0.0.0.111          portmapper superuser
   100000    3    udp       0.0.0.0.0.111          portmapper superuser
   100000    2    udp       0.0.0.0.0.111          portmapper superuser
   100000    4    tcp6      ::.0.111               portmapper superuser
   100000    3    tcp6      ::.0.111               portmapper superuser
   100000    4    udp6      ::.0.111               portmapper superuser
   100000    3    udp6      ::.0.111               portmapper superuser
   100000    4    local     /var/run/rpcbind.sock  portmapper superuser
   100000    3    local     /var/run/rpcbind.sock  portmapper superuser
   100000    2    local     /var/run/rpcbind.sock  portmapper superuser
````

