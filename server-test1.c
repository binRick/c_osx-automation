/*******************/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
/*******************/
#include "include/strsplit.h"
/*******************/
#include "src/re.c"
/*******************/
#include "src/server.c"
#include <parson.h>
/*******************/
//#include "src/strsplit.c"
/*******************/
char *GetNameAndTitleOfActiveWindow = "./bin/GetNameAndTitleOfActiveWindow.sh";


/*******************/


char * aw(){
  FILE *fp;
  char path[1035];
  char ret[2048];

  fp = popen(GetNameAndTitleOfActiveWindow, "r");
  if (fp == NULL) {
    printf("Failed to run command\n");
    exit(1);
  }

  while (fgets(path, sizeof(path), fp) != NULL) {
    sprintf(&ret, "%s%s", ret, path);
  }

  pclose(fp);
  return(strdup(ret));
}


int main(int argc, const char *const argv[]) {
  char        *ls    = aw();
  JSON_Value  *J     = json_parse_string(ls);
  int         jt     = json_value_get_type(J);
  JSON_Object *R     = json_value_get_object(J);
  char        *name  = json_object_get_string(R, "APP_NAME");
  char        *title = json_object_get_string(R, "WINDOW_TITLE");
  int         pid    = json_object_get_number(R, "PID");


  fprintf(stdout, "\n\nname:%s\ntitle:%s\npid:%d\n", name, title, pid);


  assert(((int)json_value_get_type(J) == (int)JSONObject));
  fprintf(stdout, "\n\njt:%d|JSONArray:%d|JSONObject:%d|\n\n", jt, JSONArray, JSONObject);
  fprintf(stdout, "\n\nactive window:      '%s'         \n\n", ls);
  fprintf(stdout, "\n\nactive window:      '%s'         \n\n", ls);
  return(server_main(argc, argv));
} /* main */
