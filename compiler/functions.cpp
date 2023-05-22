#include <iostream>
#include <vector>
#include <algorithm> // std::find_if
#include <map>
#include <cmath>

using namespace std;

struct proc_spec
{                  
    size_t args;    // ilość argumentów procedury
    size_t fst_arg; // adres pierwszego wskaźnika  
    size_t start;   // linia w której zaczyna się procedura
    size_t back;    // adres pod którym zajduje się linia do której chcemy powrócić po wykonaniu procedury
};                  

map<string, long long> p;
map<string, proc_spec> procs;
vector<string> init_var;
vector<string> p_arg;
vector<string> id_stack;
vector<string> procs_stack;
size_t p_len = 6;  // 6 rejestrów pomocniczych
size_t k = 1;      // linia kodu maszynowego
size_t cond_state; // 0 - not NUM NUM, 1 - NUM NUM always true, 2 - NUM NUM always false
size_t args_count = 0;
bool after_proc_decl = false;
bool after_decl = false;

// ************************** OTHER **************************

string add_proc(string body) {
    // czyścimy pomocnicze wektory
    p.clear(); 
    p_arg.clear(); 
    id_stack.clear();
    init_var.clear();
    // adres pod którym będzie przechowywana linia po wykonanej procedurze
    procs[procs_stack.back()].back = p_len;
    
    p_len++; k++; 
    args_count = 0;
    after_proc_decl = false;
    after_decl = false;
    return body + "JUMPI " + to_string(p_len-1) + "\n";                                                                        
}

bool is_p_arg(string val)
{
    if (count(p_arg.begin(), p_arg.end(), val))
        return true;
    return false;
}

bool is_init(string val)
{
    if (count(init_var.begin(), init_var.end(), val))
        return true;
    return false;
}

bool is_decl(string val, size_t a) {
    for (size_t i = 0; i < size(id_stack) - a; i++) {
        if (id_stack[i] == val)
            return true;
    }
    return false;
}

int logarytm2(string num){
    long double log2_num = log2l(stold(num));
    if(log2_num == floorl(log2_num))
        return int(log2_num);
    return 0;
}

// ************************** COMMANDS **************************

string com_proc(string name)
{
    if (procs.find(name) == procs.end())
    {   // deklaracja procedury
        for (auto const &[id, adress] : p)
        {
            init_var.push_back(id);
            p_arg.push_back(id); // będąc wewnątrz procedury wiemy które zmienne są argumentami
        }
        proc_spec spec;
        spec.args = size(p);
        spec.fst_arg = p_len - spec.args;
        spec.start = k;
        procs.insert(pair<string, proc_spec>(name, spec));
        procs_stack.push_back(name);
        return "";
    }
    else
    {   // wywołanie procedury
        proc_spec x = procs[name];
        size_t id_stack_len = size(id_stack);
        string buf = "";
        string cur_id;
        if (id_stack_len == size(p)) // zadeklarownie procedury o tej samej nazwie
            return "D";        

        for (size_t i = 0; i < x.args; i++)
        {
            cur_id = id_stack[id_stack_len - x.args + i];
            if(!is_decl(cur_id, x.args))
                return to_string(id_stack_len - x.args + i);
            if (is_p_arg(cur_id))
            {
                buf += "LOAD " + to_string(p[cur_id]) + "\n";
            }
            else
            {
                buf += "SET " + to_string(p[cur_id]) + "\n";
            }
            buf += "STORE " + to_string(x.fst_arg + i) + "\n";
            k += 2;
            init_var.push_back(cur_id); // zakładamy że zmienne wprowadzane do procedury są zainicjowane
        }
        
        args_count += x.args;
        if (id_stack_len != size(p) + args_count)   // nieprawidłowa ilość argumentów
            return "E";
        
        k += 3;
        buf += "SET " + to_string(k) + "\n" +
               "STORE " + to_string(x.back) + "\n" +
               "JUMP " + to_string(x.start) + "\n";
        return buf;
    }
}

string com_if(string cond, string comm)
{
    if (cond_state == 0)         // 0 - not NUM NUM
        return cond + to_string(k) + "\n" + comm;
    else if (cond_state == 1)   // 1 - NUM NUM always true
        return comm;
    else
    { // 2 - NUM NUM always false
        size_t comm_len = count(comm.begin(), comm.end(), '\n');
        k -= comm_len; // odejmujemy od k długośc niewpisywanych komend
        return "";
    }
}

