#define DBG_H_DEF_ONCE
#define DBG_H_NO_WARNING
#define HTTP_SERVER_PORT    8085
#define ACTIVE_PID_CMD      "~/Desktop/applescript-experiments/GetPidOfActiveWindow.osa"
/********************************/
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
/********************************/
#include "../include/server.h"
/********************************/
#include "../deps/dbg.h/dbg.h"
#include "../deps/generic-print/print.h"
#include "../deps/subprocess.h/subprocess.h"
/********************************/
#include "../include/ansicodes.h"
#include "../include/strconv.h"
/********************************/
#include "../src/stringbuffer.c"
#include "../src/stringfn.c"
///#include "include/commander.h"
//#include "src/server-commander.c"
/********************************/
#include "../src/bytes.c"
#include "../src/strsplit.c"
/********************************/
#include "../src/lswin.c"
#include "../src/movewin.c"
#include "../src/winutils.c"
#define HTTPSERVER_IMPL
#include "../deps/httpserver.h/httpserver.h"
/********************************/
#define RESPONSE    "Hello, World!"
/********************************/
char * run_proc();

/********************************/
int chunk_count = 0;
/********************************/
/********************************/
typedef struct window_t window_t;
typedef struct window_t {
  char *name;
  char *title;
  char *line;
  int  x; int y;
  int  width; int height;
} window_t;


/********************************/
void process_ls_win_output(char *output){
  size_t qty;
  char   **lines = strsplit_count(output, "\n", &qty);

  fprintf(stdout, "acquired %lu lines from %lu bytes\n", qty, strlen(output));
  for (int i = 0; i < qty; i++) {
    char *line = lines[i];
    if (strlen(line) < 1) {
      continue;
    }
    size_t   sp_qty;
    char     **spaced = strsplit_count(line, " ", &sp_qty);
    window_t *w       = malloc(sizeof(window_t));
    w->line   = strdup(line);
    w->name   = strdup(spaced[0]);
    w->x      = str_to_int32(spaced[sp_qty - 4]);
    w->y      = str_to_int32(spaced[sp_qty - 3]);
    w->height = str_to_int32(spaced[sp_qty - 2]);
    w->width  = str_to_int32(spaced[sp_qty - 1]);

    fprintf(stderr,
            AC_RESETALL AC_REVERSED AC_BLUE " - #%d/%lu - %s\n" AC_RESETALL " -> "
            AC_RESETALL AC_REVERSED AC_YELLOW "%s" AC_RESETALL " "
            AC_RESETALL AC_REVERSED AC_GREEN "spaced qty: %lu" AC_RESETALL " "
            AC_RESETALL AC_REVERSED AC_RED "==%s==" AC_RESETALL " "
            AC_RESETALL AC_REVERSED AC_BLUE "==Width/Height==%d/%d==" AC_RESETALL " "
            AC_RESETALL AC_REVERSED AC_BLUE "==X/Y==%d/%d==" AC_RESETALL
            "\n",
            i + 1,
            qty,
            bytes_to_string(strlen(lines[i])),
            lines[i],
            sp_qty,
            w->name,
            w->width, w->height,
            w->x, w->y
            );
  }
}
/********************************/


int request_target_is(struct http_request_s *request, char const *target) {
  http_string_t url = http_request_target(request);
  int           len = strlen(target);

  return(len == url.len && memcmp(url.buf, target, url.len) == 0);
}


void chunk_cb(struct http_request_s *request) {
  chunk_count++;
  struct http_response_s *response = http_response_init();

  http_response_body(response, RESPONSE, sizeof(RESPONSE) - 1);
  if (chunk_count < 3) {
    http_respond_chunk(request, response, chunk_cb);
  } else {
    http_response_header(response, "Foo-Header", "bar");
    http_respond_chunk_end(request, response);
  }
}

typedef struct {
  char                   *buf;
  struct http_response_s *response;
  int                    index;
} chunk_buf_t;


void chunk_req_cb(struct http_request_s *request) {
  http_string_t str           = http_request_chunk(request);
  chunk_buf_t   *chunk_buffer = (chunk_buf_t *)http_request_userdata(request);

  if (str.len > 0) {
    memcpy(chunk_buffer->buf + chunk_buffer->index, str.buf, str.len);
    chunk_buffer->index += str.len;
    http_request_read_chunk(request, chunk_req_cb);
  } else {
    http_response_body(chunk_buffer->response, chunk_buffer->buf, chunk_buffer->index);
    http_respond(request, chunk_buffer->response);
    free(chunk_buffer->buf);
    free(chunk_buffer);
  }
}

struct http_server_s *poll_server;


