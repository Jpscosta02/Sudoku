// comum/util.h
#ifndef UTIL_H
#define UTIL_H

int readn(int fd, char *ptr, int nbytes);
int writen(int fd, char *ptr, int nbytes);
int readline(int fd, char *ptr, int maxlen);
void err_dump(char *msg);

#endif