string com_if_else(string cond, string comm1, string comm2)
{
    if (cond_state == 0)
    {   // 0 - not NUM NUM
        size_t comm2_len = count(comm2.begin(), comm2.end(), '\n');;
        return cond + to_string(k - comm2_len) + "\n" + comm1 + "JUMP " + to_string(k) + "\n" + comm2;
    }
    else if (cond_state == 1)
    { // 1 - NUM NUM always true
        size_t comm2_len = count(comm2.begin(), comm2.end(), '\n');
        k -= comm2_len;
        return comm1;
    }
    else
    { // 2 - NUM NUM always false
        size_t comm1_len = count(comm1.begin(), comm1.end(), '\n');
        k -= comm1_len;
        return comm2;
    }
}

string com_while(string cond, string comm)
{
    if (cond_state == 0)
    {        // 0 - not NUM NUM
        k++; // dopisujemy jumpa
        string buf = cond + comm;
        size_t buf_len = count(buf.begin(), buf.end(), '\n') + 2; // odejmujemy dwa poniższe newline
        return cond + to_string(k) + "\n" + comm + "JUMP " + to_string(k - buf_len) + "\n";
    }
    else if (cond_state == 1)
    {        // 1 - NUM NUM always true
        k++; // dodajemy linie od JUMPa
        size_t comm_len = count(comm.begin(), comm.end(), '\n');
        return comm + "JUMP " + to_string(k - comm_len - 1) + "\n";
    }
    else
    { // 2 - NUM NUM always false
        size_t comm_len = count(comm.begin(), comm.end(), '\n');
        k -= comm_len; // odejmujemy od k długośc niewpisywanych komend
        return "";
    }
}

string com_until(string comm, string cond)
{
    if (cond_state == 0)
    { // 0 - not NUM NUM
        string buf = comm + cond;
        size_t buf_len = count(buf.begin(), buf.end(), '\n') + 1; // odejmujemy nieuzupełniony skok w cond
        return buf + to_string(k - buf_len) + "\n";
    }
    else if (cond_state == 1)
    {        // 1 - NUM NUM always true
        k++; // dodejemy linie JUMP
        size_t comm_len = count(comm.begin(), comm.end(), '\n');
        return comm + "JUMP " + to_string(k - comm_len - 1) + "\n"; // -1 poniewaz dodalismy wczeniej jumpa.
    }
    else
    { // 2 - NUM NUM always false
        return comm;
    }
}

string write_val(string val)
{
    if (isdigit(val[0]))
    {
        k += 2;
        return "SET " + val + "\n" +
               "PUT 0\n";
    }
    else
    {
        if (is_p_arg(val))
        {
            k += 2;
            return "LOADI " + to_string(p[val]) + "\n" +
                   "PUT 0\n";
        }
        k++;
        return "PUT " + to_string(p[val]) + "\n";
    }
}

string read_val(string val)
{
    if (is_p_arg(val))
    {
        k += 2;
        return "GET 0\nSTOREI " + to_string(p[val]) + "\n";
    }
    k++;
    return "GET " + to_string(p[val]) + "\n";
}

string assign_val(string val)
{
    k++;
    if (is_p_arg(val))
    {
        return "STOREI " + to_string(p[val]) + "\n";
    }
    return "STORE " + to_string(p[val]) + "\n";
}

// ************************** EXPRESIONS **************************

string set_val(string val)
{

    if (isdigit(val[0]))
    {
        k++;
        return "SET " + val + "\n";
    }
    else
    {
        k++;
        if (is_p_arg(val))
        { // v1 jest argumentem procedury
            return "LOADI " + to_string(p[val]) + "\n";
        }
        return "LOAD " + to_string(p[val]) + "\n";
    }
}

