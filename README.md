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

4. [x] Always show the usage when giving a local direction.(example: ./sws dir) [Fixed in main.c v1.02]

5. [x] No support for IPv6 [Fixed in net.c 1.07]

6. [x] Does not handle simultaneous clients, only sequential. [Fixed in net.c v1.03]

7. [x] Yields 400 for unknown requests (should be not-implemented). [All GET/HEAD/POST are implemented from main.c v1.05, net.c v1.15, http.c v1.05. For HTTP/0.9 simple request, GET is the only decent request, otherwise Bad Requests.]

8. [x] By default only binds on IPv4. [Fixed in patch Dec6/2013 & patch Dec14/2013].

9. [x] Magic numbers on request status code define. [Fixed in net.h v1.06, http.c v1.03]

10. [x] Memory mapping error for "Get / HTTP/1.0". [Fixed in patch Dec6/2013]

11. [x] Always 522 Timeout while dealing requests from web browser. [Fixed in net.c v1.12]

12. [ ] Unexpected feature could occur on some systems which IPV6_V6ONLY is turned on by default.(In such systems, sws will listen on IPv4 or IPv6 only, which against the feature: "By default sws will listen on all IPv4 and IPv6 addresses on this host.")

13. [ ] In daemon model, which accept simultaneous connections would create zombie processes.

#### Notice

1. An HTTP/1.0 server should respond with a 400 message if it cannot determine the length of the request message's content.