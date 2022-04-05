#include "../include/winutils.h"

#define ME            "lswin"
#define USAGE         "usage: " ME " [-h] [-l] [-i id] [title]\n"
#define FULL_USAGE    USAGE                                           \
  "    -h       display this help text and exit\n"                    \
  "    -l       long display, include window ID column in output\n"   \
  "    -i id    show only windows with this window ID (-1 for all)\n" \
  "    title    pattern to match \"Application - Title\" against\n"

typedef struct {
  int longDisplay;     /* include window ID column in output */
  int id;              /* show only windows with this window ID (-1 for all) */
  int numFound;        /* out parameter, number of windows found */
} LsWinCtx;


/* Callback for EnumerateWindows() prints title of each window it encounters */
void PrintWindow(CFDictionaryRef window, void *ctxPtr) {
  LsWinCtx *ctx        = (LsWinCtx *)ctxPtr;
  int      windowId    = CFDictionaryGetInt(window, kCGWindowNumber);
  char     *appName    = CFDictionaryCopyCString(window, kCGWindowOwnerName);
  char     *windowName = CFDictionaryCopyCString(window, kCGWindowName);
  char     *title      = windowTitle(appName, windowName);
  CGPoint  position    = CGWindowGetPosition(window);
  CGSize   size        = CGWindowGetSize(window);

  if (ctx->id == -1 || ctx->id == windowId) {
    if (ctx->longDisplay) {
      printf("%d - ", windowId);
    }
    printf(
      "%s - %d %d %d %d\n", title,
      (int)position.x, (int)position.y,
      (int)size.width, (int)size.height
      );
    ctx->numFound++;
  }
  free(title);
  free(windowName);
  free(appName);
}


int lswin_main(int argc, char **argv) {
  LsWinCtx ctx;
  int      ch;
  char     *pattern = NULL;

#define DIE(msg)    { fprintf(stderr, ME ": " msg "\n"); exit(1); }
#define DIE_OPT(msg) \
  { fprintf(stderr, ME ": " msg " -- %c\n" USAGE, optopt); return(1); }

  /* Parse and sanitize command line arguments */
  ctx.longDisplay = 0;
  ctx.id          = -1;
  ctx.numFound    = 0;
  while ((ch = getopt(argc, argv, ":hli:")) != -1) {
    switch (ch) {
    case 'h':
      printf(FULL_USAGE);
      return(0);

    case 'l':
      ctx.longDisplay = 1;
      break;
    case 'i':
      ctx.id = atoi(optarg);
      break;
    case ':':
      DIE_OPT("option requires an argument");
    default:
      DIE_OPT("illegal option");
    }
  }
  argc -= optind;
  argv += optind;
  if (argc > 0) {
    pattern = argv[0];
  }

  /* Die if we are not authorized to do screen recording */
  if (!isAuthorizedForScreenRecording()) {
    DIE("not authorized to do screen recording");
  }

  /* Print matching windows */
  EnumerateWindows(pattern, PrintWindow, (void *)&ctx);

  /* Return success if found any windows, or no windows but also no query */
  return((ctx.numFound > 0 || (pattern == NULL && ctx.id == -1)) ? 0 : 1);

#undef DIE_OPT
} /* lswin_main */


/* ======================================================================== */
