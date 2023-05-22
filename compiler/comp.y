%{
    #include <iostream>
    #include <string>
    #include <stdio.h>
    #include <cstdlib>
    #include <vector>
    #include <map>
    #include <fstream>
    #include "../functions.hpp"

    using namespace std;

    extern int yylex (void);
    extern int yyparse();
    extern int yylineno;
    extern FILE* yyin;

    void yyerror(string s);
    #define YYDEBUG 1
    string last_proc = "";
    string program = "";
%}

%code requires {
    #include <string>
}
/* Tell bison to give descriptive error messages. */
%define parse.error verbose
%define api.value.type {std::string}
%locations

%token NUM
%token IDENTIFIER
%token ASSIGN
%token NEQ LEQ GEQ
%token IS VAR PROGRAM PROCEDURE BEGINN END IF THEN ELSE ENDIF WHILE DO ENDWHILE REPEAT UNTIL READ WRITE

%%
program_all:
    procedures main {size_t proc_len = count($1.begin(), $1.end(), '\n');
                    program = "JUMP " + to_string(proc_len+1) + "\n" +  $1 + $2 + "HALT\n";}

procedures:
    procedures PROCEDURE proc_head IS VAR declarations BEGINN commands END  {$$ += add_proc($8);}
|   procedures PROCEDURE proc_head IS BEGINN commands END                   {$$ += add_proc($6);}
|   %empty  

main:
    PROGRAM IS VAR declarations BEGINN commands END {$$ = $6;}
|   PROGRAM IS BEGINN commands END                  {$$ = $4;}

commands:
    commands command    {$$ += $2;}
|   command             {$$ = $1;}

command:
    IDENTIFIER ASSIGN expression ';'        {init_var.push_back($1);if(error3456($1,yylineno) != ""){yyerror(error3456($1,yylineno)); YYERROR;} $$ = $3 + assign_val($1); }
|   IF condition THEN commands              {if(cond_state==0){k++;}} 
    ELSE commands ENDIF                     {$$ = com_if_else($2, $4, $7);}
|   IF condition THEN commands ENDIF        {$$ = com_if($2, $4);}
|   WHILE condition DO commands ENDWHILE    {$$ = com_while($2, $4);}
|   REPEAT commands UNTIL condition ';'     {$$ = com_until($2,$4);}
|   proc_head ';'                           {if(errors_proc($1, last_proc)!=""){yyerror(errors_proc($1, last_proc)); YYERROR;} $$ = $1;}
|   READ IDENTIFIER ';'                     {init_var.push_back($2); if(error3456($2,yylineno) != ""){yyerror(error3456($2,yylineno)); YYERROR;} $$ = read_val($2); }
|   WRITE value ';'                         {$$ = write_val($2);}

proc_head:
    IDENTIFIER '(' proc_declarations ')' {$$ = com_proc($1); last_proc = $1; after_proc_decl = true;
                                          if($$ == "D"){yyerror("Procedura o nazwie '" + last_proc + "' już istnieje!\n"); YYERROR;}}

proc_declarations:
    proc_declarations ',' IDENTIFIER    {p.insert(pair<string,int>($3,p_len)); p_len++; id_stack.push_back($3);
                                        if(error012($3)!="") {yyerror(error012($3)); YYERROR;}}
|   IDENTIFIER                          {p.insert(pair<string,int>($1,p_len)); p_len++; id_stack.push_back($1);
                                        if(error012($1)!="") {yyerror(error012($1)); YYERROR;}}

declarations:
    declarations ',' IDENTIFIER     {p.insert(pair<string,int>($3,p_len)); p_len++; id_stack.push_back($3);
                                    if(error012_decl($3)!="") {yyerror(error012_decl($3)); YYERROR;}}
|   IDENTIFIER                      {p.insert(pair<string,int>($1,p_len)); p_len++; id_stack.push_back($1);
                                    if(error012_decl($1)!="") {yyerror(error012_decl($1)); YYERROR;} 
                                    after_decl = true;}

expression:
    value               {$$ = set_val($1);}
|   value '+' value     {$$ = add($1, $3);}
|   value '-' value     {$$ = sub($1, $3);}
|   value '*' value     {$$ = mul($1, $3);}
|   value '/' value     {$$ = div($1, $3);}
|   value '%' value     {$$ = mod($1, $3);}

condition:
    value '=' value     {$$ = is_eq($1,$3);}
|   value '>' value     {$$ = is_greater($1,$3);}
|   value '<' value     {$$ = is_less($1,$3);}
|   value GEQ value     {$$ = is_geq($1,$3);}
|   value LEQ value     {$$ = is_leq($1,$3);}
|   value NEQ value     {$$ = is_noteq($1,$3);}

value:
    NUM         {}
|   IDENTIFIER  {if(error3456($1,yylineno) != ""){yyerror(error3456($1,yylineno)); YYERROR;}}
%%

void yyerror(std::string s){
  cout << "Błąd w linii: " << yylineno << endl << s<<endl;
}

int main(int argc, char const * argv[]){
  
    if(argc < 3){
        cout << "Za mała liczba argumentów!" << endl;
        return -1;
    }
    FILE *data;
    data = fopen( argv[1], "r" );
    if( !data ){
        cout << "Nie ma takiego pliku!" << endl; 
        return -1;
    }
    cout << endl;
    yyin = data;
    yyparse();
    ofstream out(argv[2]);
    out << program;
    fclose(data);
    return 0;
}
