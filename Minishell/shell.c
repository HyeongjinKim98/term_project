#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#define MAX_SIZE  24
#define MAX_CMDSC 14
#define DELIMS " \n"
char* home_dir;
int pid;
char* path = NULL;
char* cmd_stack[50] ;
int stack_index=0;

int _cmds_num(char* input);
bool out = false;
bool cmd_ls(char* cwd, int argc);
bool cmd_pwd(int argc);
bool cmd_cd(char* argv[], int argc);
bool cmd_mkdir(char* argv[], int argc);
bool cmd_rm(char* argv[], int argc);
bool cmd_cat(char* argv[], int argc);
bool cmd_chmod(char* argv[], int argc);
bool cmd_stat(char* argv[], int argc);
bool cmd_ps(char* argv[], int argc);
bool cmd_cp(char* argv[], int argc);
bool cmd_ln(char* argv[], int argc);
bool cmd_alias(char* argv[], int argc);
void* run_alias(char* argv[], int argc);
char* history_num(char* argv[], int argc);
void cmd_history();
void read_txt();
char* current_dir();

char* _cmds[MAX_CMDSC] = {"cat","pwd","cd","mkdir","ls","history","ps","cp","rm","ln","stat","chmod","alias","!"};      

int redirect(char* cmd){
	int i;
	for(i=0;i<MAX_CMDSC;i++){
		if(strcmp(cmd,_cmds[i])){
			return i;
			break;
		}
	}
}

void eliminate(char *line, char c){
	for(; *line!= '\0'; line++){
		if(*line==c){
			strcpy(line,line+1);
			line--;
		}
	}
}
void run(char* input){
	
	cmd_stack[stack_index] =  input;
	stack_index++;
	int argc = 0; //count
	char* argv[5] = {NULL,};
	char *tmp;
	strcat(input," ");

	if(strstr(input,"!")!=NULL){
		tmp = strtok(input,DELIMS);
		tmp = strtok(input,"!");
		argv[0] = "!";
		argv[1] = tmp;
		if(argv[1]==NULL)argc--;
	}
	else{
		tmp = strtok(input,DELIMS);
		while(tmp!=NULL) {
			argv[argc]=tmp;
			argc++;
			tmp = strtok(NULL,DELIMS);
		}
	}
	  
	int direction = _cmds_num(argv[0]);
	switch(direction){
		case 0 :
			cmd_cat(argv,argc);
			break;
		case 1 :
			cmd_pwd(argc);
			break;
		case 2 :
			cmd_cd(argv, argc);
			break;
		case  3 :
			cmd_mkdir(argv,argc);
			break;
		case 4 :
			cmd_ls(current_dir(),argc);
			break;
		case 5 :
			read_txt(argv[0]);
			break;
		case 6 :
			cmd_ps(argv,argc);
			break;
		case 7 :
			cmd_cp(argv,argc);
			break;
		case 8 :
			cmd_rm(argv,argc);
			break;
		case 9 :
			cmd_ln(argv,argc);
			break;
		case 10 :
			cmd_stat(argv,argc);
			break;
		case 11 : 
			cmd_chmod(argv, argc);
			break;
		case 12 :
			cmd_alias(argv,argc);
			break;
		case 13 :
			run(history_num(argv,argc));
			break;
		default : 
			run_alias(argv,argc);
			break;
	}
}

int _cmds_num(char* input){
	int r=14;
	int i;
	for(i=0;i<MAX_CMDSC;i++){
		if(!strcmp(input,_cmds[i])) {
			r = i;
		}
	}
	return r;
}

bool cmd_ls(char* cwd, int argc){
	if(argc==1){
		struct dirent * entry = NULL;
		DIR* dir = NULL;
	
		if((dir=opendir(cwd))==NULL){
			printf("current directory error\n");
		}
		while((entry = readdir(dir)) !=NULL){
			if(entry->d_name[0]!='.')
				printf("%s\n", entry ->d_name);
		}
		closedir(dir);
		return true;
	}else return false;

}

bool cmd_pwd(int argc){
	if(argc==1){
		printf("%s\n",current_dir());
		return true;
	}
	else return false;
}

char* current_dir(){
	return getcwd(path,256);
}

bool cmd_cd(char* argv[], int argc){
	int error=0;
	char* org = current_dir();
	if(argc<2){
		if(chdir(home_dir))printf(" Wrong name\n");
		return true;
	}
	if(argc==2){
		char slash[16] = "/";
		char org[256];
		strcpy(org , current_dir()) ;
		strcat(slash,argv[1]);
	
		strcat(org,slash);
		if(chdir(org))printf("Wrong name\n");
		return true;
	}
	else {
		if(chdir(org))fprintf(stderr,"Directory Change Failed\n");
		return true;
	}
	
}