string add(string v1, string v2)
{
    if (is_p_arg(v1)) // v1 jest argumentem procedury
        v1 = "I " + to_string(p[v1]);
    else if (!isdigit(v1[0])) // v1 jest zmienną wewnętrzną maina lub procedury
        v1 = " " + to_string(p[v1]);
    // jesli v1 jest liczbą to nie zmianiamy v1

    if (is_p_arg(v2)) // v2 jest argumentem procedury
        v2 = "I " + to_string(p[v2]);
    else if (!isdigit(v2[0])) // v2 jest zmienną wewnętrzną maina lub procedury
        v2 = " " + to_string(p[v2]);
    // jesli v2 jest liczbą to nie zmianiamy v2                          

    if (isdigit(v1[0]))
    {
        if (isdigit(v2[0]))
        {
            // NUM NUM
            long long a = stoll(v1);
            long long b = stoll(v2);
            k++;
            return "SET " + to_string(a + b) + "\n";
        }
        else
        {
            // NUM ID
            k += 2;
            return "SET " + v1 + "\n" +
                   "ADD" + v2 + "\n";
        }
    }
    else
    {
        if (isdigit(v2[0]))
        {
            // ID NUM
            k += 2;
            return "SET " + v2 + "\n" +
                   "ADD" + v1 + "\n";
        }
        else
        {
            // ID ID
            k += 2;
            return "LOAD" + v1 + "\n" +
                   "ADD" + v2 + "\n";
        }
    }
}

string sub(string v1, string v2)
{
    if (is_p_arg(v1)) // v1 jest argumentem procedury
        v1 = "I " + to_string(p[v1]);
    else if (!isdigit(v1[0])) // v1 jest zmienną wewnętrzną maina lub procedury
        v1 = " " + to_string(p[v1]);
    // jesli v1 jest liczbą to nie zmianiamy v1

    if (is_p_arg(v2)) // v2 jest argumentem procedury
        v2 = "I " + to_string(p[v2]);
    else if (!isdigit(v2[0])) // v2 jest zmienną wewnętrzną maina lub procedury
        v2 = " " + to_string(p[v2]);
    // jesli v2 jest liczbą to nie zmianiamy v2
    if (isdigit(v1[0]))
    {
        if (isdigit(v2[0]))
        {
            // NUM NUM
            long long a = stoll(v1);
            long long b = stoll(v2);
            ;
            k++;
            return "SET " + to_string(a - b) + "\n";
        }
        else
        {
            // NUM ID
            k += 2;
            return "SET " + v1 + "\n" +
                   "SUB" + v2 + "\n";
        }
    }
    else
    {
        if (isdigit(v2[0]))
        {
            // ID NUM
            k += 4;
            return "SET " + v2 + "\n" +
                   "STORE 1\n" +
                   "LOAD" + v1 + "\n" +
                   "SUB 1\n";
        }
        else
        {
            // ID ID
            k += 2;
            return "LOAD" + v1 + "\n"
                    "SUB" + v2 + "\n";
        }
    }
}

string mul(string v1, string v2)
{
    if (is_p_arg(v1)) // v1 jest argumentem procedury
        v1 = "I " + to_string(p[v1]);
    else if (!isdigit(v1[0])) // v1 jest zmienną wewnętrzną maina lub procedury
        v1 = " " + to_string(p[v1]);
    // jesli v1 jest liczbą to nie zmianiamy v1

    if (is_p_arg(v2)) // v2 jest argumentem procedury
        v2 = "I " + to_string(p[v2]);
    else if (!isdigit(v2[0])) // v2 jest zmienną wewnętrzną maina lub procedury
        v2 = " " + to_string(p[v2]);
    // jesli v2 jest liczbą to nie zmianiamy v2

    string buf = "";
    if (isdigit(v1[0]))
    {
        if (isdigit(v2[0]))
        {
            // NUM NUM
            buf = "SET " + v1 + "\nSTORE 3\nSET " + v2 + "\nSTORE 4\n";
        }
        else
        {
            // NUM ID
            if (v1 == "0"){
                k++;
                return "SET 0\n";
            }
            if (v1 == "1") {
                k++;
                return "LOAD" + v2 + "\n";
            }
            if (logarytm2(v1)){
                k++;
                buf = "LOAD" + v2 + "\n";
                for (size_t i = 0; i < logarytm2(v1); i++){
                    k++;
                    buf += "ADD 0\n";
                }
                return buf;
            }

            buf = "SET " + v1 + "\nSTORE 3\nLOAD" + v2 + "\nSTORE 4\n";
        }
    }
    else
    {
        if (isdigit(v2[0]))
        {
            // ID NUM
            if (v2 == "0"){
                k++;
                return "SET 0\n";
            }
            if (v2 == "1") {
                k++;
                return "LOAD" + v1 + "\n";
            }
            if (logarytm2(v2)){
                k++;
                buf = "LOAD" + v1 + "\n";
                for (size_t i = 0; i < logarytm2(v2); i++){
                    k++;
                    buf += "ADD 0\n";
                }
                return buf;
            }
            buf = "LOAD" + v1 + "\nSTORE 3\nSET " + v2 + "\nSTORE 4\n";
        }
        else
        {
            // ID ID
            buf = "LOAD" + v1 + "\nSTORE 3\nLOAD" + v2 + "\nSTORE 4\n";
        }
    }
    k += 23;
    return "SET 0\nSTORE 2\n" +
           buf + // cond odd(b)
           "HALF\n" +
           "ADD 0\n" +
           "STORE 1\n" +
           "LOAD 4\n" +
           "SUB 1\n" +
           "JZERO " + to_string(k - 8) + "\n" +
           "LOAD 3\n" +
           "ADD 2\n" +
           "STORE 2\n" +
           "LOAD 3\n" +
           "ADD 0\n" +
           "STORE 3\n" +
           "LOAD 4\n" +
           "HALF\n" +
           "STORE 4\n" +
           "JPOS " + to_string(k - 17) + "\n" +
           "LOAD 2\n";
}

