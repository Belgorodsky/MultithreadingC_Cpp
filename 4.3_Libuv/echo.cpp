#include <uv.h>
#include <stdio.h>
#include <cstdlib>

uv_tcp_t server;
uv_loop_t *loop;

void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    uv_write_t *req = (uv_write_t*)malloc(sizeof(uv_write_t));
    uv_write(req, stream, buf, 1, nullptr);
    free(buf->base);
}

void alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
    *buf = uv_buf_init((char*) malloc(size), size);
}

void connection_cb(uv_stream_t* server, int status)
{
    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));

    uv_tcp_init(loop, client);
    uv_accept(server, (uv_stream_t*) client);
    uv_read_start((uv_stream_t*) client, alloc_buffer, read_cb);
}

int main(int argc, char **argv) {

    loop = uv_default_loop();

    struct sockaddr_in addr;
    uv_ip4_addr("127.0.0.1", 12345, &addr);

    uv_tcp_init(loop, &server);
    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
    uv_listen((uv_stream_t *)&server, 128, connection_cb);

    return uv_run(loop, UV_RUN_DEFAULT);
}
