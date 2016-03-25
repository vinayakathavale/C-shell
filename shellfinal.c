#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<termios.h>
#include<errno.h>
pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;
void initshell ()
{

  /* See if we are running interactively.  */
  shell_terminal = STDIN_FILENO;
  shell_is_interactive = isatty (shell_terminal);
  if (shell_is_interactive)
    {
      /* Loop until we are in the foreground.  */
      while (tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp ()))
        kill (- shell_pgid, SIGTTIN);

      /* Ignore interactive and job-control signals.  */
      signal (SIGINT, SIG_IGN);
      signal (SIGQUIT, SIG_IGN);
      signal (SIGTSTP, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
      signal (SIGTTOU, SIG_IGN);
      signal (SIGCHLD, SIG_IGN);

      /* Put ourselves in our own process group.  */
      shell_pgid = getpid ();
      if (setpgid (shell_pgid, shell_pgid) < 0)
        {
          perror ("Couldn't put the shell in its own process group");
          exit (1);
        }

      /* Grab control of the terminal.  */
      tcsetpgrp (shell_terminal, shell_pgid);

      /* Save default terminal attributes for shell.  */
      tcgetattr (shell_terminal, &shell_tmodes);
    }
}

char *home;
typedef struct proc
{
	int id;
	char process[1024];//stores mapping of process and process id
}proc;

