#include<iostream>
#include<fstream>
#include<cstring>
#include<bits/stdc++.h>
#include<stdio.h>
using namespace std;


//////////////symbol table structure////////////////
struct SymbolTable{
	string symbol;
	int address;
	SymbolTable* chain;
};
//////////////////////////////////////////////////////

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

SymbolTable* HashTable::handleCollision(SymbolTable *rec){
	SymbolTable* node=new SymbolTable;
	node->chain=NULL;
	rec->chain=node;
	return node;
}

HashTable::~HashTable(){
	delete[] keys;
}
/////////////////////end of hashtable class//////////////////////////////////////////////

/////////////////////HashTable wrapper class///////////////////////////////////////////////
#include<bits/stdc++.h>
class HTWrapper{
	HashTable *H;
	string str;
	public:
	HTWrapper(HashTable*);
	void serialize();
	void saveToFile();
	void loadFromFile();
};

HTWrapper::HTWrapper(HashTable *H){
	this->H=H;
}

void HTWrapper::serialize(){
	SymbolTable *node;
	for(int i=0;i<H->size;i++){
		if(H->keys[i].symbol!="")
			str.append(H->keys[i].symbol+"\t"+to_string(H->keys[i].address)+"\n");
		if(H->keys[i].chain==NULL) continue;
		node=H->keys[i].chain;
		do{
			str.append(node->symbol+"\t"+to_string(node->address)+"\n");
			node=node->chain;
		}while(node);
	}
}
void HTWrapper::saveToFile(){
	serialize();
	ofstream file;
	file.open("Input.txt",ios::out);
	file<<str;
	file.close();
}

void HTWrapper::loadFromFile(){
	ifstream file;
	file.open("Input.txt",ios::in);
	char ch;
	while(file.get(ch)){
		str.append(1,ch);
	}
	stringstream s(str);
	string symbol, address;
	while(s>>symbol){
		s>>address;
		H->addRecord(symbol,stoi(address));
	}
	file.close();
}
//////////////////////end of HTWrapper class///////////////////////////////////

///////////////////// assembler class //////////////////////////////////////////
class Assembler{
	OpCodeTable op[4];
	RegisterTable rt[4];
	HashTable H;
	public:
	Assembler(){
		op[0].addOpcode("MOV",44,2,4);
		op[1].addOpcode("ADD",47,2,4);
		op[2].addOpcode("JUMP",67,1,2);
		op[3].addOpcode("CMP",23,2,4);

		rt[0].addRegister('A',0);
		rt[1].addRegister('B',1);
		rt[2].addRegister('C',10);
		rt[3].addRegister('D',11);
	}
	char **LoadFile();
	int validateCode(char **);
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
	int numLines=0;
	ifstream file;
	char **instruction,ch;
	int i=0,j=0;
	string unused;
	file.open("myprog1.asm",ios::in);
	if(!file){
		cout<<"File doesn't exist\n";
		exit(0);
	}
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
	return instruction;
}


int Assembler::validateCode(char **instruction){
	int i,index,no_op;
	char *original=new char[50];
	char *token=strtok(instruction[0]," ");
	if(strcmp(token,"START"))
		cout<<"Line "<<"1:"<<"Code was not starting with START(or case is not matching)\n";

	else{
		token=strtok(NULL," ");
		if(token!=NULL && !isNumber(token)){
			cout<<"Line "<<"1:'"<<token<<"' is not an integer\n";
		}
		token=strtok(NULL," ");
		while(token!=NULL){
			cout<<"Line "<<"1:"<<"Irrelevant symbol \""<<token<<"\"\n";
			token=strtok(NULL," ");
		}
	}

	for(i=0;i<sizeof(instruction);i++){
		strcpy(original,instruction[i]);
		token=strtok(instruction[i]," ");
		if(!strcmp(token,"START")){
			if(i==0) continue;
			else{
				cout<<"Line "<<i+1<<":START is written at the beginning of code\n";
				continue;
			}
		}
		if(!strcmp(token,"END")){
			if(i==sizeof(instruction)-1) continue;
			else{
				cout<<"Line "<<i+1<<":END is written at the end of code\n";
				continue;
			}
		}
		if(!strcmp(token,"BYTE")){
			string s,a;
			token=strtok(NULL," ");
			if(!isNumber(token)) s=token;
			else cout<<"Line "<<i+1<<":Non-numerical indentifier was expected\n";
			token=strtok(NULL," ");
			if(token[0]=='#' && isNumber(token+1)) a=token+1;
			else cout<<"Line "<<i+1<<":#Number was expected\n";
			if(s!="" && a!="") H.addRecord(s,stoi(a)); continue;
		}
		if(token[strlen(token)-1]==':'){
			string o(original);
			o.replace(0,strlen(token),"");
			remove(token,':');
			string s(token);
			H.addRecord(s,i);
			strcpy(original,o.c_str());
			token=strtok(NULL," ");
		}
		if(checkOpcodeTable(token)!=-1){
			index=checkOpcodeTable(token);
			no_op=0;
			token=strtok(NULL," ");
			while(token!=NULL){
				no_op++;
				token=strtok(NULL," ");
			}
			if(no_op!=op[index].valid_operands){
				cout<<"Line "<<i+1<<":Invalid # of operands for "<<strtok(instruction[i]," ")<<"\n";
				continue;
			}
			token=strtok(original," ");
			token=strtok(NULL," ");
			while(token!=NULL){
				remove(token,',');
				if(!validateOperand(token)) cout<<"Line "<<i+1<<":"<<token<<" was never declared in the program\n";
				token=strtok(NULL," ");
			}
		}
		else{
			cout<<"Line "<<i+1<<":Unknown operation "<<token<<"\n";
		}
	}

	token=strtok(instruction[i-1]," ");
	if(strcmp(token,"END"))
		cout<<"Line "<<i<<":Code was not ending with END(or case is not matching)\n";
	else{
		token=strtok(NULL," ");
		while(token!=NULL){
			cout<<"Line "<<i<<":Irrelevant symbol \""<<token<<"\"\n";
			token=strtok(NULL," ");
		}
	}
}
////////////////////////////end of assembler class//////////////////////////////////////////////////


int main()
{
	cout<<"\n\n";

	Assembler a;
	char **instruction=a.LoadFile();
	a.validateCode(instruction);

	cout<<"\n\n";
	return 0;
}












