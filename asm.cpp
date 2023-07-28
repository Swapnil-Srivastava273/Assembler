/*****************************************************************************
TITLE: Assembler																																
AUTHOR: Swapnil Srivastava
ROLL NUMBER: 2101AI34
Declaration of Authorship
This cpp file, asm.cpp, is part of the assignment of CS210 at the 
department of Computer Science and Engg, IIT Patna . 
*****************************************************************************/

#include <bits/stdc++.h>
using namespace std;
map<string,int>labels; // For storing all the label names and PC / SET values in the first pass.
multiset<pair<int,string>>errors; // Storing errors (if any).
void error(int line,string err){
    errors.insert({line,err});
}
struct instr{
    int opcode;
    long long operand=0;
};
bool checklabel(string s){ // Checks if a label name is valid.
    if(s.size()==0)return false;
    if(!(s[0]<='z'&&s[0]>='a')&&!(s[0]<='Z'&&s[0]>='A')&&s[0]!='_')return false;
    for(auto c:s){
        if(!(c<='z'&&c>='a')&&!(c<='Z'&&c>='A')&&c!='_'&&!(c<='9'&&c>='0'))return false;
    }
    return true;
}
int parsei(int line,string s){ // Tries to parse an operand. Successful if it is a number or a label.
    if(labels.find(s)!=labels.end())return labels[s]; 
    const char* s1=s.c_str();
    char* end;
    long n;
    if(s[0]=='0'&&s.size()>1){ // For parsing hex / oct / bin numbers
        if(s[1]=='x')n=strtol(s1+2,&end,16);
        else if(s[1]=='b')n=strtol(s1+2,&end,2);
        else n=strtol(s1,&end,8);
    }
    else n=strtol(s1,&end,10);
    if(end-s1!=(int)s.size()){ // If the entire string did not get converted to a decimal number.
        if(checklabel(s)){
            error(line,"No such label: "+s);
        }else if(s[0]<='9'&&s[0]>='0')error(line,"Invalid number format : "+s);
        else error(line,"Invalid expression.");
    }
    return n;
}