int no=0;
proc pro[1024];
proc jobs[1024];
char built_func[4][5];//stores built in functions
int size_builtin=4;
void gen()//generating builtin functions
{
	strcpy(built_func[0],"echo");
	strcpy(built_func[1],"cd");
	strcpy(built_func[2],"pwd");
	strcpy(built_func[3],"exit");
}
int cd(char **args)//cd command
{

	if(!args[1])//check for no argument in cd
	{

		chdir(home);
		return 1;

	}
	int i=0;
	char dir[1024];
	strcpy(dir,args[1]);
	for(i=0;dir[i]!='\0';i++) //check for ~ in cd command
		if(dir[i]=='~' && i>0)
			perror("error");
	if(dir[0]=='~')
	{
		char temp1[1024];
		char temp2[1024];
		int j=1;
		int m=0;
		for(;dir[j]!='\0';j++,m++)
			temp1[m]=dir[j];
		int k=m;
		j=0;
		for(;home[j]!='\0';j++)
		{
			temp2[j]=home[j];
		}
		int l=0;
		for(l=0;l<k;l++)
		{
			temp2[j]=temp1[l];
			j++;
		}
		temp2[j]='\0';
		strcpy(dir,temp2);
	}
	strcpy(args[1],dir);
	if(args[1][0]!='\0')
	{
		if(chdir(args[1])!=0)
		{
			perror("error");
		}
	}
	return 1;
}
int fg_to_bg(proc t,proc u,int val)
{

	tcsetpgrp(shell_terminal,shell_pgid);

  	tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes);	
	int i;
	for(i=val-1;i<no-1;i++)
	{
		pro[i]=pro[i+1];
		jobs[i]=jobs[i+1];
	}
	pro[no-1]=t;
	jobs[no-1]=u;
}
int killall()
{
	int i=0;
	for(i=1;i<no;i++)
	{
		kill(pro[i].id,9);
	}
	no=1;
	return 1;
}
int bg_to_fg(char **args)
{
	int i=0;
	int val=0;
	while(args[1][i]!='\0')
	{
		val=10*val+args[1][i]-'0';
		i++;
	}
//	printf("%d",val);
//	int bash=getppid();
	//int grp=getpid(pro[val-1].id);
	
//	tcsetpgrp(STDIN_FILENO,grp);
	int pgid=getpgid(pro[val-1].id);
	tcsetpgrp(STDIN_FILENO,pgid);
//	setpgid(pro[val-1].id,pgid);
//	tcsetpgrp(STDIN_FILENO,pgid);
//	int grp=tcgetpgrp(getpid());
	
//	int grp= getpgid(pro[val-1].id);
//	int grp=tcsetpgrp(p
//	setpgid(pro[val-1].id,grp);

	kill(pro[val-1].id,SIGCONT);
	int status,wpid;
	do 
	{
			//	printf("1");
		wpid = waitpid(pro[val-1].id, &status, WUNTRACED);
		if(WIFSTOPPED(status))
		{
			fg_to_bg(pro[val-1],jobs[val-1],val);
			return 1;

		}
//		tcsetpgrp(STDIN_FILENO,pgid);
//

	}while (!WIFEXITED(status) && !WIFSIGNALED(status));//wait for foreground process*/
//	printf("aravind\n");
//	printf("%d \n",getpid());
	tcsetpgrp(STDIN_FILENO,getpgid(getpid()));
//	kill(getpid(),SIGCONT);
	proc t;
	int c=0;
	int j=0;
	proc temp[1024];
	proc temp2[1024];
	for(c=0;c<no;c++)
	{
		if(c!=val-1)
		{	
			temp[j]=pro[c];
			temp2[j]=jobs[c];
			j++;
		}
	}
	no=j;
	for(c=0;c<no;c++)
	{
		pro[c]=temp[c];
		jobs[c]=temp2[c];
	}
	return 1;
}
int echo(char **args,int argc) //echo command
{


	if (argc > 1)
	{

		int j;
		for(j=0;args[1][j]!='\0';j++)
		{
			if(args[1][j]!='"')
				printf("%c",args[1][j]);

		}
		printf(" ");
	}
	int i;
	for (i = 2; i < argc; i++)
	{
		int j;
		for(j=0;args[i][j]!='\0';j++)
		{
			if(args[i][j]!='"')
				printf("%c",args[i][j]);

		}
		printf(" ");
	}

	printf("\n");
	return 1;
}
int  pwd()//pwd command
{	char cwd[1024];
	getcwd(cwd,sizeof(cwd));
	printf("%s\n",cwd);
	return 1;
}
int spawn(char **args,int in,int out)
{
	int i;
	for(i=0;args[i]!=NULL;i++);
	int len=i;
	if(strcmp(args[len-1],"&")==0)//background process
	{
		pid_t pid;
		args[len-1]=NULL;
		pid=fork();
		if(pid==0)
		{
			setpgid(0,0);
					
			signal (SIGINT, SIG_DFL);
      			signal (SIGQUIT, SIG_DFL);
     			signal (SIGTSTP, SIG_DFL);
      			signal (SIGTTIN, SIG_DFL);
      			signal (SIGTTOU, SIG_DFL);
      			signal (SIGCHLD, SIG_DFL);
			if (execvp(args[0], args) == -1) 
			{
				perror("error");
			}
			//	execvp(args[0],args);
	//		tcsetpgrp(STDIN_FILENO,getpgrp());
		}			
		else if(pid<0)
		{

			perror("error forking");
		}
		else
		{

			setpgid(pid,pid);
			pro[no].id=(int)pid;
			jobs[no].id=(int)pid;
			strcpy(jobs[no].process,"\0");
			strcpy(pro[no].process,args[0]);
			int j=0;
			for(;args[j]!=NULL;j++)
			{	
				strcat(jobs[no].process,args[j]);
				strcat(jobs[no].process," ");

			}
			no++;
		}
		return 1;
	}
	pid_t pid, wpid;//foreground processes
	int status=0;
	pid = fork();
	if (pid == 0) 
	{
		setpgid(0,0);
		signal (SIGINT, SIG_DFL);
      		signal (SIGQUIT, SIG_DFL);
     		signal (SIGTSTP, SIG_DFL);
      		signal (SIGTTIN, SIG_DFL);
      		signal (SIGTTOU, SIG_DFL);
      		signal (SIGCHLD, SIG_DFL);
		
		if (in != 0)
		{
			dup2 (in, 0);
			close (in);
		}
		if (out != 1)
		{
			dup2 (out, 1);
			close (out);
		}
		int i;
		for(i=0;args[i]!=NULL;i++);
		int argc=i;
		i=0;
		for(i=0;i<size_builtin;i++)//if it is builtin
		{

			if(strcmp(args[0],built_func[i])==0)
			{
				int val;
				if(i==0)
					val= echo(args,argc);
				if(i==2)
					val=  pwd(in);
				kill(getpid(),9);
				return val;
			}

		}
		if(strcmp(args[0],"jobs")==0)
		{
			int val=printbgjobs();
			kill(getpid(),9);
			return val;
		}
		if (execvp(args[0], args) == -1) 
		{
			perror("error");
		}
		exit(EXIT_FAILURE);
	} 
	else if (pid < 0) 
	{
		// Error forking
		perror("error forking");
	} 
	else 
	{
//		int pgid = getpgid (pid);
  //    		if(pgid == 0) pgid = pid;
     		 setpgid (pid, pid);
		int pgid=getpgid(pid);
        	tcsetpgrp (shell_terminal, pgid);
		do 
		{
			//	printf("1");
			wpid = waitpid(pid, &status, WUNTRACED);

			if(WIFSTOPPED(status))
			{

				tcsetpgrp(shell_terminal,shell_pgid);

  				tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes);	
				pro[no].id=(int)pid;
				
				jobs[no].id=(int)pid;
				strcpy(jobs[no].process,"\0");
				strcpy(pro[no].process,args[0]);
					int j=0;
				for(;args[j]!=NULL;j++)
				{	
					strcat(jobs[no].process,args[j]);
					strcat(jobs[no].process," ");

				}
				no++;
				return 1;

			
		
				
			}

		}while (!WIFEXITED(status) && !WIFSIGNALED(status));//wait for foreground process
		tcsetpgrp (shell_terminal, shell_pgid);
	//	tcgetattr (shell_terminal, &j->tmodes);
	//
  		tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes);	
	}			                                                    
	return 1;
}
int printbgjobs()
{
	int i=0;
	for(;i<no;i++)
	{
		printf("[%d] %s[%d]",i+1,jobs[i].process,jobs[i].id);
		printf("\n");

	}
	return 1;
}
int kjobs(char **args)
{
	int i=0;
	int val=0;;
//	printf("%s ",args[1]);
	while(args[1][i]!='\0')
	{
		
		val=10*val+args[1][i]-'0';
		i++;
	}
//	printf("%d",val);
///	printf("%d" ,pro[val-1].id);

	int sig=0;
	i=0;
	while(args[2][i]!='\0')
	{
		sig=10*sig+args[2][i]-'0';
		i++;
	}
//	printf("%d \n",sig);
	kill(pro[val-1].id,sig);
	return 1;
}
int exec(char **args,int in,int out)
{
	if(args[0]==NULL)
		return 1;
	if(strcmp(args[0],"exit")==0)
		return 0;
	if(strcmp(args[0],"quit")==0)
		return 0;
	if(strcmp(args[0],"cd")==0)
		return cd(args);
	if(strcmp(args[0],"overkill")==0)
		return killall();
	if(strcmp(args[0],"fg")==0)
		return bg_to_fg(args);
	if(strcmp(args[0],"kjob")==0)
		return kjobs(args);
	return spawn(args,in,out);
}
char *readline(void)//readline
{
	char *line = NULL;
	ssize_t size = 0;
	getline(&line, &size, stdin);
	return line;
}
char **split(char *line,char *delim)//split using delimiters
{
	char * newline=(char*)malloc((1024)*sizeof(char));
	strcpy(newline,line);
	int size = 20,i = 0;
	char **tokens = malloc(size * sizeof(char*));
	char *token;
	if (!tokens) 
	{
		fprintf(stderr, "allocation error\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(newline, delim);
	while (token != NULL) 
	{
		tokens[i] = token;
		i++;
		if (i >= size) 
		{
			size+=20;
			tokens = realloc(tokens,size * sizeof(char*));
			if (!tokens) 
			{
				fprintf(stderr, "allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, delim);
	}
	tokens[i] = NULL;
	return tokens;
}
void remove_np(char * a)
{
	char ** nline = split(a," \t\n");
	int i;
	for(i=0;i<=strlen(nline[0]);i++)
		a[i]=nline[0][i];

}
void checkprint(char **args)//checkprint
{
	int i;
	for(i=0;args[i]!=NULL;i++)
		printf("%s\n",args[i]);
}
int main()
{
	initshell();
	gen();
	char username[1024];
	getlogin_r(username,sizeof(username));
	char hostname[1024];
	char path[1024];
	char path_tild[1024];
	getcwd(path_tild,sizeof(path_tild));
	//	strcpy(home,path_tild);
	home=strdup(path_tild);
	int len_tild=strlen(path_tild);
	gethostname(hostname,sizeof(hostname));
	int pid_main=getpid();
	strcpy(pro[0].process,"a.out ");
	strcpy(jobs[0].process,"a.out ");
	pro[0].id=jobs[0].id=pid_main;
	no++;
	strcpy(path,"~");
	int status=1;
	int saved_stdout = dup(1);
	int saved_stdin = dup(0);
	do
	{
		int status1;
		pid_t t;

//		printf("ara");

		proc temp3[1024];
		proc temp4[1024];
	//	while((t=waitpid(-1,&status1,WNOHANG))>0)//check for background processes status
	//	{

//			if(WIFEXITED(status1)==0)
//			{
				int c=0;
				int m=0;
		//		printf("arbads");
				
				for(c=0;c<no;c++)
				{
				//	pid_t pid_result = waitpid(pro[c].id, &status, WNOHANG);

					if(kill(pro[c].id,0)==-1 && errno==ESRCH)
						printf("Process %s  exited whose id is %d\n",pro[c].process,pro[c].id);
					else
					{	
						temp3[m]=pro[c];
						temp4[m]=jobs[c];
						m++;
					}
				}
				no=m;
				for(c=0;c<no;c++)
				{
					pro[c]=temp3[c];
					jobs[c]=temp4[c];
				}
			
	//	}
		printf("<%s@%s:%s>",username,hostname,path);
		char *line;
		line=readline();
		char **args2;
		args2=split(line,";");
		int i;
		char **afterpiping;
		char **afterredirect,**afterredirect2,**afterredirect3;
		for(i=0;args2[i]!=NULL;i++)
		{
			afterpiping = split(args2[i],"|");

	//printf("%s %s",afterpiping[0],afterpiping[1]);
			int no_of_pipes=0;
			int piperead=0;
			while(afterpiping[no_of_pipes]!=NULL)no_of_pipes++;
			//dout(no_of_pipes);
			int loop_var=0;
			int fd[2];
			for (loop_var = 0; loop_var < no_of_pipes ; ++loop_var)
			{

				pipe (fd);
				int in,out;
				int s1=0,s2=0;
				afterredirect = split(afterpiping[loop_var],"<");
				if(strcmp(afterredirect[0],afterpiping[loop_var])==0)
				{
					afterredirect2 = split(afterpiping[loop_var],">");
					if(strcmp(afterredirect2[0],afterpiping[loop_var])!=0)
					{

						remove_np(afterredirect2[1]);
						out = open(afterredirect2[1], O_WRONLY | O_TRUNC | O_CREAT | O_RDONLY, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
						if(out==-1)
						{
							perror("Cant open such a file");
							break;

						}	
						dup2(out,1);
						s2=1;
						strcpy(afterpiping[loop_var],afterredirect2[0]);
					}
					
					
				

				}
				else
				{
					strcpy(afterpiping[loop_var],afterredirect[0]);
					afterredirect3 = split(afterredirect[1],">");
					if(strcmp(afterredirect[1],afterredirect3[0])==0)
					{
						remove_np(afterredirect3[0]);
						in = open(afterredirect3[0], O_RDONLY);
						if(in==-1)
						{
							perror("Cant open file with such a name");
							break;
							
						}

						dup2(in,0);
						s1=1;
					}
					else
					{

						remove_np(afterredirect3[1]);
						remove_np(afterredirect3[0]);
						out = open(afterredirect3[1], O_WRONLY | O_TRUNC | O_CREAT | O_RDONLY, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
						in = open(afterredirect3[0], O_RDONLY);

						if(in==-1)
						{
							perror("Cant open file with such a name");
							break;
						}
						dup2(in,0);

						s1=1;

						if(out==-1)
						{
							perror("Cant open such a file");
							break;
						}
						dup2(out,1);
						s2=1;
					}

				}

				char **args1;

				args1=split(afterpiping[loop_var]," \t\n");
				if(loop_var==no_of_pipes-1)
				{
					if(exec(args1,piperead,1)==0)
						return 0;

				}
				else
				{
					if(exec(args1,piperead,fd[1])==0)
						return 0;
					
				}

				free(args1);
				if(s1)
				close(in);
				if(s2)
				close(out);
				close (fd[1]);
				piperead = fd[0];
			}
			dup2(saved_stdout,1);
			dup2(saved_stdin,0);
		}
		getcwd(path,sizeof(path));
		char temp[1024];
		int len_path=strlen(path);
		int j;
		temp[0]='~'; 
		int flag=0;
		for(j=0;j<len_tild;j++)//check for path
		{
			if(path_tild[j]!=path[j])
			{
				flag=1;
				break;
			}
		}
		if(flag==0)
		{	
			for(i=1;j<=len_path;j++,i++)
				temp[i]=path[j];
			strcpy(path,temp);
		}
		free(args2);
	}while(status);
	return 0;
}





































