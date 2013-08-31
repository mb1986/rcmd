g++ -std=c++11 -Wall -IC:\MinGW\msys\1.0\mingw\include -c rcmd.cpp -o rcmd.o
g++ -std=c++11 -Wall -IC:\MinGW\msys\1.0\mingw\include -c ssh_executor.cpp -o ssh_executor.o
g++ -std=c++11 -Wall rcmd.o ssh_executor.o -LC:\MinGW\msys\1.0\mingw\lib -lssh2 -lssl -lcrypto -lws2_32 -lgdi32 -lz -o rcmd
