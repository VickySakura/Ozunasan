#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>

const unsigned char
HEAD[40]={
'R','I','F','F',  // ChunkID
0  ,0  ,0  ,0  ,  // ChunkSize
'W','A','V','E',  // Format
'f','m','t',' ',  // Subchunk1ID
16 ,0  ,0  ,0  ,  // Subchunk1Size
1  ,0  ,          // AudioFormat
1  ,0  ,          // NumChannels
0  ,0x7D,0 ,0  ,  // SampleRate
0  ,0xFA,0 ,0  ,  // ByteRate
2  ,0  ,          // BlockAlign
16 ,0  ,          // BitPerSample
'd','a','t','a'   // Subchunk2ID
};

static unsigned long long int fSize;

char *fAddr;

FILE *in,*out;

COORD M;

int tmp;

int main(int iCounter,char **iGet){
	
	if(iCounter==2){
		iGet++;
		fAddr = *iGet;
	}else if(iCounter==1){
		fAddr = (char*)calloc(4096,1);
		puts("Please enter the PCM file\'s address");
		gets(fAddr);
	}else{
		exit(1);
	}
	
	system("CLS");
	printf("Please wait a moment...\n");
	
	in = fopen(fAddr,"rb");
	if(in == NULL){
		printf("Fatal : No file called %s.\n",fAddr);
		Sleep(500);
		exit(2);
	}
	printf("Open file[%s]succeed.\n",fAddr);
	
	strcat(fAddr,".WAV");
	out = fopen(fAddr,"wb");
	if(out==NULL){
		printf("Fatal : Create File .\\OUT.WAV Failed.\n");
		Sleep(500);
		exit(3);
	}
	printf("Create File[.\\OUT.WAV]succeed.\n");
	
	fseek(in, 0, SEEK_END);
	fSize = ftell(in);
	
	fseek(in, 0, SEEK_SET);
	
	unsigned char c;
	fwrite(&HEAD[0],1,4,out);
	
	tmp=fSize+36;
	fwrite(&tmp,4,1,out);
	
	fwrite(&HEAD[8],1,32,out);
	
	fwrite(&fSize,4,1,out);
	c=0;
	puts("Writting Head\n"); 
	
	while((fread(&c,1,1,in))!=0){
		fwrite(&c,1,1,out);
		printf("Writting...%X/%X\n",
			ftell(in),
			fSize
		);
	}
	fclose(in);
	fclose(out);
	free(fAddr);
	return 0;
}
