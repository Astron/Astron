#include "frozenweb.h"

std::map<std::string, std::string> g_frozenWeb;

void initFrozen() {
    g_frozenWeb["/admin/index.html"] = "<script src=\"index.js\"></script><h1>Frozen Web</h1><h2><i>This file is served from frozenweb</i></h2>";
    g_frozenWeb["/admin/index.js"] = "alert('external frozen web js file');";
}