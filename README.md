# sqlog
Light weight process logging using sqlite. Can be used as embedded or linked to an OS package with precompiled sqlite library.

# Instructions to use sqlog with sqlite3 embedded
Get sqlog.c and sqlog.h. You must have sqlite3 amalgamation from https://www.sqlite.org/download.html in order to use sqlog with sqlite embedded in the application. Include "sqlog.h" and the path to "sqlite3.h". 

Compile your application with:

<code>gcc -o [executable_name] sqlog.c /path/to/sqlite3.c [your_C_application.c files] -lpthread -ldl</code>

Notes on embedded usage: Compile time will increase if including sqlite3 source code.

# Instructions to use sqlog with OS library link
This has currently only been tested on Ubuntu LTS 20.04. You will need to get the sqlite dev package:

<code>sudo apt-get install libsqlite3-dev</code>

From there, you should be able to compile your application + sqlog.c and link directly to the precompiled sqlite3 library. This is ideal for development applications and saves time on compilation of the entire sqlite3 source code.

To compile with libsqlite3-dev:

<code>gcc -o [executable_name] sqlog.c [your_C_application files] -lsqlite3 -lpthread -ldl</code>

# Examples
testlog.c contains a simple program that exhibits logging. Compile with either options above and you will see a new .db file, a sqlog table, and a sqlog_[archivetime] table to exhibit sqlog's ability to archive logs.
