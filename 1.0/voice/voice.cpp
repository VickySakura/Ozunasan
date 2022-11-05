/*
	Author:...
*/
/*--------头文件--------*/
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <conio.h>
#include <cmath>
#include <cstring>
#include <windows.h>
using namespace std;

/*--------一些常量--------*/
// 圆周率 
const float PI  =3.1415926535897932384626433832795f;
// 两个绝对音高的相差倍数，即2的十二分之一次方 
const float STEP=1.0594630943592952645618252949463f;
// 一个音符的最大内存占用 
const unsigned int MAX_BUFFER = 1048576;
// 采样率
const unsigned int SampleRate = 32000  ;
// 声道数
const unsigned short AudioChannel = 1  ;
// 采样位数
const unsigned short BitPerSample = 16 ; 

/*--------全局变量--------*/
CONSOLE_SCREEN_BUFFER_INFO csbi;
// 用于输出的句柄
HANDLE hOut;

// 音符(采样缓冲区)
vector<float> buffer;
// 缓冲区备份 
vector<float> backup;
// 剪贴板
vector<float> cutting;
// 音轨
vector<float> track;

// 光标位置
COORD cur;

// 3个for循环用计数器
int i,j,k;
// 1个通用计数器
signed long long int counter;

// 输入缓冲区
string word;
string wbackup;
char str[256];
char cmd[384]=".\\packager.exe ";
float fdat;
int dat;

// 文件IO
FILE *fp; 

// --模式--
// 区域编辑(true)/单个编辑(false)
bool EditMode=false;
// 实践编辑(true)/单个编辑(false) 
bool TimeMode=false;

/*--------函数声明(部分已定义)--------*/
// 保存光标位置
void SaveCur(void);
// 设置光标位置
void SetCur(void);

// push命令
void push_(int a){
	for(i=0;i<a;i++){
		track.push_back(buffer[i]);
	}
}
// mode命令
void mode_(string type,string value){
	if(type=="-e"){
		if(value=="true")EditMode=true;
		if(value=="false")EditMode=false; 
	}
	if(type=="-t"){
		if(value=="true")TimeMode=true;
		if(value=="false")TimeMode=false; 
	}
}
// fill命令
void fill_(int a){
	float *p=&buffer[a];
	for(i=a;i<MAX_BUFFER;i++){
		*p=buffer[i-a];
		p++;
	}
}
// catch命令
void catch_(string match){
	match.copy(str,match.length());
	fp=fopen(str,"rb");
	if(fp!=NULL){
		signed short tmp;
		i=0;
		while(fread(&tmp,2,1,fp)!=0&&i<MAX_BUFFER){
			buffer[i]=(float)tmp;
			buffer[i]/=32768.0;
			i++;
		}
		fclose(fp);
	}else{
		cout<<"Fatal:No file called "<<match<<endl;
	}
	if(i<MAX_BUFFER-1){
		fill_(i);
	}
}
void catch__(string match){
	match.copy(str,match.length());
	fp=fopen(str,"rb");
	if(fp!=NULL){
		signed short tmp;
		track.clear();
		float tmp1;
		while(fread(&tmp,2,1,fp)!=0){
			tmp1=(float)tmp;
			tmp1/=32768.0;
			track.push_back(tmp1);
		}
		fclose(fp);
	}else{
		cout<<"Fatal:No file called "<<match<<endl;
	}
}
// make命令
void make_(string match){
	match.copy(str,match.length());
	fp=fopen(str,"wb");
	if(fp!=NULL){
		short tmp;
		i=0;
		while(i<track.size()){
			if(track[i]<=-1.0){
				tmp=-32768;
			}else if(track[i]>=1.0){
				tmp=32767;
			}else{
				tmp=(short)(track[i]*32768.0f);
			}
			fwrite(&tmp,2,1,fp);
			i++;
		}
	}
}
// edit命令(重载三遍) 
void edit_(int a,float get){
	buffer[a]=get;
}
void edit_(int st,int ed,float get){
	for(;st<ed;st++){
		buffer[st]=get;
	}
}
void edit_(void){
	for(i=0;i<MAX_BUFFER;i++){
		buffer[i]=0.0f;
	}
}
// erase命令 
void erase_(int a){
	buffer.erase(buffer.begin()+a);
	buffer.push_back(0.0f);
}
void erase_(int st,int ed){
	i=ed-st;
	buffer.erase(buffer.begin()+st,buffer.begin()+ed);
	for(;buffer.size()<MAX_BUFFER-1;){
		buffer.push_back(0.0f);
	}
}
void erase_(void){
	vector<float>().swap(buffer);
	for(;buffer.size()<MAX_BUFFER-1;){
		buffer.push_back(0.0f);
	}
}
void erase__(int a){
	track.erase(track.begin()+a);
}
void erase__(void){
	vector<float>().swap(track);
}
void erase__(int st,int ed){
	i=ed-st;
	track.erase(track.begin()+st,track.begin()+ed);
}
// pop命令
void pop_(int a){
	for(;a>0;a--){
		track.pop_back();
	}
}
// 遍历命令
void show_(int a){
	for(i=a;i<a+SampleRate/1000;i++){
		cout<<i<<' '<<buffer[i]<<endl;
	}
	cout<<endl;
}
void show__(int a){
	for(i=a;i<a+SampleRate/1000;i++){
		cout<<i<<' '<<track[i]<<endl;
	}
	cout<<endl;
}
// remix命令
void remix_(int a,int b){
	track[b]+=buffer[a];
}
void remix_(int st,int ed,int a){
	float *p1=&buffer[st];
	float *p2=&track[a];
	for(i=st;i<ed;i++){
		*p2+=*p1;
		p1++;
		p2++;
	}
}
void vol_(int a,float f){
	buffer[a]*=f;
}
void vol_(int st,int ed,float f){
	for(i=st;i<ed;i++){
		buffer[i]*=f;
	}
}
void vol__(int st,int ed,float f){
	for(i=st;i<ed;i++){
		track[i]*=f;
	}
}

