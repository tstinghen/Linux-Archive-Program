/*
  Name: Tracy Stinghen 
  OSU email: stinghet@onid.oregonstate.edu 
  Class: CS 344-400
  Assignment: Homework #4 Part 2
  
 
*/


#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h> 
#include <limits.h>


int get_fd( int* file_arr, int num_fd, int cur_fd);
/*
Resources: 
http://stackoverflow.com/questions/11198604/c-split-string-into-an-array-of-strings
*/
int main (int argc, char **argv)
{
	char *buff;	   
	char *p; 
	
	
	char key; 

	char file_name[50] = "myfile.txt"; 
	
	char temp_file_base[] = "stinghet-temp-"; /* to be appended, to make temporary storage files */
	char temp_file_curr[50]; 
	char add_I[] = "I"; 	

	struct stat file_stat; 
	
	int word_len = 0;
	
	int num_sorts; 				/*number of sort functions */
	int arr_files[50]; 	  	  	  /*array of file descriptors */ 
	int arr_out_files[50]; 
	
	char plain_new_line[] = "\x0A"; 	/*creates a new line to print in file*/
	char *new_line = plain_new_line; 
	
	int orig_file; 
	int out_file; 
	int temp_file;
	int temp_out_file; 

	 
	int bytes_read; 
	int file_size; 
	int write_spot = 0; 

	int i; 
	int j; 
	
	/* pipe variables */
	char *uniq_args[] = {"uniq", "-c", NULL};
/*	  char *sort_args[] = {"sort", NULL};
	char *sort_m_args[] = {"sort", "-m", NULL}; */
	int status;
	/*
	char *cat_args[] = {"cat", NULL};
	*/
	
	int a_pipe[50][2]; 
	/*int pipe_one[2];
	int pipe_two[2];*/
	/*int pipe_three[2];*/
	 

															
	if(argc < 3)
	{
		num_sorts = 1; 
		
		if(argc == 2)
		{

			strcpy(file_name, argv[3]); 
			
		}
	
	}
	
	else 
	{
		key = getopt(argc, argv, "n");
		
		switch (key) 
		{
			case 'n' : 
			
				num_sorts = atoi(argv[2]);
				
				if(num_sorts >= 50) 
				{
					printf("Error: Chosen number of sorts is too high.\nReverting to one sort.\n"); 
					num_sorts = 1; 
				} 
			
				if(argc == 4)
				{
					strcpy(file_name, argv[3]); 
				
				}
				break; 
		
			
			default: 
				printf("No idea what's going on here.\nReverting to one sort.\n"); 
				num_sorts = 1; 
				if(argc == 4)
				{
					strcpy(file_name, argv[3]); 
				
				}
				break; 
		
		}

	
	}
	
	
	orig_file = open(file_name, O_RDONLY, S_IWRITE | S_IREAD);

	out_file = open("uniq_outfile.txt", O_RDWR | O_TRUNC | O_CREAT, S_IWRITE | S_IREAD); 

	
	stat(file_name, &file_stat); 								/*gets the size of the file */
	file_size = (int) file_stat.st_size; 
	
	printf("File Size: %d\n", file_size);
	
	
	buff = malloc(sizeof(file_size + 1)); 	

	/******** read data from file *********/
	
	
	bytes_read = read(orig_file, buff, file_size); 
	
	if(bytes_read < file_size) 
	{
		printf("Error: %s\n", strerror(errno)); 
		printf("%d bytes read of %d\n", bytes_read, file_size); 
	} 
	
	
	/********************************* 
	THIS WORKS GREAT for getting all the data into the file 
	 but fails when iterating through to format. 
	 Re-instate if I can figure that out. 
	***********************************
	
	while((bytes_read + 10000) < file_size)
	{

	bytes_read = bytes_read + pread(orig_file, buff, 10000, bytes_read); 
	
	}
	
	while((bytes_read + 100) < file_size)
	{

	bytes_read = bytes_read + pread(orig_file, buff, 100, bytes_read); 
	
	}
	
	while(bytes_read < file_size)
	{
	
	bytes_read = bytes_read + pread(orig_file, buff, 1, bytes_read); 
	
	}
	
	if(bytes_read < file_size) 
	{
		printf("Error: %s\n", strerror(errno)); 
		printf("%d bytes read of %d\n", bytes_read, file_size); 
	} 
	printf("%d bytes read of %d\n", bytes_read, file_size);
	
	********* format text ***********/ 
	i = 0; 
	while(i < bytes_read) 
	{
		 
		if(isalpha(buff[i]) == 0)
		{
			buff[i] = ' '; 
		
		}
	
		if(isupper(buff[i]))
		{
			buff[i] = tolower(buff[i]); 
		}
	
		i++; 
		
	}

	/******* create n file descriptors ***********/ 
	strcpy(temp_file_curr, temp_file_base);
	
	for(i = 0; i < num_sorts; i++) 
	{
		
		strcat(temp_file_curr, add_I); 
		temp_file = open(temp_file_curr, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);
	
		arr_files[i] = temp_file; 
	
		/*printf("#%d: %s FD: %d \n", i, temp_file_curr, arr_files[i]); */
	}
	
	strcpy(temp_file_curr, temp_file_base);
	strcat(temp_file_curr, "out"); 
	
	for(i = 0; i < num_sorts; i++) 
	{
		
		strcat(temp_file_curr, add_I); 
		temp_out_file = open(temp_file_curr, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);
	
		arr_out_files[i] = temp_out_file; 
	
		/*printf("#%d: %s FD: %d \n", i, temp_file_curr, arr_out_files[i]); */ 
	}
	
	
	
	/******* distribute data among files *******/
	
	p = strtok(buff, " "); 
	
	temp_file = arr_files[0]; 
	
	while(p)
	{  
		word_len = strlen(p);
		if(word_len > 1)
		{ 
			fstat(temp_file, &file_stat); 
			file_size = (int) file_stat.st_size; 
			
			write_spot = file_size; 
		
			word_len = strlen(p);
			
			pwrite(temp_file, p, word_len, write_spot); 
			
			write_spot = write_spot + word_len; 
			
			pwrite(temp_file, new_line, 1, write_spot); 
			
			write_spot++; 
			
			temp_file = get_fd(arr_files, num_sorts, temp_file);
		}
			p = strtok(NULL, " ");
			
		


	}

	
/*	  free(buff); */



	
	/*printf("FD for orig = %d\n", orig_file); */
/*
	for (i = 0; i < num_sorts; i++)
	{
		pipe(a_pipe[i]); 
	
	}
	
	
	for(i = 0; i < num_sorts; i++)
	{
	
		temp_file = get_fd(arr_files, num_sorts, temp_file);
		printf("Temp file: %d\n", temp_file); 
		dup2(temp_file, 0); 
		
		temp_out_file = get_fd(arr_out_files, num_sorts, temp_out_file);
		printf("Temp out file: %d\n", temp_out_file); 
		dup2(temp_out_file, 1); 
	
		
		
		if(fork() == 0) 
		{

			dup2(a_pipe[i][0], 0); 
			dup2(a_pipe[i+1][1], 1); 
			
				for (j = 0; j < num_sorts; j++)
				{
					close(a_pipe[j][0]); 
					close(a_pipe[j][1]); 	
					
				}
			
			execvp(*sort_m_args, sort_m_args);	 
			   	   
			
			 	  
		}
		else 
		{
			printf("In parent"); 
			for (j = 0; j < num_sorts; j++)
			{
				close(a_pipe[j][0]); 
				close(a_pipe[j][1]); 	
			}
		
		}
	
	
	}
	
	for (j = 0; j < num_sorts; j++)
	{
		close(a_pipe[j][0]); 
		close(a_pipe[j][1]); 	
	}

	
	
	for(j = 0; j < num_sorts; j++)
	{
		wait(&status); 
	
	}
	
	for(i = 0; i < num_sorts; i++) 
	{
	temp_file = get_fd(arr_files, num_sorts, temp_file);	
	temp_out_file = get_fd(arr_out_files, num_sorts, temp_out_file);
	printf("Temp out file: %d\n", temp_out_file); 
	printf("Temp file: %d\n", temp_file);  
	printf("Temp file direct access %d\n", arr_out_files[i]);  

	}
	
*/
	
	
		
	strcpy(temp_file_curr, temp_file_base);
	/*strcat(temp_file_curr, "out"); */
	
	for(i = 0; i < num_sorts; i++) 
	{
		
		strcat(temp_file_curr, add_I);
		unlink(temp_file_curr);  
	}

/**********************Do Another set of Children ***************/ 

	 
	dup2(out_file, 1); 
	
	for (i = 0; i < num_sorts; i++)
	{
		pipe(a_pipe[i]); 
	
	}
	
	
	for(i = 0; i < num_sorts; i++)
	{
	
		/*temp_out_file = get_fd(arr_out_files, num_sorts, temp_out_file);
		printf("Temp out file: %d\n", temp_out_file); 
		dup2(temp_out_file, 0); 
		dup2(out_file, 1); */
		
		temp_file = get_fd(arr_files, num_sorts, temp_file);
		dup2(temp_file, 0); 
	
		
		if(fork() == 0) 
		{
			printf("In child\n");
			/*printf("With Temp out file: %d\n", temp_out_file);*/ 
			dup2(a_pipe[i][0], 0); 
			dup2(a_pipe[i+1][1], 1); 
			
			for (j = 0; j < num_sorts; j++)
			{
				close(a_pipe[j][0]); 
				close(a_pipe[j][1]); 	
				
			}
			
			execvp(*uniq_args, uniq_args);	   	   
			printf("Passed exec, shouldn't get here \n");
			 	  
		}
		else 
		{
			for (j = 0; j < num_sorts; j++)
			{
				close(a_pipe[j][0]); 
				close(a_pipe[j][1]); 	
			}
		
		}
	
	
	}
	
	for (j = 0; j < num_sorts; j++)
	{
		close(a_pipe[j][0]); 
		close(a_pipe[j][1]); 	
	}

	
	
	for(j = 0; j < num_sorts; j++)
	{
		wait(&status); 
	
	}
	
	  
	strcpy(temp_file_curr, temp_file_base);
	strcat(temp_file_curr, "out");
	
	for(i = 0; i < num_sorts; i++) 
	{
		
		strcat(temp_file_curr, add_I);
		unlink(temp_file_curr);  
	}



	return 0; 
}


/********* Function: Get Next File Descriptor ***********/


int get_fd( int* file_arr, int num_fd, int cur_fd)
{
	int i; 
	
	if (num_fd == 1)
	{
		return cur_fd; 
	}
	
	if (num_fd > 1)
	{
		if(cur_fd == file_arr[num_fd - 1])
		{
			return file_arr[0]; 
		}
		
		for(i = 0; i < num_fd; i++)
		{
			if(cur_fd == file_arr[i]) 
			{
				return file_arr[i+1]; 
			}	 
		}
	
	}


	return 0; 
}