string div(string v1, string v2)
{
    if (is_p_arg(v1)) // v1 jest argumentem procedury
        v1 = "I " + to_string(p[v1]);
    else if (!isdigit(v1[0])) // v1 jest zmienną wewnętrzną maina lub procedury
        v1 = " " + to_string(p[v1]);
    // jesli v1 jest liczbą to nie zmianiamy v1

    if (is_p_arg(v2)) // v2 jest argumentem procedury
        v2 = "I " + to_string(p[v2]);
    else if (!isdigit(v2[0])) // v2 jest zmienną wewnętrzną maina lub procedury
        v2 = " " + to_string(p[v2]);
    // jesli v2 jest liczbą to nie zmianiamy v2
    string buf = "";
    if (isdigit(v1[0]))
    {
        if (isdigit(v2[0]))
        {
            // NUM NUM
            long long a = stoll(v1);
            long long b = stoll(v2);
            ;
            k++;
            return "SET " + to_string(a / b) + "\n";
        }
        else
        {
            // NUM ID
            buf = "SET " + v1 + "\nSTORE 3\nLOAD" + v2 + "\nSTORE 4\nSTORE 1\n";
        }
    }
    else
    {
        if (isdigit(v2[0]))
        {
            // ID NUM
            if (v2 == "0"){
                k++;
                return "SET 0\n";
            }
            if (v2 == "1") {
                k++;
                return "LOAD" + v1 + "\n";
            }
            if (logarytm2(v2)){
                k++;
                buf = "LOAD" + v1 + "\n";
                for (size_t i = 0; i < logarytm2(v2); i++){
                    k++;
                    buf += "HALF\n";
                }
                return buf;
            }

            buf = "LOAD" + v1 + "\nSTORE 3\nSET " + v2 + "\nSTORE 4\nSTORE 1\n";
        }
        else
        {
            // ID ID
            buf = "LOAD" + v1 + "\nSTORE 3\nLOAD" + v2 + "\nSTORE 4\nSTORE 1\n";
        }
    }
    k += 36;
    return "SET 0\nSTORE 2\nSET 1\nSTORE 5\n" +
           buf +
           "JZERO " + to_string(k) + "\n" +     
           "SUB 3\n" +                          
           "JPOS " + to_string(k - 17) + "\n" +
           "LOAD 5\n" +
           "ADD 5\n" +
           "STORE 5\n" +
           "LOAD 4\n" +
           "ADD 4\n" +
           "STORE 4\n" +
           "JUMP " + to_string(k - 26) + "\n" + 
           "LOAD 5\n" +
           "HALF\n" +
           "ADD 2\n" +
           "STORE 2\n" + 
           "SET 1\n" +
           "STORE 5\n" + 
           "LOAD 4\n" +
           "HALF\n" +
           "STORE 4\n" + 
           "LOAD 3\n" +
           "SUB 4\n" +
           "STORE 3\n" + 
           "LOAD 1\n" +
           "STORE 4\n" +
           "SUB 3\n" +
           "JZERO " + to_string(k - 24) + "\n" +
           "LOAD 2\n";
}

