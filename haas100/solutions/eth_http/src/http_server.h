#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Start the embedded HTTP server (lwIP sockets) on port 80. */
int http_server_start(void);

#ifdef __cplusplus
}
#endif

#endif /* HTTP_SERVER_H */
