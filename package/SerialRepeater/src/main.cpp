#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <string.h>

#include <glib-2.0/glib.h>

#include <common_defs.h>
#include <numeric.hpp>
#include <delay.hpp>
#include <log.h>
#include <cpuset.h>
#include <evloop.h>
#include <service.h>

int                 debug_mode  = 0;
static EventLoop    *main_ev    = NULL;

static void refresh_log(evutil_socket_t fd, short flags, void* args)
{
    (void) fd;
    (void) flags;
    (void) args;
    log_refresh();
}

static int stop(int err_flag)
{
    stop_Service();
    delete main_ev;
    g_message("Exit program, status=%d", err_flag);
    log_refresh();
    log_close();
    return err_flag;
}

static void run()
{
    int rc = init_Service();
    if (rc) {
        g_warning("Failed initializing service");
        return;
    }

    start_Service();
    g_message("Service initialized ---> Running\n");

    {
        main_ev = new EventLoop();
        main_ev->CreateTimerEvent(refresh_log, 2, 0, NULL);
        main_ev->Start();
    }
}

static void optargs_handler(int *argc, char ***argv)
{
    struct option opts[] = {
        {"debug", 0, 0, 'd'},
        {"version", 0, 0, 'V'},
        {"help", 0, 0, 'h'},
        {NULL, 0, 0, 0},
    };

    const char *opts_help[] = {
        "Debug Mode.",                              /* debug */
        "Print version information.",               /* version */
        "Print this message.",                      /* help */
    };

    struct option *opt;
    const char **hlp;
    int max, size;

    for(;;) {
        int i;
        i = getopt_long(*argc, *argv, "dVh", opts, NULL);
        if(i == -1) {
            break;
        }

        switch(i) {
        case 'd':
            debug_mode = 1;
            break;
        case 'V':
            fprintf(stderr, " *** SerialRepeater 1.0\n");
            exit(EXIT_SUCCESS);
        case 'h':
        default:
            fprintf(stderr, "Usage: %s [OPTIONS]\n", (*argv)[0]);
            max = 0;
            for(opt = opts; opt->name; opt++) {
                size = strlen(opt->name);
                if(size > max)
                    max = size;
            }
            for(opt = opts, hlp = opts_help; opt->name; opt++, hlp++) {
                fprintf(stderr, "  -%c, --%s", opt->val, opt->name);
                size = strlen(opt->name);
                for(; size < max; size++)
                    fprintf(stderr, " ");
                fprintf(stderr, "  %s\n", *hlp);
            }
            exit(EXIT_FAILURE);
            break;
        }
    }

    *argc -= optind;
    *argv += optind;
}

static void signals_handler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
        exit(stop(0));
        break;
    case SIGHUP:
        break;
    case SIGABRT:
    case SIGILL:
    case SIGBUS:
    case SIGSEGV:
        g_warning("Caught s system fault, call stop(ERR)");
        exit(stop(-999));
        break;
    }
}

static void set_signals()
{
    struct sigaction sig;
    sig.sa_handler = signals_handler;
    sig.sa_flags = 0;
    sigaction(SIGINT, &sig, NULL);
    sigaction(SIGQUIT, &sig, NULL);
    sigaction(SIGTERM, &sig, NULL);
    sigaction(SIGHUP, &sig, NULL);
    sigaction(SIGABRT, &sig, NULL);
    sigaction(SIGILL, &sig, NULL);
    sigaction(SIGBUS, &sig, NULL);
    sigaction(SIGSEGV, &sig, NULL);
}

int main(int argc, char *argv[])
{
//    set_process_affinity(0, 0);

    optargs_handler(&argc, &argv);
    log_open(debug_mode);
    set_signals();
    run();
    return stop(0);
}