string mod(string v1, string v2)
{

    if (is_p_arg(v1)) // v1 jest argumentem procedury
        v1 = "I " + to_string(p[v1]);
    else if (!isdigit(v1[0])) // v1 jest zmienną wewnętrzną maina lub procedury
        v1 = " " + to_string(p[v1]);
    // jesli v1 jest liczbą to nie zmianiamy v1

    if (is_p_arg(v2)) // v2 jest argumentem procedury
        v2 = "I " + to_string(p[v2]);
    else if (!isdigit(v2[0])) // v2 jest zmienną wewnętrzną maina lub procedury
        v2 = " " + to_string(p[v2]);
    // jesli v2 jest liczbą to nie zmianiamy v2
    string buf = "";
    if (isdigit(v1[0]))
    {
        if (isdigit(v2[0]))
        {
            // NUM NUM
            long long a = stoll(v1);
            long long b = stoll(v2);
            ;
            k++;
            return "SET " + to_string(a % b) + "\n";
        }
        else
        {
            // NUM ID
            buf = "SET " + v1 + "\nSTORE 3\nLOAD" + v2 + "\nSTORE 4\nSTORE 1\n";
        }
    }
    else
    {
        if (isdigit(v2[0]))
        {
            // ID NUM
            if(v2 == "2") {
                k+=6;
                return  "LOAD" + v1 +"\n" +
                        "HALF\n" +
                        "ADD 0\n" +
                        "STORE 1\n" +
                        "LOAD" + v1 + "\n" +
                        "SUB 1\n";
            }

            buf = "LOAD" + v1 + "\nSTORE 3\nSET " + v2 + "\nSTORE 4\nSTORE 1\n";
        }
        else
        {
            // ID ID
            buf = "LOAD" + v1 + "\nSTORE 3\nLOAD" + v2 + "\nSTORE 4\nSTORE 1\n";
        }
    }
    k += 26;
    return buf +
           "JZERO " + to_string(k) + "\n" + // b=0
           "SUB 3\n" +                      // a >= b
           "JPOS " + to_string(k - 1) + "\n" +
           "LOAD 4\n" +
           "SUB 3\n" + // a >= b
           "JPOS " + to_string(k - 11) + "\n" +
           "LOAD 4\n" +
           "ADD 4\n" +
           "STORE 4\n" +
           "JUMP " + to_string(k - 17) + "\n" + // skok do
           "LOAD 4\n" +
           "HALF\n" +
           "STORE 4\n" + // ac4 = ac4/2
           "LOAD 3\n" +
           "SUB 4\n" +
           "STORE 3\n" + // ac3 = ac3 - ac4
           "LOAD 1\n" +
           "STORE 4\n" +
           "SUB 3\n" +
           "JZERO " + to_string(k - 15) + "\n" +
           "LOAD 3\n";
}

// ************************** CONDITIONS **************************
// UWAGA : w k bierzemy pod uwagę skok bez \n w condition
//         ale poczas count(cond.begin(), cond.end(), '\n'); musimy dodać +1.
string is_eq(string v1, string v2)
{
    string pom = "";
    cond_state = 0;
    if (isdigit(v1[0]))
    {
        if (isdigit(v2[0]))
        {
            // Dwie NUM
            long long a = stoll(v1);
            long long b = stoll(v2);
            ;
            if (a == b)
                cond_state = 1;
            else
                cond_state = 2;
            return "";
        }
        else
        {
            // NUM ID
            pom = sub(v1, v2) +                       // k
                  "JPOS " + to_string(k + 3) + "\n" + // k = k+2
                  sub(v2, v1) +                       // k = k+4
                  "JPOS ";
            k += 2; // dodajemy dwie linijki ze skokami
            return pom;
        }
    }
    else
    {
        if (isdigit(v2[0]))
        {
            // ID NUM
            pom = sub(v2, v1) +                       // k
                  "JPOS " + to_string(k + 3) + "\n" + // k = k+2
                  sub(v1, v2) +                       // k = k+4
                  "JPOS ";
            k += 2; // dodajemy dwie linijki ze skokami
            return pom;
        }
        else
        {
            // ID ID
            if (is_p_arg(v1)) // v1 jest argumentem procedury
                v1 = "I " + to_string(p[v1]);
            else // v1 jest zmienną wewnętrzną maina lub procedury
                v1 = " " + to_string(p[v1]);

            if (is_p_arg(v2)) // v2 jest argumentem procedury
                v2 = "I " + to_string(p[v2]);
            else // v2 jest zmienną wewnętrzną maina lub procedury
                v2 = " " + to_string(p[v2]);

            pom = "LOAD" + v1 + "\n" +
                  "SUB" + v2 + "\n" +
                  "JPOS " + to_string(k + 5) + "\n" +
                  "LOAD" + v2 + "\n" +
                  "SUB" + v1 + "\n" +
                  "JPOS ";
            k += 6;
            return pom;
        }
    }
}