vector<instr>parsedlines; // Stores parsed instructions in numeric format. Indexed by file line. Stores opcode as -1 if no/invalid opcode is present.
map<string,pair<int,bool>>opcodes{ // Table for mnemonics in the form {name, {opcode, num of operands}}
    {"ldc",{0,1}},
    {"adc",{1,1}},
    {"ldl",{2,1}},
    {"stl",{3,1}},
    {"ldnl",{4,1}},
    {"stnl",{5,1}},
    {"add",{6,0}},
    {"sub",{7,0}},
    {"shl",{8,0}},
    {"shr",{9,0}},
    {"adj",{10,1}},
    {"a2sp",{11,0}},
    {"sp2a",{12,0}},
    {"call",{13,1}},
    {"return",{14,0}},
    {"brz",{15,1}},
    {"brlz",{16,1}},
    {"br",{17,1}},
    {"HALT",{18,0}},
    {"data",{0,1}},
    {"SET",{-1,1}}
};
map<int,string>labelSet;// Labels using SET
void makelabel(string label,int line,int linecount){ // Adds a label to the list of labels, interpreted as PC values or values decided through SET.
    if(!checklabel(label)){
        error(linecount,"Invalid label: "+label);
        return;
    }
    if(labels.find(label)==labels.end()){
        labels[label]=line;
    }else{
        error(linecount,"Duplicate label declaration: "+label);
    }
}
string toHex(int num, int s){ // Converts num into a hex string of s characters.
    stringstream ss;
    ss<<hex<<num;
    string ans(ss.str());
    ans=string(max(0,s-(int)ans.size()),'0')+ans;
    if((int)ans.size()>s)ans=ans.substr((int)ans.size()-s,s);
    return ans;
}
int main(int argc,char* argv[]){
    if(argc!=2){
        cout<<"Usage: ./asm file.asm\nWhere file.asm is the file to be compiled.\n";
        return 0;
    }
    string filename=string(argv[1]);
    ifstream input(filename);
    if(filename.find('.')!=string::npos)filename=filename.substr(0,filename.find('.'));
    string text;
    int line=0; // Program Counter (can be different from the line number of the file.)
    int linecount=0; // Line number of the file.
    vector<vector<string>>lines; // Stores each line as string vector of opcode and operands without labels and comments.
    vector<string>inputlines; // Stores lines of the file.
    // First Pass : Reads to internal form.
    while(getline(input,text)){
        linecount++;
        inputlines.push_back(text);
        istringstream ss(text);
        string word;
        vector<string>inst; // Stores tokens parsed in this line.
        string lab=""; // Label found in this line.
        int wordcount=0;
        
        while(ss>>word){
            wordcount++;
            if(word[0]==';')break;
            if(word.find(':')!=string::npos){
                if(wordcount==2&&word[0]==':'){ // If there is a space b/w label name and colon
                    lab=inst[0];
                    inst={};
                    word=word.substr(1,word.size()-1);
                }
                if(wordcount!=1)error(linecount,"Invalid label syntax."); // Label should be at the starting of the line
                else{
                    int k=word.find(':');
                    lab=word.substr(0,k);
                    word=word.substr(k+1,word.size()-k);
                }
            }
            if(word.find(';')!=string::npos){
                int k=word.find(';');
                word=word.substr(0,k); // Discarding everything after ;
                
            }
            if(word!="")inst.push_back(word);
        }
        if(lab!=""){
            if(inst.size()>=2&&inst[0]=="SET"){ // SET implementation
                makelabel(lab,parsei(line,inst[1]),linecount);
            }
            else makelabel(lab,line,linecount);
        }
        lines.push_back(inst);
        if(inst.size()&&inst[0]!="SET")line++;

    }
    ofstream listingfile(filename+".l");
    ofstream logfile(filename+".log");
    ofstream binfile(filename+".o",ios::out|ios::binary);
    // Second Pass : Checks for errors and writes to file.
    for(int i=0;i<(int)lines.size();i++){
        if(lines[i].size()==0){
            parsedlines.push_back({-1,0}); // Empty lines with comments / only labels stored as opcode -1
            continue;
        }
        if(opcodes.find(lines[i][0])==opcodes.end()){
            error(i+1,"Unknown mnemonic: "+lines[i][0]);
            parsedlines.push_back({-1,0});
        }else{
            if((int)lines[i].size()<1+opcodes[lines[i][0]].second)error(i+1,"Operand missing.");
            instr next={opcodes[lines[i][0]].first,opcodes[lines[i][0]].second?parsei(i,lines[i].size()>=2?lines[i][1]:"0"):0};
            if(lines[i][0]=="data"){ // Store data in the instruction itself. 
                next.opcode=next.operand&0xff; // First byte of data in opcode
                next.operand=next.operand>>8; // Other bytes in operand
            }
            if((int)lines[i].size()>1+opcodes[lines[i][0]].second)error(i+1,"Unexpected operand.");
            parsedlines.push_back(next);
        }
    }
    if(errors.size()){
        cout<<"Errors found. Check the log file for details.\n";
        for(auto i:errors){
            logfile<<"ERROR: Line "<<i.first<<": "<<i.second<<'\n';
        }
        return 0;
    }else{
        cout<<"Compilation successful.\n";
    }
    line=0; // Program Counter
    
    for(int i=0;i<(int)inputlines.size();i++){
        listingfile<<toHex(line,8)<<' ';
        if(parsedlines[i].opcode!=-1){
            long long opc=parsedlines[i].opcode,opr=parsedlines[i].operand;
            if(opc==13||opc==15||opc==16||opc==17)opr-=line+1; // These instructions require PC offset and not PC value
            char bin_instr[4]={(char)opc,(char)(opr&0xff),(char)((opr&0xff00)>>8),(char)((opr&0xff0000)>>16)}; // Converting the instruction to a char array of 4 bytes.
            listingfile<<toHex(opr,6)<<toHex(opc,2)<<' ';
            binfile.write(bin_instr,4);
            line++;
        }
        listingfile<<inputlines[i]<<'\n';
        
    }
    return 0;
}