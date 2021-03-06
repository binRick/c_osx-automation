
#include "../include/commander.h"
#include <stdio.h>


static void verbose() {
  printf("verbose: enabled\n");
}


static void required(command_t *self) {
  printf("required: %s\n", self->arg);
}


static void optional(command_t *self) {
  printf("optional: %s\n", self->arg);
}


int server_commander(int argc, char **argv) {
  command_t cmd;

  command_init(&cmd, argv[0], "0.0.1");
  command_option(&cmd, "-v", "--verbose", "enable verbose stuff", verbose);
  command_option(&cmd, "-r", "--required <arg>", "required arg", required);
  command_option(&cmd, "-o", "--optional [arg]", "optional arg", optional);
  command_parse(&cmd, argc, argv);
  printf("additional args:\n");
  for (int i = 0; i < cmd.argc; ++i) {
    printf("  - '%s'\n", cmd.argv[i]);
  }
  command_free(&cmd);
  return(0);
}