string is_noteq(string v1, string v2)
{
    string pom = "";
    cond_state = 0;
    if (isdigit(v1[0]))
    {
        if (isdigit(v2[0]))
        {
            // Dwie NUM
            long long a = stoll(v1);
            long long b = stoll(v2);
            ;
            if (a != b)
                cond_state = 1;
            else
                cond_state = 2;
            return "";
        }
        else
        {
            // NUM ID
            pom = sub(v1, v2) +                       // k
                  "JPOS " + to_string(k + 3) + "\n" + // k = k+2
                  sub(v2, v1) +                       // k = k+4
                  "JZERO ";
            k += 2; // dodajemy dwie linijki ze skokami
            return pom;
        }
    }
    else
    {
        if (isdigit(v2[0]))
        {
            // ID NUM
            pom = sub(v2, v1) +                       // k
                  "JPOS " + to_string(k + 3) + "\n" + // k = k+2
                  sub(v1, v2) +                       // k = k+4
                  "JZERO ";
            k += 2; // dodajemy dwie linijki ze skokami
            return pom;
        }
        else
        {
            // ID ID
            if (is_p_arg(v1)) // v1 jest argumentem procedury
                v1 = "I " + to_string(p[v1]);
            else // v1 jest zmienną wewnętrzną maina lub procedury
                v1 = " " + to_string(p[v1]);

            if (is_p_arg(v2)) // v2 jest argumentem procedury
                v2 = "I " + to_string(p[v2]);
            else // v2 jest zmienną wewnętrzną maina lub procedury
                v2 = " " + to_string(p[v2]);

            pom = "LOAD" + v1 + "\n" +
                  "SUB" + v2 + "\n" +
                  "JPOS " + to_string(k + 5) + "\n" +
                  "LOAD" + v2 + "\n" +
                  "SUB" + v1 + "\n" +
                  "JZERO ";
            k += 6;
            return pom;
        }
    }
}

string is_geq(string v1, string v2)
{
    cond_state = 0;
    if (isdigit(v1[0]) && isdigit(v2[0]))
    {
        // Dwie NUM
        long long a = stoll(v1);
        long long b = stoll(v2);
        ;
        if (a >= b)
            cond_state = 1;
        else
            cond_state = 2;
        return "";
    }
    else
    {
        k++;
        return sub(v2, v1) +
               "JPOS ";
    }
}

string is_leq(string v1, string v2)
{
    return is_geq(v2, v1);
}

string is_greater(string v1, string v2)
{
    cond_state = 0;
    if (isdigit(v1[0]) && isdigit(v2[0]))
    {
        // Dwie NUM
        long long a = stoll(v1);
        long long b = stoll(v2);
        ;
        if (a > b)
            cond_state = 1;
        else
            cond_state = 2;
        return "";
    }
    else
    {
        k++;
        return sub(v1, v2) +
               "JZERO ";
    }
}

string is_less(string v1, string v2)
{
    return is_greater(v2, v1);
}

// ************************** ERRORS **************************
string error012(string val)
{
    if (size(p) != size(id_stack) && !after_decl && !after_proc_decl) // argument procedury nazywa się tak samo jak nowa zmienna
        return "Druga deklaracja zmiennej '" + id_stack.back() + "'!\n";
    return "";
}
string error012_decl(string val){
    if (size(p) != size(id_stack))
        return "Druga deklaracja zmiennej '" + id_stack.back() + "!'\n";
    return "";
}

string error3456(string v1, int line)
{
    if (p.find(v1) == p.end())
        return "Użycie nie zadeklarowanej zmiennej '" + v1 + "'!\n";
    if (!is_init(v1))
        cout << "Ostrzeżenie:\nUżycie nie zainicjowanej zmiennej '" + v1 + "' w linii " + to_string(line) + "!\n";
    // return "Użycie nie zainicjowanej zmiennej '" + v1 + "'!\n";
    return "";
}

string errors_proc(string val, string proc_name) {
    if(val == ""){ 
        return "Wywołanie nie zadeklarowanej procedury '" + proc_name + "'!\n";
    }
    if(val == "E") {
        return "Wywołanie procedury '" + proc_name + "' z niewłaściwą liczbą argumentów!\n";
    }
    if(isdigit(val[0])) {
        return "Wywołanie procedury '" + proc_name + "' z nie zadeklarowaną zmienną '" + id_stack[stoi(val)] + "'!\n";
    }
    return "";
}
