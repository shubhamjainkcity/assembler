#include<iostream>
#include<fstream>
#include<string.h>
using namespace std;

//////////////hash table structure////////////////
struct SymbolTable{
	string symbol;
	int address;
	SymbolTable* chain;
};

/////////////opcode table class ///////////////////////
class OpCodeTable{
	public:
	string mnemonics;
	int machine_code;
	int valid_operands;
	int instruction_size;
	void addOpcode(string mnemonics, int machine_code, int valid_operands, int instruction_size){
		this->mnemonics=mnemonics;
		this->machine_code=machine_code;
		this->valid_operands=valid_operands;
		this->instruction_size=instruction_size;
	}
	void showOpcode(){
		cout<<mnemonics<<"\t|\t"<<machine_code<<"\t|\t"<<valid_operands<<"\t|\t"<<instruction_size<<endl;
	}
};
///////////////////////////////////////////////////////////////

//////////////// register table class ///////////////////
class RegisterTable{
	public:
	char Register;
	short binary;
	void addRegister(char Register, short binary){
		this->Register=Register;
		this->binary=binary;
	}
	void showRegister(){
		cout<<Register<<"\t|\t"<<Register<<"\t|\t"<<binary<<endl;
	}
};
////////////////////////////////////////////////////////////

/////////////////hash table class/////////////////
class HashTable{
	private:
		SymbolTable* keys;
		int size;
	public:
		HashTable();
		HashTable(int);
		int computeHash(string);
		bool hasKey(string);
		void addRecord(string,int);
		int getRecord(string);
		void replaceRecord(string,int);
		void showHashTable();
		SymbolTable* getEntries();
		SymbolTable* handleCollision(SymbolTable*);
		~HashTable();
		friend class HTWrapper;
};

HashTable::HashTable(){
	size=10;
	keys=new SymbolTable[size];
}

HashTable::HashTable(int size){
	this->size=size;
	keys=new SymbolTable[size];
}

int HashTable::computeHash(string id){
	int asciiSum=0;
	for(int i=0;i<id.length();i++){
		asciiSum=asciiSum+id[i];
	}
	return (asciiSum%size);
}

bool HashTable::hasKey(string id){
	int key=computeHash(id);
	if(keys[key].symbol==id)
		return true;
	if(keys[key].chain==NULL)
		return false;
	SymbolTable* node=keys[key].chain;
	do{
		if(node->symbol==id)
			return true;
		node=node->chain;
	}while(node);	
	return false;
}

void HashTable::addRecord(string id, int value){
	if(hasKey(id)){
		cout<<"Duplicate key entries not allowed\n";
		return;
	}
	int key=computeHash(id);
	if(keys[key].symbol==""){
		keys[key].symbol=id;
		keys[key].address=value;
		return;
	}
	SymbolTable* node=handleCollision(keys+key);
	node->symbol=id;
	node->address=value;
}

int HashTable::getRecord(string id){
	int key=computeHash(id);
	if(keys[key].symbol==id)
		return keys[key].address;
	if(keys[key].chain){
		SymbolTable* node=keys[key].chain;
		do{
			if(node->symbol==id)
				return node->address;
			node=node->chain;
		}while(node);	
	}
	cout<<"Record doesn't exists\n";
	return 0;
}

void HashTable::replaceRecord(string id, int value){
	int key=computeHash(id);
	if(keys[key].symbol==id){
		keys[key].address=value;
		return;
	}
	if(keys[key].chain){
		SymbolTable* node=keys[key].chain;
		do{
			if(node->symbol==id){
				node->address=value;
				return;
			}
			node=node->chain;
		}while(node);	
	}
	cout<<"Record doesn't exists\n";
}



void HashTable::showHashTable(){
	SymbolTable *node;
	for(int i=0;i<size;i++){
		cout<<i+1<<".\t|\t"<<keys[i].symbol<<"\t|\t"<<keys[i].address<<endl;
		if(keys[i].chain==NULL) continue;
		node=keys[i].chain;
		do{			
			cout<<i+1<<".\t|\t"<<node->symbol<<"\t|\t"<<node->address<<endl;
			node=node->chain;
		}while(node);
	}
}

