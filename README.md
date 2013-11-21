# NTN

A simple web server runs on Unix-like system.

This project is written during the class CS631, Advanced Programming 
in the UNIX Environment, Fall 2013, SIT.

## Final Project:

#### DESCRIPTION

[sws.1.pdf](http://www.cs.stevens.edu/~jschauma/631A/sws.1.pdf)

#### References:

> http://www.cs.stevens.edu/~jschauma/631A/

> http://linux.die.net/man/

> http://www.gnu.org/software/libc/manual/html_node/Internet-Address-Formats.html

> http://www.linuxhowtos.org/C_C++/socket.htm

> http://pubs.opengroup.org/

#### Pending BUGs

(Fixed in main.c v1.02)1. Options Validation Check: dircheck(): closedir()

2. Options Validation Check: still at "rudimentary" stage

3. Client~INFO may not be displayed correctly occasionally when using telnet to connect. (Web browser connection behaved better.)

(Fixed in main.c v1.02)4. Always show the usage when giving a local direction. (example: ./sws dir)

5. No support for IPv6 

(Fixed in net.c v1.03)6. Does not handle simultaneous clients, only sequential.
