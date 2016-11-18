#include "shutdown.h"
#include <stdio.h>
#include <iostream>
#include <mutex>
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#else
#  include <signal.h>
#  include <stdlib.h>
#  include <unistd.h>
#endif
using namespace std;

#include "global.h"


const static int MAX_INTERRUPTS = 3;

static int exit_code = 0;
static int interrupts = 0;
static mutex exit_mtx;
static mutex ctrlc_mtx;


#ifdef _WIN32 /* Handle Windows signals */
static BOOL handle_interrupt(DWORD)
{
    interrupts += 1;
    if(interrupts >= MAX_INTERRUPTS) {
        ctrlc_mtx.lock();
        /*log->fatal()*/
        cerr << "Received " << MAX_INTERRUPTS << " interrupts; exiting immediately!\n";
        exit(1);
    }

    /*log->info()*/
    cerr << "Received interrupt (Ctrl + C)\n";
    astron_shutdown(0, false);
    return true;
}
void astron_handle_signals()
{
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)handle_interrupt, true);
}

#else /* Handle POSIX signals */
static void handle_interrupt(int)
{
    interrupts += 1;
    if(interrupts >= MAX_INTERRUPTS) {
        ctrlc_mtx.lock();
        /*log->fatal()*/
        cerr << "Received " << MAX_INTERRUPTS << " interrupts; exiting immediately!\n";
        exit(1);
    }

    /*log->info()*/
    cerr << "Received interrupt (Ctrl + C)\n";
    astron_shutdown(0, false);
}
void astron_handle_signals()
{
    struct sigaction interruptHandler;
    interruptHandler.sa_handler = handle_interrupt;
    sigemptyset(&interruptHandler.sa_mask);
    interruptHandler.sa_flags = 0;
    sigaction(SIGINT, &interruptHandler, nullptr);
}

#endif


// astron_shutdown tells astron to exit gracefully with a given error code
void astron_shutdown(int code, bool throw_exception)
{
    exit_mtx.lock();
    /*log->info()*/
    cerr << "Exiting...\n";
    exit_code = code;
    io_service.stop();
    if(throw_exception) {
        throw ShutdownException(code);
    }
}

// astron_exit_code returns the exit code astron should exit with
int astron_exit_code()
{
    return exit_code;
}


ShutdownException::ShutdownException(int exit_code) : m_exit_code(exit_code)
{
}

int ShutdownException::exit_code() const
{
    return m_exit_code;
}

const char* ShutdownException::what() const throw()
{
    return "Astron is shutting down.";
}