/* *------* 主函数 *------* */
int main(int count,char **Strings){
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	for(i=0;i<MAX_BUFFER;i++){
		buffer.push_back(0);
	}
	do{
		i=0;j=0;k=0;
		word.clear();
		cout<<endl;
		cout<<track.size()<<"Samples";
		cout<<"--|";
		if(EditMode)cout<<"AREA";else cout<<"SPLE";
		cout<<"|--"<<endl;
		cout<<"$>";
		cin>>word;
		if(word=="exit"){
			break;
		}else
		if(word=="catch"||word=="load"){
			word.clear();
			cin>>word;
			catch_(word);
		}else
		if(word=="mode"){
			word.clear();
			cin>>word;
			cin>>wbackup;
			mode_(word,wbackup);
			wbackup.clear();
		}else
		if(word=="edit"||word=="set"){
			if(EditMode){
				cin>>i>>j>>fdat;
				edit_(i,j,fdat);
			}else{
				cin>>dat>>fdat;
				edit_(dat,fdat);
			}
		}else
		if(word=="erase"||word=="del"){
			if(EditMode){
				cin>>j;
				word.copy(str,word.length());
				i=atoi(str);
				erase_(i,j);
			}else{
				word.copy(str,word.length());
				dat=atoi(str);
				erase_(dat);
			}
		}else
		if(word=="push"){
			cin>>dat;
			push_(dat);
		}else
		if(word=="fill"){
			cin>>dat;
			fill_(dat);
		}else
		if(word=="make"){
			cin>>word;
			make_(word);
			cout<<"Save to WAV music?[Yes/...]";
			cin>>word;
			if(word=="Yes"){
				strcat(cmd,str);
				system(cmd);
			}
		}else
		if(word=="show"){
			cin>>dat;
			show_(dat);
		}else
		if(word=="show$"){
			cin>>dat;
			show__(dat);
		}else
		if(word=="erase$"||word=="del$"){
			if(EditMode){
				cin>>i>>j;
				erase__(i,j);
			}else{
				cin>>dat;
				erase__(dat);
			}
		}else
		if(word=="clear"){
			erase_();
		}else
		if(word=="clear$"){
			erase__();
		}else
		if(word=="pop"){
			cin>>dat;
			pop_(dat);
		}else
		if(word=="remix"){
			if(EditMode){
				cin>>i>>j>>k;
				remix_(i,j,k);
			}else{
				cin>>i,j;
				remix_(i,j);
			}
		}else
		if(word=="catch$"){
			cin>>word;
			catch__(word);
		}else
		if(word=="vol"){
			if(EditMode){
				cin>>i>>j>>fdat;
				vol_(i,j,fdat);
			}else{
				cin>>i>>fdat;
				vol_(i,dat);
			}
		}else
		if(word=="vol$"){
			cin>>i>>j>>fdat;
			vol__(i,j,fdat);
		}
		
		
		FlushConsoleInputBuffer(hOut);
		for(i=0;i<255;i++){
			str[i]=0;
		}
	}while(1);
	
	cout<<"Exiting...";
	vector<float>().swap(buffer);
	printf("--");
	vector<float>().swap(track);
	printf("--");
	if(fp!=NULL)fclose(fp);
	printf("--");
	CloseHandle(hOut);
	cout<<"--";
	putchar('\n');
	return 0;
}

/*--------函数定义--------*/
// 保存光标位置
void SaveCur(void){
	GetConsoleScreenBufferInfo(hOut,&csbi);
	cur=csbi.dwCursorPosition;
}
// 设置光标位置
void SetCur(void){
	SetConsoleCursorPosition(hOut,cur);
}
