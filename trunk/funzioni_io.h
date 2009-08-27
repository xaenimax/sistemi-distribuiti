int estraiTempo(char * risposta_ping);
int estraiPaese(char * risposta_hostinfo);
char * leggiStdout(char * comando);
ssize_t	writen(int fd, const void *buf, size_t n);
int	readline(int fd, void *vptr, int maxlen);
