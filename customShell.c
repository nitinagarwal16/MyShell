#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
void myexec(char **arg)
{
	if(!(strcmp(arg[0],"cd")==0))
	{
		execvp(arg[0],arg);
		write(1,arg[0],strlen(arg[0]));
		write(1,": No such command found\n",strlen(": No such command found\n"));
	}
	_exit(1);
}
int  mySplit(char *I,char **in, int *flag,int *flag2,int *pipArr,int *sizePip)
{
	int index=*sizePip;
	int indicate=0;
	strcat(I," ");
	int i=strlen(I);
	int k=0,j,p=-1;
	for(j=0;j<i;j++)
	{
		if(I[j]!=' '&&I[j]!='|'&&I[j]!='>'&&I[j]!='<'&&I[j]!='&')
		{
			sprintf(in[k],"%s%c",in[k],I[j]);
		}
		else
		{

			sprintf(in[k],"%s%c",in[k],'\0');
			k++;
		}
		if(I[j]=='|')
		{

			pipArr[index++]=k-1;
			p=1;
		}
		if(I[j]=='>'||I[j]=='<')
		{
			*flag=k-1;
			p=2;
			if(I[j]=='>')
				*flag2=1;
			else
				*flag2=2;

		}
		if(I[j]=='&')
		{
			p=3;
			j++;
		}
	}
	*sizePip=index;
	in[k]=NULL;
	return p;
}
void split2(char **arg,char **arr1, char **arr2, int k)
{
	int i,index=0;

	for(i=0;i<=k;i++)
	{

		strcpy(arr1[i],arg[i]);

	}	
	arr1[i]=NULL;

	for(i=k+1;arg[i]!=0;i++)
	{
		strcpy(arr2[index++],arg[i]);
	}

	arr2[index]=NULL;
}
void fileRedirection(char **arg, int k,int flag)
{

	int i;
	char **arr1;
	char **arr2;
	arr1=(char **)malloc(10*sizeof(char *));
	for(i=0;i<10;i++)
		arr1[i]=(char *)malloc(20*sizeof(char));
	arr2=(char **)malloc(10*sizeof(char *));
	for(i=0;i<10;i++)
		arr2[i]=(char *)malloc(20*sizeof(char));

	split2(arg,arr1,arr2,k);
	if(flag==1)	
		freopen(arr2[0],"w",stdout);
	else if(flag==2)
		freopen(arr2[0],"r",stdin);
	myexec(arr1);

}
void Shandler(int signum)
{
	while(waitpid(-1,0,WNOHANG)>0);
}
void background(char **arg)
{
	pid_t pid;
	int status;
	pid=fork();
	if(pid>0)
	{
		signal(SIGCHLD,Shandler);
		_exit(1);
	}
	else
	{
		setpgid(0,0);
		myexec(arg);
	}

}
void pipeProcess(char **arg, int *k,int size)
{
	int s;
	fflush(stdout);
	fflush(stdout);
	int i,*pfd,pid,fd=0,ii,m,status,index=0,m1;
	char **arr;
	pfd=(int *)malloc(2*sizeof(int));
	
	for(i=0;i<=size;i++)	
	{
		m1=0;
		arr=(char **)malloc(5*sizeof(char *));
		for(ii=0;ii<5;ii++)
			arr[ii]=(char *)malloc(10*sizeof(char));
		if(i!=size)
		{
			if(i==0)
				m=0;
			else
				m=k[i-1]+1;
			for(;m<=k[i];m++)
			{	
				strcpy(arr[m1++],arg[index++]);
			}
			arr[m1]=NULL;
		}
		else if(i==size)
		{
			for(m=0;arg[index]!=0;m++)
			{
				strcpy(arr[m1++],arg[index++]);
			}
			arr[m1]=NULL;
		}
		pipe(pfd);
		pid=fork();
		if(pid==0)
		{
			dup2(fd,0);
			if(i!=size)
			{
				dup2(pfd[1],1);
			}
			else
				dup2(1,1);
			close(pfd[1]);
			close(pfd[0]);
			if(execvp(arr[0],arr)==-1)
				perror("error\n");
		}
		else if(pid>0)
		{
			waitpid(pid,0,0);
			close(pfd[1]);
			fd=pfd[0];
			if(i==size)
				exit(0);
		}
		for(s=0;s<=m1;s++)
			free(arr[s]);
	}
	
}
int main()
{
	while(1)
	{
		int k,i,ik,status,pipeflag,flag2=0,fileflag=0,pipArr[10],sizePip=0;
		pid_t pid;
		write(1,"\033[34m?\033[0m",strlen("\033[34m?\033[0m"));
		char *I;
		I=(char *)malloc(100);
		gets(I);
		char **in;
		in=(char **)malloc(10*sizeof(char *));
		for(ik=0;ik<10;ik++)
			in[ik]=(char *)malloc(20*sizeof(char));
		pipeflag=mySplit(I,in,&flag2,&fileflag,pipArr,&sizePip);
		if(strcmp(in[0],"exit")==0)
			_exit(0);
		if(strcmp(in[0],"")==0)
			continue;
		if(strcmp(in[0],"cd")==0)
		{	
			if (in[1] == NULL)
				fprintf(stderr, "expected argument to \"cd\"\n");
			else if (chdir(in[1]) != 0)
				perror("Error! cd failed");
		}
		pid=fork();
		if(pid==0)
		{
			if(pipeflag==-1)	
				myexec(in);
			else if(pipeflag==1)
				pipeProcess(in,pipArr,sizePip);
			else if(pipeflag==2)
				fileRedirection(in,flag2,fileflag);
			else if(pipeflag==3)
			{
				fflush(stdout);
				background(in);
			}
		}
		else 
		{
			if(pipeflag!=3)
				waitpid((pid_t)pid,0,0);
			free(I);
		}
	}
}