bool cmd_mkdir(char* argv[], int argc){
	if(argc!=2){
		fprintf(stderr,"Mkdir Failed : Not enough arguments\n");
	}
	if(mkdir(argv[1],0775)){
		fprintf(stderr,"Mkdir Failed \n");
	}
}

bool cmd_rm(char* argv[], int argc){
	if(argc!=2){
		fprintf(stderr,"Remove Failed : Not enough arguments)\n");
	}
	char* path;
	path = strcat(current_dir(),"/");
	path = strcat(path,argv[1]);

	if(remove(path)==-1){
		fprintf(stderr,"Mkdir Failed \n");
	}
	return true;
}

bool cmd_cat(char* argv[], int argc){
	FILE *f;
	int loop = 0;
	char *name =argv[1];
	char buf[256];
	if(argc<2){
		fprintf(stderr,"Cat Failed : Not enough arguments\n");
	}
	do{
		loop++;
		name = argv[loop];
		f= fopen(name,"r");
		if(f==NULL){
			fprintf(stderr,"Cat Failed\n");
			return true;
		}
		while((fgets(buf,256,f))!=NULL){
			printf("%s", buf);
		}
	}while(loop<argc-1);
	fclose(f);
	return true;	
}

bool cmd_chmod(char* argv[], int argc){
	if(argc!=3) fprintf(stderr,"Chmod Failed : Not enough arguments\n");
	
	if(access(argv[1],F_OK)!=0){
		fprintf(stderr,"Chmod Failed : File does not exist\n");
		return true;
	}
	int mode = 0;
	
	sscanf(argv[1], "0%o",&mode);
	if(chmod(argv[1],mode)!=0){
		fprintf(stderr,"Chmod Failed \n");
	}
	return true;
}

bool cmd_alias(char* argv[], int argc){
	if(argc==1){
		read_txt(argv[0]);
		return true;
	}
	else{
		char* path = strcat(current_dir(),"/alias.txt");
		FILE* fp = fopen(path,"a");
		fputs(argv[1],fp);
		fclose(fp);
		return true;
	}
}

void cmd_history(){
	int i;
	for(i=0;i<stack_index;i++){
		printf("[%d] : %s \n",i,cmd_stack[i]);
	}
}

void hst_mkdir(){
	char* path = strcat(current_dir(),"/history.txt");
	FILE* fp = fopen(path,"a+");

	if(fp==NULL){
		fprintf(stderr,"fail");
	}else{
		fprintf(stderr,"success");
	}
}
void alias_mkdir(){
	char* path = strcat(current_dir(),"/alias.txt");
	FILE* fp = fopen(path,"w+");

	if(fp==NULL){
		fprintf(stderr,"fail");
	}else{
		fprintf(stderr,"success");
	}
}

void record_history(char* input){
	char* path = strcat(home_dir,"/history.txt");
	FILE* fp = fopen(path,"a");
	fputs(input,fp);
	fclose(fp);
}

void read_txt(char* cmd){
	char* path = strcat(current_dir(),"/");
	path = strcat(path,cmd);
	path =strcat(path,".txt");
	FILE* fp = fopen(path,"r");
	int number = 1;
	char buffer[64]= {0,};

	while(fgets(buffer,sizeof(buffer),fp)!=NULL){
		printf("[%d] : %s",number++ ,buffer);
	}
	printf("\n");
	fclose(fp);
}


char* history_num(char* argv[], int argc){
	if(argc!=2)  fprintf(stderr,"[!]history Failed : Enter Two Arguments\n");
	int num = atoi(argv[1]);
	char* history;
	char* path = strcat(current_dir(),"/history.txt");
	FILE* fp = fopen(path,"r");
	int number = 1;
	char buffer[64]= {0,};
	while(fgets(buffer,sizeof(buffer),fp)!=NULL){
		if(num==number){
			history = buffer;
			break;
		}
		number++;
	}
	return history;
}


void* run_alias(char* argv[], int argc){
	int i;
	char* path = strcat(current_dir(),"/alias.txt");
	char* alias_cmd[5];
	char* tmp;
	char* out="";
	FILE* fp = fopen(path,"r");
	char buffer[64]= {0,};
	while(fgets(buffer,sizeof(buffer),fp)!=NULL){
		alias_cmd[0] = strtok(buffer,"=");
		while(tmp!=NULL) {
			tmp = strtok(NULL,DELIMS);
		}	
		eliminate(tmp,'\'');		
		if(strcmp(argv[0],alias_cmd[0])){
			argv[0] = tmp;
		}
		else return"";
	}
	for(i=1;i<5;i++){
		strcat(out,argv[i]);
	}
	run(out);
}



