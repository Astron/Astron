#include "password_prompt.h"
using namespace std;

#ifdef _WIN32

// Define getpass for windows
#include <iostream>
#include <string>
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
string password_prompt(const string& prompt)
{
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));

    cout << prompt;

    string password;
    getline(cin, password);
    return password;
}


#else
// Defined in unistd.h
#include <stdlib.h>
#include <unistd.h>
string password_prompt(const string& prompt)
{
    string password;
    char *raw = getpass(prompt.c_str());
    password = raw;
    free(raw);

    return password;
}
#endif
