#ifndef FUNCTIONS
#define FUNCTIONS

#include <iostream> 
#include <vector>
#include <algorithm>    // std::find_if
#include <map>

using namespace std;

struct proc_spec
{             // Structure declaration
  size_t args;      // ilość argumentów procedury
  size_t fst_arg;   // adres pierwszego wskaźnika  
  size_t start;     // linia w której zaczyna się procedura
  size_t back;      // adres pod którym zajduje się linia do której chcemy powrócić po wykonaniu procedury
};       // Structure variable 

extern map<string, proc_spec> procs;
extern vector<string> init_var;
extern vector<string> p_arg;
extern vector<string> id_stack;
extern vector<string> procs_stack;
extern map<string, long long> p;
extern size_t p_len;
extern size_t k;
extern size_t cond_state; // 0 - not NUM NUM, 1 - NUM NUM always true, 2 - NUM NUM always false
extern size_t args_count;
extern bool after_proc_decl;
extern bool after_decl;

// commands
string com_proc(string name);
string com_while(string cond, string comm);
string com_until(string comm, string cond);
string com_if(string cond, string comm);
string com_if_else(string cond, string comm1, string comm2);
string read_val(string val);
string write_val(string val);
string assign_val(string val);

// expressions
string set_val(string val);
string add(string v1, string v2);
string sub(string v1, string v2);
string mul(string v1, string v2);
string div(string v1, string v2);
string mod(string v1, string v2);

// conditions
string is_eq     (string v1, string v2);
string is_noteq  (string v1, string v2);
string is_geq    (string v1, string v2);
string is_leq    (string v1, string v2);
string is_greater(string v1, string v2);
string is_less   (string v1, string v2);

// errors
string error012(string val);
string error3456(string v1, int line);
string errors_proc(string val, string proc_name);
string error012_decl(string val);

// other
string add_proc(string body);
bool is_decl(string val, size_t a);
bool is_p_arg(string val);
bool is_init(string val);

#endif
