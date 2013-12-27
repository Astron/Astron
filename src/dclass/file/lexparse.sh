bison --defines=Parser.h --output=Parser.cpp Parser.ypp
flex --outfile=Lexer.cpp Lexer.lpp