SymbolTable* HashTable::getEntries(){
	SymbolTable *node,*temp,*start,*p,*q;
	temp=new SymbolTable;
	start=temp;
	for(int i=0;i<size;i++){
		if(keys[i].symbol!=""){
			temp->symbol=keys[i].symbol;
			temp->address=keys[i].address;
			p=new SymbolTable;
			temp->chain=p;
			q=temp;
			temp=p;
		}
		if(keys[i].chain==NULL) continue;
		node=keys[i].chain;
		do{			
			if(keys[i].symbol!=""){
				temp->symbol=node->symbol;
				temp->address=node->address;
				p=new SymbolTable;
				temp->chain=p;
				q=temp;
				temp=p;
			}
			node=node->chain;
		}while(node);
	}
	q->chain=NULL;
	delete(temp);
	return start;
}

SymbolTable* HashTable::handleCollision(SymbolTable *rec){
	SymbolTable* node=new SymbolTable;
	node->chain=NULL;
	rec->chain=node;
	return node;
}

HashTable::~HashTable(){
	delete[] keys;
}
////////////////////////end of hashtable class////////////////////////

////////////////////////assembler class//////////////////////////////
class Assembler{
	OpCodeTable op[9];
	RegisterTable rt[5];
	HashTable H;
	public:
	int numLines;
	char label[10];
	char opcode[10];
	char operands[2][10];
	Assembler(){
		op[0].addOpcode("MOV",44,2,4);
		op[1].addOpcode("ADD",47,2,4);
		op[2].addOpcode("JUMP",67,1,2);
		op[3].addOpcode("CMP",23,2,4);
		op[4].addOpcode("INC",17,1,1);
		op[5].addOpcode("BYTE",0,2,0);
		op[6].addOpcode("WORD",0,2,0);
		op[7].addOpcode("RESB",0,1,0);
		op[8].addOpcode("RESW",0,1,0);

		rt[0].addRegister('A',0);
		rt[1].addRegister('B',1);
		rt[2].addRegister('C',10);
		rt[3].addRegister('D',11);
	}
	char **LoadFile();
	bool validateCode(char *);
	void pass1(char **instruction);
	bool isNumber(char *token){
		for(int i=0;i<strlen(token);i++)
			if(!isdigit(token[i])) return false;
		return true;
	}
	int checkOpcodeTable(char* mnemonic){
		int size=sizeof(op)/sizeof(op[0]);
		for(int i=0;i<size;i++)
			if(!strcmp(op[i].mnemonics.c_str(),mnemonic))
				return i;
		return -1;
	}
	void remove(char *word,char _char){
		int i,j;
		for(i=0,j=0;i<strlen(word);i++)
			if(word[i]!=_char)
				word[j++]=word[i];
		word[j]='\0';
	}
	bool validateOperand(char *operand){
		int size=sizeof(rt)/sizeof(rt[0]);
		string s(operand);
		if(strlen(operand)==1){
			for(int i=0;i<size;i++)
				if(rt[i].Register==operand[0])
					return true;
			return H.hasKey(s);
		}
		else if(operand[0]=='#' && isNumber(operand+1)) return true;
		else return H.hasKey(s);
	}
};

char** Assembler::LoadFile(){
	numLines=0;
	ifstream file;
	char **instruction,ch;
	int i=0,j=0;
	string unused;
	file.open("myprog2.asm",ios::in);
	while(getline(file,unused)) ++numLines;
	file.clear();
	file.seekg(0,ios::beg);
	instruction=new char*[numLines];
	instruction[i]=new char[50];
	while(file.get(ch)){
		if(ch=='\n'){
			instruction[i][j]='\0';
			j=0;
			i++;
			instruction[i]=new char[50];
			continue;
		}
		instruction[i][j]=ch;
		j++;
	}
	file.close();
	return instruction;
}


