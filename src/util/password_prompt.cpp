#include "password_prompt.h"

#ifdef WIN32

// Define getpass for windows
#include <iostream>
#include <string>
#include <windows.h>
using namespace std;
char *getpass(const char *prompt)
{
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));

    cout << prompt;

    string password;
    getline(cin, password);
    return password.c_str();
}


#else
// Defined in unistd.h
#include <unistd.h>
#endif
