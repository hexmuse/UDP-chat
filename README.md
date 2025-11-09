#  UDP Chat in C 



## A simple chat implementation based on the UDP protocol using POSIX sockets (Linux)


![7777777](https://github.com/user-attachments/assets/43544919-72b5-4545-84bf-bbf341a46a76)


>## Features :dart:

* Users can register with a username and password
* Users can log in using their credentials
* Server sends notifications when users log in and out
* All communication is carried out via UDP which means there are no guarantees of delivery and message order


## System requirements :computer:

**Operating system:** Linux (or other POSIX compatible)

**Compiler:** GCC (or other compatible)

**POSIX Threads Library [(pthread)](https://en.wikipedia.org/wiki/Pthreads)**



>## Install :rocket:
```
git clone https://github.com/hexmuse/UDP-chat.git
```




>## Build :hammer:
Go to the root directory of the project

```
cd UDP-chat
```

Build the project using Makefile:

```
make all
```

After a successful build you will receive server and client executable files in the bin/ directory







> ## Commands :pushpin:
```
/register <username> <password>
```
```
/login <username> <password>
```
```
/quit
```















