#pragma once

#ifdef _WIN32
    #include <Windows.h>
    #include <io.h>
    #include <stdio.h>
    #include <stdlib.h>
    // The POSIX-compliant access(2) API is deprecated on Windows
    // We use the ISO C++-conformant function _access in this case.
    #define access _access
#else
    #include <limits.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace fs
{ 
    inline bool file_exists(const std::string& path)
    {
        #ifdef _WIN32
            DWORD attr = GetFileAttributes((LPCTSTR)path.c_str());
            return attr != INVALID_FILE_ATTRIBUTES && 
                        !(attr & FILE_ATTRIBUTE_DIRECTORY);
        #else
            struct stat path_stat;
            stat(path.c_str(), &path_stat);
            return S_ISREG(path_stat.st_mode);
        #endif
    }

    inline bool dir_exists(const std::string& path)
    {
        #ifdef _WIN32
            DWORD attr = GetFileAttributes((LPCTSTR)path.c_str());
            return attr != INVALID_FILE_ATTRIBUTES &&
                   (attr & FILE_ATTRIBUTE_DIRECTORY);
        #else
            struct stat path_stat;
            stat(path.c_str(), &path_stat);
            return S_ISDIR(path_stat.st_mode);
        #endif
    }

    inline bool is_readable(const std::string& path)
    {
        return !access(path.c_str(), R_OK);
    }

    inline std::string parent_of(const std::string& path)
    {
        if(path.empty())
            return path;

        std::size_t parent_ends = path.find_last_of("/\\");
        if(parent_ends == std::string::npos)
            return "";

        return path.substr(0, parent_ends);
    }

    inline std::string filename(const std::string& path)
    {
        if(path.empty())
            return path;

        // We need +1 regardless:
        // If the result is std::string::npos (-1) then the substr will start from 0 (file path start)
        // If the result isn't std::string::npos we need to skip the next character (index of last slash)
        std::size_t parent_ends = path.find_last_of("/\\") + 1;
        return path.substr(parent_ends, path.length());
    }

    inline bool current_path(const std::string& path)
    {
        if(!dir_exists(path))
            return false;

        #ifdef _WIN32
            return SetCurrentDirectory((LPCTSTR)path.c_str());
        #else
            return !chdir(path.c_str());
        #endif
    }

    inline std::string current_path()
    {
        #ifdef _WIN32
            TCHAR path[MAX_PATH];
            if(!GetCurrentDirectory(MAX_PATH, path))
                return "";

            path[MAX_PATH - 1] = '\0';
            return std::string(path);
        #else
            char path[PATH_MAX];
            if(!getcwd(path, PATH_MAX - 1))
                return "";

            path[PATH_MAX - 1] = '\0';
            return std::string(path);
        #endif
    }

};