bool cmd_stat(char* argv[], int argc){
	struct stat sb;
	char* path;
	path = strcat(current_dir(),"/");
	path = strcat(path,argv[1]);

	if(argc!=2) { 
		 fprintf(stderr,"[!]Stat Failed : Enter Two Arguments\n");
		return true;
	}
	if(stat(path, &sb)==-1){
		perror("stat");
		return true;
	}
	printf("File : %s \n",argv[1]);
	printf("Type : ");
	switch(sb.st_mode & S_IFMT){
		case S_IFBLK : printf("block device\n"); break;
		case S_IFCHR : printf("character device\n"); break;
		case S_IFDIR : printf("directory\n"); break;
		case S_IFIFO : printf("FIFO/pipe\n"); break;
		case S_IFLNK : printf("symlink\n"); break;
		case S_IFREG : printf("regular file\n"); break;
		case S_IFSOCK : printf("socket\n"); break;
		default : printf("?"); break;
	}
	printf("Size : %ld bytes\n",(long)sb.st_size);
	printf("Blocks :  %ld\n",(long)sb.st_blocks);
	printf("IO Block : %ld bytes\n",(long)sb.st_blksize);
	printf("Device : %lld\n",(long long)sb.st_dev);
	printf("Link: %ld\n",(long)sb.st_nlink);
	printf("Inode: %ld\n",(long)sb.st_ino);
	printf("Access: %s",ctime(&sb.st_atime));
	printf("Modify: %s",ctime(&sb.st_mtime));
	printf("Change: %s",ctime(&sb.st_ctime));

	return true;
	}
}

bool cmd_ps(char* argv[], int argc){
	DIR * dir;
   	struct dirent *entry;
   	struct stat fileStat;
   	int pid=0;
  	 char cmdLine[256];
   	char tempPath[256];
   	char tty_self[256];
   	char path[256];

   	char *tty;
   	int fd;
   	int fd_self = open("/proc/self/fd/0",O_RDONLY);
   	sprintf(tty_self,"%s",ttyname(fd_self));

 	 dir = opendir("/proc");

   	printf("PID COMMAND\n");
   	while((entry = readdir(dir)) !=NULL){
      		lstat(entry->d_name,&fileStat);
      		if(!S_ISDIR(fileStat.st_mode))
         			continue;
     		pid = atoi(entry->d_name);
      		if(pid <=0) continue;
      		sprintf(path,"/proc/%d/fd/0",pid);

   		fd = open(path,O_RDONLY);
      		tty = ttyname(fd);

      		sprintf(tempPath,"/proc/%d/cmdline",pid);
      
     		if(tty&&strcmp(tty,tty_self) ==0){
         		printf("%d %s\n",pid,cmdLine);
		}
	}
   	closedir(dir);
	return true;
}

bool cmd_cp(char* argv[], int argc){

	int fexist, fanew;
	ssize_t nread;
	char line[128];
	char c;
	
	if(argc!=3){
		fprintf(stderr,"[!]Copy Failed : Enter Two Arguments\n");
		return true;
	}
	fexist =open(argv[1],O_RDONLY);
	if(fexist==-1){
		fprintf(stderr,"[!]Copy Failed : File does not exist\n");
		exit(1);
	}
	fanew = open(argv[2],O_RDONLY|O_CREAT|O_EXCL,0644);
	if(fanew==-1){
		fprintf(stderr,"[!]Copy Failed : File does not exist\n");
		exit(1);
	}
	while((nread = read(fexist,line,sizeof(line)))>0){
		if(write(fanew,line,nread)>0) break;
	}
	//fclose(fexist);
	//fclose(fanew);
	return true;
}
bool cmd_ln(char* argv[], int argc){
	if(argc!=3){
		fprintf(stderr,"[!]Hard Link Failed : Enter Two Arguments\n");
		return true;
	}
	if(link(argv[1],argv[2])==-1){
		fprintf(stderr,"[!]Hard Link Failed : Enter Two Arguments\n");
		return true;
	}
}
int main(){
	char input[256];
	char* hst[256];
	home_dir=current_dir();
	hst_mkdir();
	alias_mkdir();
	do{
		printf("%s $~ : ",current_dir());
		fgets(input, sizeof(input), stdin);
		record_history(input);
		run(input);
	}while(strcmp(input,"exit"));

	return 0;
}

// 주석 추가 test