# NTN

A simple web server runs on Unix-like system.

This project is written during the class CS631, Advanced Programming 
in the UNIX Environment, Fall 2013, SIT.

## Final Project:

#### Description

[sws.1.pdf](http://www.cs.stevens.edu/~jschauma/631A/sws.1.pdf)

#### References

> www.cs.stevens.edu/~jschauma/631A/

> www.linux.die.net

> www.gnu.org

> www.linuxhowtos.org

> www.pubs.opengroup.org

> www.beej.us

#### BUGs History

1. [x] Options Validation Check: dircheck(): closedir() [Fixed in main.c v1.02]

2. [x] Options Validation Check: still at "rudimentary" stage

3. [x] Client~INFO may not be displayed correctly occasionally when using telnet to connect. (Web browser connection behaved better.) [Fixed in net.c v1.04]

4. [x] Always show the usage when giving a local direction. [x] (example: ./sws dir) [Fixed in main.c v1.02]

5. [x] No support for IPv6 [Fixed in net.c 1.07]

6. [x] Does not handle simultaneous clients, only sequential. [Fixed in net.c v1.03]

7. [ ] Yields 400 for unknown requests (should be not-implemented).

8. [x] By default only binds on IPv4. [Fixed in patch Dec6/2013].

9. [x] Magic numbers on request status code define. [Fixed in net.h v1.06, http.c v1.03]

10. [x] Memory mapping error for "Get / HTTP/1.0". [Fixed in patch Dec6/2013]

11. [x] Always 522 Timeout while dealing requests from web browser. [Fixed in net.c v1.12]