void handle_request(struct http_request_s *request) {
  chunk_count = 0;
  http_request_connection(request, HTTP_AUTOMATIC);
  struct http_response_s *response = http_response_init();

  http_response_status(response, 200);
  if (request_target_is(request, "/echo")) {
    http_string_t body = http_request_body(request);
    http_response_header(response, "Content-Type", "text/plain");
    http_response_body(response, body.buf, body.len);
  } else if (request_target_is(request, "/ls")) {
    char          *out = run_proc();
    //process_ls_win_output(out);
    http_string_t ua = http_request_header(request, "Host");
    http_response_header(response, "Content-Type", "text/plain");
    http_response_body(response, out, strlen(out));
  } else if (request_target_is(request, "/host")) {
    http_string_t ua = http_request_header(request, "Host");
    http_response_header(response, "Content-Type", "text/plain");
    http_response_body(response, ua.buf, ua.len);
  } else if (request_target_is(request, "/poll")) {
    while (http_server_poll(poll_server) > 0) {
    }
    http_response_header(response, "Content-Type", "text/plain");
    http_response_body(response, RESPONSE, sizeof(RESPONSE) - 1);
  } else if (request_target_is(request, "/empty")) {
    // No Body
  } else if (request_target_is(request, "/chunked")) {
    http_response_header(response, "Content-Type", "text/plain");
    http_response_body(response, RESPONSE, sizeof(RESPONSE) - 1);
    http_respond_chunk(request, response, chunk_cb);
    return;
  } else if (request_target_is(request, "/chunked-req")) {
    chunk_buf_t *chunk_buffer = (chunk_buf_t *)calloc(1, sizeof(chunk_buf_t));
    chunk_buffer->buf      = (char *)malloc(512 * 1024);
    chunk_buffer->response = response;
    http_request_set_userdata(request, chunk_buffer);
    http_request_read_chunk(request, chunk_req_cb);
    return;
  } else if (request_target_is(request, "/large")) {
    chunk_buf_t *chunk_buffer = (chunk_buf_t *)calloc(1, sizeof(chunk_buf_t));
    chunk_buffer->buf      = (char *)malloc(25165824);
    chunk_buffer->response = response;
    http_request_set_userdata(request, chunk_buffer);
    http_request_read_chunk(request, chunk_req_cb);
    return;
  } else if (request_target_is(request, "/headers")) {
    int           iter = 0, i = 0;
    http_string_t key, val;
    char          buf[512];
    while (http_request_iterate_headers(request, &key, &val, &iter)) {
      i += snprintf(buf + i, 512 - i, "%.*s: %.*s\n", key.len, key.buf, val.len, val.buf);
    }
    http_response_header(response, "Content-Type", "text/plain");
    http_response_body(response, buf, i);
    return(http_respond(request, response));
  } else {
    http_response_header(response, "Content-Type", "text/plain");
    http_response_body(response, RESPONSE, sizeof(RESPONSE) - 1);
  }
  http_respond(request, response);
} /* handle_request */

struct http_server_s *server;


void handle_sigterm(int signum) {
  (void)signum;
  free(server);
  free(poll_server);
  exit(0);
}


int http() {
  signal(SIGTERM, handle_sigterm);
  server      = http_server_init(HTTP_SERVER_PORT, handle_request);
  poll_server = http_server_init(8081, handle_request);
  http_server_listen_poll(poll_server);
  http_server_listen(server);
}


/********************************/
char * run_proc(){
  const char          *command_line[] = { "/usr/local/bin/lswin", "-j", NULL };
  struct subprocess_s subprocess;
  int                 ret                = -1;
  static char         data[1048576 + 1]  = { 0 };
  static char         edata[1048576 + 1] = { 0 };
  unsigned            index              = 0;
  unsigned            eindex             = 0;
  unsigned            bytes_read         = 0;
  unsigned            ebytes_read        = 0;

  assert(subprocess_create(command_line, 0, &subprocess) == 0);

  do {
    bytes_read = subprocess_read_stdout(&subprocess, data + index, sizeof(data) - 1 - index);
    index     += bytes_read;
  } while (bytes_read != 0);
  char *out = stringfn_trim(data);

  do {
    ebytes_read = subprocess_read_stdout(&subprocess, edata + eindex, sizeof(edata) - 1 - eindex);
    eindex     += ebytes_read;
  } while (ebytes_read != 0);
  char *err = stringfn_trim(edata);

  if (strlen(err) > 0) {
    fprintf(stderr, "ERR:%s\n", err);
  }

  assert(subprocess_join(&subprocess, &ret) == 0);
  assert(subprocess_destroy(&subprocess) == 0);
  return(strdup(out));
}


int server_main(int argc, const char *const argv[]) {
  return(http());
}