bool Assembler::validateCode(char *instruction){
	if(!strcmp(instruction,"END")) return true;
	strcpy(label,"");
	strcpy(opcode,"");
	for(int i=0;i<2;i++)
		strcpy(operands[i],"");
	char *original=new char[50];
	strcpy(original,instruction);
	char *token=strtok(instruction," ");
	if(token[strlen(token)-1]==':'){
		strcpy(label,token);
		remove(label,':');
		string o(original);
		o.replace(0,strlen(token)+1,"");
		strcpy(original,o.c_str());
		token=strtok(NULL," ");
	}
	if(checkOpcodeTable(token)!=-1){
		int index=checkOpcodeTable(token);
		strcpy(opcode,op[index].mnemonics.c_str());
		int no_op=0;
		token=strtok(NULL," ");
		while(token!=NULL){
			no_op++;
			if(!strcmp(token,",")) no_op--;
			token=strtok(NULL," ");
		}
		if(no_op!=op[index].valid_operands){
			strcpy(instruction,original);
			cout<<"Invalid # of operands for "<<strtok(instruction," ")<<"\n";
			return false;
		}
		token=strtok(original," ");
		token=strtok(NULL," ");
		int i=0;
		while(token!=NULL){
			remove(token,',');
			strcpy(operands[i++],token);
			token=strtok(NULL," ");
		}
		return true;
	}
	else{	
		if(!strcmp(token,"START")){
			strcpy(opcode,token);
			token=strtok(NULL," ");
			if(token) strcpy(operands[0],token);
			return true;
		}
		cout<<"Unknown operation "<<token<<"\n";
		return false;
	}
}

void Assembler::pass1(char **instruction){
	int line_num,loc_ctr;
	line_num=loc_ctr=0;
	int word_size=4;
	bool flag=true;
	flag=validateCode(instruction[line_num]);
	if(!strcmp(opcode,"START")){
		string op(operands[0]);
		if(op!="") loc_ctr=stoi(op);
		if(strcmp(label,"")){
			string lb(label);
			H.addRecord(lb,loc_ctr);
		}
		flag=validateCode(instruction[++line_num]);
	}
	while(strcmp(instruction[line_num],"END")){
		if(strcmp(label,"")){
			string lb(label);
			if(!H.hasKey(lb)) H.addRecord(lb,loc_ctr);
			else{
				cout<<line_num+1<<": Duplicate label declaration\n";
				flag=false;
			}
		}
		if(strcmp(opcode,"")){
			if(!strcmp(opcode,"RESB") || !strcmp(opcode,"BYTE")){
				loc_ctr++;
				string op(operands[0]);
				if(!H.hasKey(op)) H.addRecord(op,loc_ctr);
				else{
					if(H.getRecord(op) == -1)
						H.replaceRecord(op,loc_ctr);
					else{
						cout<<line_num+1<<": Duplicate label declaration\n";
						flag=false;
					}
				}
			}
			else if(!strcmp(opcode,"RESW") || !strcmp(opcode,"WORD")){
				loc_ctr+=word_size;
				string op(operands[0]);
				if(!H.hasKey(op)) H.addRecord(op,loc_ctr);
				else{
					if(H.getRecord(op) == -1)
						H.replaceRecord(op,loc_ctr);
					else{
						cout<<line_num+1<<": Duplicate label declaration\n";
						flag=false;
					}
				}
			}
			else{
				int index=checkOpcodeTable(opcode);
				loc_ctr+=op[index].instruction_size;
				for(int i=0;i<op[index].valid_operands;i++){
					if(strcmp(operands[i],"") && !validateOperand(operands[i])){
						string op(operands[i]);
						H.addRecord(op,-1);
					}
				}
			}
		}
		else{
			cout<<line_num+1<<": Opcode Not Found\n";
			flag=false;
		}
		flag=validateCode(instruction[++line_num]);
	}
	SymbolTable *symbol=H.getEntries();
	do{
		if(symbol->address==-1){
			cout<<symbol->symbol<<" was never defined in the program\n";
			flag=false;
		}
		else
			cout<<symbol->symbol<<", "<<symbol->address<<"\n";
		symbol=symbol->chain;
	}while(symbol);
	
	if(!flag)
		cout<<"\nASSEMBLER: Found some errors in your program. Cannot proceed!\n";
}
////////////////////////////////////end of assembler class///////////////////////////////////////
 
 
int main()
{
	cout<<"\n\n";
	
	Assembler a;
	char **instruction=a.LoadFile();
	a.pass1(instruction);
	cout<<"\n\n";
	return 0;
}












