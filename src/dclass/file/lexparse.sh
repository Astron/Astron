bison --defines=parser.h --output=parser.cpp parser.ypp
flex --outfile=lexer.cpp lexer.lpp
