#include <stdio.h>
#include <stdlib.h>
/*******************/
#include "src/re.c"
#include "src/server.c"


void MATCH_STRING(char *NAME, char *STRING, char *REGEX){
  int  _l;
  char *n  = strdup(NAME);
  char *s  = strdup(STRING);
  re_t _re = re_compile(REGEX);
  int  _m  = re_matchp(_re, s, &_l);

  if (_m != -1) {
    printf("%s match at idx %i, %i chars long.\n", n, _m, _l);
  }else{
    printf("NO %s match\n", n);
  }
}


char * aw(){
  char *cmd = "./osa/GetNameAndTitleOfActiveWindow.osa";
  FILE *fp;
  char path[1035];
  char ret[2048];

  fp = popen(cmd, "r");
  if (fp == NULL) {
    printf("Failed to run command\n");
    exit(1);
  }

  while (fgets(path, sizeof(path), fp) != NULL) {
    // printf("STDOUT>      '%s'       ", path);
    sprintf(&ret, "%s%s", ret, path);
  }

  /* close */
  pclose(fp);
  return(strdup(ret));
}


int main(int argc, const char *const argv[]) {
  char *ls = aw();
/* Standard int to hold length of match */
  int  match_length; int l0, l1, l_pid, l_app_name_start, l_app_name_end;

/********************************************************************************************/
  MATCH_STRING("START END", ls, "\\s*<START>\\s*</END>\\s*");
  MATCH_STRING("PID", ls, "<PID>, \\d*, </PID>");
  MATCH_STRING("Application Name", ls, "<APP_NAME>, \\s*, </APP_NAME>");
/********************************************************************************************/
//    re_t _pid = re_compile("<PID>, \\d*, </PID>");

/********************************************************************************************/
/* Standard null-terminated C-string to search: */
  const char *string_to_search = "ahem.. 'hello world !' ..";

/* Compile a simple regular expression using character classes, meta-char and greedy + non-greedy quantifiers: */
  re_t pattern = re_compile("[Hh]ello [Ww]orld\\s*[!]?");
  re_t p0      = re_compile("<START>\\s*</END>");
  re_t p1      = re_compile("\\s*</END>\\s*");


//re_t p_app_name = re_compile("\\s*<APP_NAME>, \\s*, </APP_NAME>\\s*");
  re_t p_app_name_start = re_compile("\\s*<APP_NAME>\\s*");
  re_t p_app_name_end   = re_compile("\\s*</APP_NAME>\\s*");
  int  m0               = re_matchp(p0, ls, &l0);
  int  m1               = re_matchp(p1, ls, &l1);
  int  m_app_name_start = re_matchp(p_app_name_start, ls, &l_app_name_start);
  int  m_app_name_end   = re_matchp(p_app_name_end, ls, &l_app_name_end);

  printf("%s\n", ls);
  if (m_app_name_start != -1 && m_app_name_end) {
    printf("start> app name match at idx %i, %i chars long.\n", m_app_name_start, l_app_name_start);
    printf("end>   app name match at idx %i, %i chars long.\n", m_app_name_end, l_app_name_end);
  }else{
    printf("NO app name match\n");
  }
  if (m0 != -1) {
    printf("XX match at idx %i, %i chars long.\n", m0, l0);
    if (m1 != -1) {
      printf("XX m1m at idx %i, %i chars long.\n", m1, l1);
    }else{
      printf("XX m1m no match");
    }
  }else{
    printf("XX no match");
  }

  int match_idx = re_matchp(pattern, string_to_search, &match_length);

  if (match_idx != -1) {
    printf("match at idx %i, %i chars long.\n", match_idx, match_length);
  }else{
    printf("no match");
  }
  exit(1);
  fprintf(stdout, "\n\nactive window:      '%s'         \n\n", ls);
  fprintf(stdout, "\n\nactive window:      '%s'         \n\n", ls);


  return(server_main(argc, argv));
} /* main */
