#include "frozenweb.h"
std::map<std::string,std::string> g_frozenWeb;
void initFrozen(){
    g_frozenWeb["/admin/index.html"] = "<img src=\"https://encrypted-tbn1.gstatic.com/images?q=tbn:ANd9GcQFQjkGGmFYQ6ZSr4eO3PN8p4UQ9xEhnON1gjScr0EOJ1EYr3N5\"/>\n\n<h1>Astron configuration</h1>\n\n<i>Powered by Puppy</i>";
}