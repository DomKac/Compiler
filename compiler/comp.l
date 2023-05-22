/* This tells flex to quit when it reaches EOF */
%option noyywrap
%option yylineno
%{
    /* definicje */
    #include "comp.tab.hpp"
    #include <iostream>
    #include <cstdlib>
    #include <string>
    int yylex();
%}

NUMBER  [0-9]+
ID      [_a-z]+
COMMENT \[([^\]]|\n)*\]
BLANK   [ \t\n\r]+ 

%%

"("         {return '(';}
")"         {return ')';}
","         {return ',';}
";"         {return ';';}
"+"         {return '+';}
"-"         {return '-';}
"*"         {return '*';}
"/"         {return '/';}
"%"         {return '%';}
"="         {return '=';}
">"         {return '>';}
"<"         {return '<';}
"!="        {return NEQ;}
">="        {return GEQ;}
"<="        {return LEQ;}
":="        {return ASSIGN;}

"IS"                {return IS;}
"VAR"               {return VAR;}
"PROGRAM"           {return PROGRAM;}
"PROCEDURE"         {return PROCEDURE;}
"BEGIN"             {return BEGINN;}
"END"               {return END;}
"IF"                {return IF;}
"THEN"              {return THEN;}
"ELSE"              {return ELSE;}
"ENDIF"             {return ENDIF;}
"WHILE"             {return WHILE;}
"DO"                {return DO;}
"ENDWHILE"          {return ENDWHILE;}
"REPEAT"            {return REPEAT;}
"UNTIL"             {return UNTIL;}
"READ"              {return READ;}
"WRITE"             {return WRITE;}

{COMMENT}   ;
{BLANK}     ;
{NUMBER}    {yylval = std::string(yytext); return NUM;}
{ID}        {yylval = std::string(yytext); return IDENTIFIER;}

.           {return(yytext[0]);}

%%
