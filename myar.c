/*
  Name: Tracy Stinghen 
  OSU email: stinghet@onid.oregonstate.edu 
  Class: CS 344-400
  Assignment: Homework #3 Part 3
  
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/utsname.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <fcntl.h>
#include <ar.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>


void append_dir(int argc, char **argv); 

void extract(int argc, char **argv); 

void append(int argc, char **argv);

void table(int argc, char **argv); 

void delete(int argc, char **argv);

void verbose_table(int argc, char **argv); 

void print_time(int epoch_time); 

void statify(struct stat read_stat, char *file_to_add, char *header);  

int len_int(int x);

int make_file(int argc, char **argv, int prev_bytes, char *file_name, int file_size, char *file_size_buff, char *buff);





int main(int argc, char **argv)
{
	/* myar key archive-file [member [...]] */ 

char key; 

	if(argc < 3) 
	{
		printf("Too few arguments\n"); 
		exit(1); 
	}
	

while ((key = getopt(argc, argv, "qxtvdAw:")) != -1)

	switch (key) 
	{
		case 'q' :
					/*"quick append" or create new ar file */ 
			append(argc, argv); 
		
			break; 
		
		case 'x' :
					/*make copies of selected files from archive*/
			extract(argc, argv); 
			
			break; 
			
		case 't' :		/*table of contents*/ 
	
			table(argc, argv); 	   
			break; 
			
		case 'v' :		/*verbose table of contents*/
		
			verbose_table(argc, argv); 
			break; 
			
		case 'd' :		/*delete named files*/
			delete(argc, argv); 
			break; 
			
		case 'A' :	/*append entire directory */
			append_dir(argc, argv); 
			break; 
			
		case 'w' :
			
			break; 
			
		default: 
		
			printf ("Option not recognized\n"); 
	}






	return 0; 
}
/****************************************
******************************************
APPEND DIRECTORY
******************************************
******************************************/

/*Note on Regular Files: 
Understanding from:
http://en.wikipedia.org/wiki/Unix_file_types
Regular files encompass pretty much everything 
one things of as a "file" including text files 
and binary files (such as executables). Unix 
does not distinguish between different file
types denoted by suffixes (such as .a, txt). 
These are only useful for humans. Unix treats 
them all the same, as regular files, just 
strings of data. 

Special files, which are not regular files, 
include directories, symbolic links, pipes 
and sockets. 

In this function, I narrow the definition of 
regular file to only include files with a 
suffix (which excludes executable files.) 
This is for ease of testing. The filtering 
mechanism would be easy to comment out, and 
would then include all regular files. 

*/ 



void append_dir(int argc, char **argv)
{


 DIR           *d;
 DIR 			*d2;
 struct dirent *dir;
 d = opendir(".");
 d2 = opendir(".");
 int i; 
 int dir_count = 3;
 int arr_place = 0;   
 char **directory; 
 
 char* arg_0 = argv[0];
 char* arg_1 = argv[1]; 
 char* arg_2 = argv[2];
 char* arg_3; 
 
 /*+++++APPEND VARIABLES++++++*/

	char plain_new_line[] = "\x0A"; 
	
	char *archive = argv[2];
	char *file_to_add = malloc(sizeof(char)*15); 
	char *file_head = malloc(sizeof(char) * 61);
	
	char *files_array = malloc(sizeof(char) * (15 * (argc - 2))); 
	char *new_line = plain_new_line; 
	
	char* read_buffer; 
	
	char *buff = malloc(120000); 

	
	struct stat write_stat;
	struct stat read_stat; 
	
	int write_handle; 
	int read_handle;
	int arch_size;
	int file_size; 
	int bytes_writ = 0;
	int bytes_read; 
	
	off_t write_spot; 
	

/*+++++++END APPEND VARIABLES+++++*/

 
 if(argc <= 2)
 {
 	printf("Error: Require archive name\nExiting\n"); 
	exit(1); 
 }
  
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    
      
	  if(dir->d_type == DT_REG)
	  {
	  	if(strstr(dir->d_name, ".") != 0) 
		{
	  		dir_count++; 
		}
	  }
	
	 closedir(d);  
  }


  directory = malloc(sizeof(char) * (15 * dir_count));

  arr_place = 3; 
  
   if (d2)
  {
    while ((dir = readdir(d2)) != NULL)
    {
      
	  if(dir->d_type == DT_REG)
	  {
	  	if(strstr(dir->d_name, ".") != 0) 
		{
	  		if(arr_place == 3)
			{
				arg_3 = dir->d_name; 
			}
			
			directory[arr_place] = dir->d_name; 
	  		arr_place++;
		}
	  }
    }

    closedir(d);
  }
  
  
  	directory[0] = arg_0;
  	directory[1] = arg_1;
  	directory[2] = arg_2;
	directory[3] = arg_3; 
/*
	for(i = 0; i < arr_place; i++)
	{
	printf("In directory array: %s \n", directory[i]);
	
	}


	
/*	  append(arr_place, directory);*/
	
	/*++++++++++++++++++++HERE STARTS THE COPY OF APPEND++++++++++++++++++++*/
 		

	char header_buff[9] = "!<arch>\n";
	
		/*open a file to write.*/
	write_handle = open(archive, O_CREAT | O_RDWR, S_IWRITE | S_IREAD); /*returns -1 if the file does not exist */

	
		/*figure out whether that file has stuff in it already*/ 
		
	stat(argv[2], &write_stat); 	  	  	  	  	  	  /*creates a struct of file stats called write_stat */
	arch_size = (int) write_stat.st_size; 	  	  	  	  /*finds out the size of the entire archive using stat, */

	
	if(arch_size == 0)
	{


		bytes_writ = write(write_handle, header_buff, 8); 
		if(bytes_writ == -1)
		{
		
			printf("Error: %s \n", strerror(errno)); 
		}
		printf("myar: creating %s \n", argv[2]); 
	
	}
	
	
	stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
	arch_size = (int) write_stat.st_size; 	  	  
	write_spot = arch_size; 
	
/*	  
	for(i = 0; i < argc - 3; i++) 
	{
		printf("Argument %d is %s \n", i+3, argv[i+3]); 
		
		for(j = 0; j < 15; j++) 
		{
			
		}
	
	
	}
	printf("Printing Out the arguments:\n\n%s, %s, %s\n\n", argv[3], argv[4], argv[5]); 
	
	*/
	i = 3; 
	
	while(i < arr_place)
	{ 
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
  	 	 arch_size = (int) write_stat.st_size; 	   	   
		write_spot = arch_size; 
		/*printf("Current Arch Size: %d \n", arch_size); */
	   	   	   	   	   	   	   	   	   	   	   	   	   
		file_to_add = directory[i];
		
		i++;
	
		
		read_handle = open(file_to_add, O_RDONLY, S_IREAD); 			
		
		if (read_handle == -1) 
		{
			printf("Trying to open %s", file_to_add); 
			printf("Error opening file to read. Exiting\n"); 
			exit(1); 
			
		}
		
		stat(file_to_add, &read_stat);						/*the statistics for the file to be added*/	   	   	   	   	   	   	   	   	   
  	 	file_size = (int) read_stat.st_size; 				/*the size of the file that is being added to the archive*/ 
		
		/*printf("Current File Size: %d \n", file_size); */
	
		
	 	 statify(read_stat, file_to_add, file_head);  	    /*creates the archive header for the file being added */	
		
		/*printf("File head: %s \n", file_head);   */   
	
		
	
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		
		
		bytes_writ = pwrite(write_handle, file_head, 60, write_spot); 	/*writes the 60 bytes of file header to the archive*/
		/*printf("bytes writ %d\n", bytes_writ); */	 
		
		if(bytes_writ == -1)
		{
		
			printf("Error: %s \n", strerror(errno)); 
		}
		
		bytes_read = 0; 
		bytes_writ = 0; 
		
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		
		/*write_spot = write_spot + bytes_writ; 	  	  	  	  	  	  /*updates the place to start writing by adding the size of the header */
		/*printf("File Size: %d\n", file_size); */
		
/* 	   	   	   	   
		buff = malloc(sizeof(file_size + 1));	  	  	  	  	  	  	  /*creates a buffer for the file data*/
			  	  
		bytes_read = read(read_handle, buff, file_size); 				/*reads the contents of the file into the buffer*/
		
		/*printf("bytes read %d\n", bytes_read); */	   
	
		bytes_writ = pwrite(write_handle, buff, file_size, write_spot); 	/*writes the file data to the archive, after the header */
		
		if(bytes_writ == -1)
		{
		
			printf("Error: %s \n", strerror(errno)); 
		}
	
	
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */	  	  	  	  	  	 
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		
		/*printf("Current Arch Size: %d \n", arch_size); */
		
		
		if(file_size % 2 != 0) 
		{
			bytes_writ = pwrite(write_handle, new_line, 1, write_spot); /*writes the file data to the archive, after the header */
			
			if(bytes_writ == -1)
			{
			
				printf("Error: %s \n", strerror(errno)); 
			}
			
			/*printf("New line attempted inserted, bytes written %d File size %d\n", bytes_writ, file_size); */
			
		}
		/*printf("bytes writ %d\n", bytes_writ); 	
		
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
		
	}
	
	close(write_handle); 
	close(read_handle); 



}

/****************************************
***************************************
	EXTRACT
****************************************
*****************************************/

void extract(int argc, char **argv)
{

	char *archive = argv[2];
	char *buff;
	char *file_buff; 
	
	struct stat fileStat;
	
	time_t raw_time; 	 
 
	int i = 0;
	int j = 0;
	
	int prev_bytes; 
	int handle; 
	int arch_size; 
	int bytes_read; 
	int offset; 
	int file_size; 
	int file_size_len; 
	int keep_on = (argc - 3);
	
	int raw_time_int; 
	
	char file_size_arr[11];
	char raw_time_arr[15];  
	char file_name[16]; 
	char adj_file_size_arr[7]; 

	 

	handle = open(archive, O_RDONLY, S_IREAD); 	/* creates a file number identifier called handle*/
	
	if (handle == -1) 
	{
		printf("Error opening file. Exiting\n"); 
		exit(1); 
		
	}
	
	stat(argv[2], &fileStat); 							/*creates a struct of file stats called fileStat */
	arch_size = (int) fileStat.st_size; 				/*finds out the size of the entire archive using stat, calls it arch_size) */
	buff = malloc(sizeof(arch_size + 1)); 				/*allocates a buffer (that will hold the archive data for manipulation, of the size of the archive */
	file_buff = malloc(sizeof(arch_size + 1)); 
	
	bytes_read = read(handle, buff, arch_size); 		/*reads the data of the archive into the buffer buff, and returns the number of bytes read */
	
	prev_bytes = 8; 									/*this is the number of bytes of the magic header at the top of the file. All offsets are based on the previous bytes */	   



														/*now we are navigating through the archive file */ 

	
while(prev_bytes < (arch_size -50))
{ 



	
/* FILE SIZE */	   
	offset = prev_bytes + 48;							/*the offset for the file size is 48 + the previous bytes */ 
	
	
	for(i = 0; i < 10; i++)
	{
			file_size_arr[i] = ' '; 	
	}	   	   	   	   	   	   
	
	j = 0; 	
	
	for (i = offset; i < (offset + 7); i++) 	 	 	 /*finds the file size stored at the offset, and coverts it into an int */ 
	{	 
		if(buff[i] != ' ')
		{
			file_size_arr[j] = buff[i];  
			j++;	
		}
	}

	file_size = atoi(file_size_arr);
	
	/*printf("File Size: %d ", file_size);*/ 
	


/* FILE NAME */
	offset = prev_bytes; 								/*the offset for the file name is 0, so it is just = to previous bytes */
	j = 0;
	for (i = offset; i < offset + 16; i++)
	{
		if(buff[i] != '/' && buff[i] != ' '  && buff[i] != '\n')
		{
			printf("%c", buff[i]);
			file_name[j] = buff[i];
			j++;
		}
	}
	
		/*printf("File_name: %s", file_name); */
		printf("\n");

	
		if(keep_on == 0)
			{
				/*printf("Making file %s\n", file_name);*/
				make_file(argc, argv, offset, file_name, file_size, file_buff, buff);
		
			}
		
			if(keep_on > 0)
			{
				for(i = 3; i < argc; i++) 
				{
					if(strcmp(file_name, argv[i]) == 0)
					{
	
						/*printf("Matched %s and %s\n", file_name, file_name);*/
						
						make_file(argc, argv, offset, file_name, file_size, file_buff, buff);
						/*keep_on--; */
					}
				}
			}



	
	prev_bytes = (prev_bytes + 60 + file_size); 
	
	/*printf("/nPrev-bytes %d / Arch-size %d", prev_bytes, arch_size); */
	
	
}
	
	close(handle); 	   	   
	
	

}
/*************************************************
***************************************************
MAKE FILE
***************************************************
***************************************************/


int make_file(int argc, char **argv, int prev_bytes, char *file_name, int file_size, char *file_size_buff, char *buff) 
{
	int i, j; 	  

	char *archive = argv[2]; 
	
	int arch_handle; 
	int txt_handle;
	
	struct stat arch_stat; 
	struct stat txt_stat; 

	int arch_size;  
	int bytes_read;
	int bytes_writ;
	
	int offset = prev_bytes;
	
	int location;


	arch_handle = open(archive, O_RDWR, S_IWRITE | S_IREAD); /*returns -1 if the file does not exist */

	txt_handle = open(file_name, O_CREAT | O_RDWR, S_IWRITE | S_IREAD); /*returns -1 if the file does not exist */

	
/*	  printf("File size %d\n", file_size); */
  
	
	location = offset + 60; 
	
	j = 0;
	
	for(i = location; i < location + file_size; i++) 
	{
		file_size_buff[j] = buff[i]; 
		j++;
	}
	

	bytes_writ = write(txt_handle, file_size_buff, file_size);
	if(bytes_writ == -1)
	{
	
		printf("Error: %s \n", strerror(errno)); 
	}
	return 0;
	

}

/**********************************************
***********************************************
VERBOSE TABLE
************************************************
***********************************************/

void verbose_table(int argc, char **argv)
{
	

	print_time(1234567890);  /*Needs to be seeded... Don't know why...?*/
	char *archive = argv[2];
	char *buff;
	
	struct stat fileStat;
	
	time_t raw_time; 	 
 
	int i = 0;
	int j = 0;
	
	int prev_bytes; 
	int handle; 
	int arch_size; 
	int bytes_read; 
	int offset; 
	int file_size; 
	int file_size_len; 
	int oct_perm; 
	
	int raw_time_int; 
	
	char permissions[9];
	char string_perm[9]; 
	char file_size_arr[11];
	char raw_time_arr[15];  
	char formatted_time[35]; 
	char adj_file_size_arr[7]; 


	
	 

	handle = open(archive, O_RDONLY, S_IREAD); 	/* creates a file number identifier called handle*/
	
	if (handle == -1) 
	{
		printf("Error opening file. Exiting\n"); 
		exit(1); 
		
	}
	
	stat(argv[2], &fileStat); 							/*creates a struct of file stats called fileStat */
	arch_size = (int) fileStat.st_size; 				/*finds out the size of the entire archive using stat, calls it arch_size) */
	buff = malloc(sizeof(arch_size + 1)); 				/*allocates a buffer (that will hold the archive data for manipulation, of the size of the archive */
	bytes_read = read(handle, buff, arch_size); 		/*reads the data of the archive into the buffer buff, and returns the number of bytes read */
	
	prev_bytes = 8; 									/*this is the number of bytes of the magic header at the top of the file. All offsets are based on the previous bytes */	   



														/*now we are navigating through the archive file */ 

	
while(prev_bytes < (arch_size -50))
{ 

/*FILE MODE*/

	j = 0; 
	offset = prev_bytes + 40;

	
	for(i = 0; i < 8; i++)
	{
			permissions[i] = ' '; 	  
	}	   	   	   	   	   	   
	
	j = 0; 	
	
	for (i = offset; i < (offset + 7); i++) 	 	 	 /*finds the file size stored at the offset, and coverts it into an int */ 
	{	 
		if(buff[i] != ' ')
		{
			permissions[j] = buff[i];  
			j++;	
		}
	}
	
	oct_perm = atoi(permissions);
	
	/*printf("%d", oct_perm); */
	
	printf("%o", oct_perm); 
	printf(" "); 	    



/*OWNER ID*/
	offset = prev_bytes + 28; 	 	 	 	 	 	 	 	 /*the offset for the owner ID is 28 */

	for (i = offset; i < offset + 6; i++)
	{
		if(buff[i] != '/' && buff[i] != ' ' && buff[i] != '\n')
		{
			printf("%c", buff[i]);
		}
	}
	
	printf("/"); 

	
/* GROUP ID */	  
	offset = prev_bytes + 34; 	 	 	 	 	 	 	 	 /*the offset for the group ID is 34 */
	for (i = offset; i < offset + 6; i++)
	{
		if(buff[i] != '/' && buff[i] != ' '  && buff[i] != '\n')
		{
			printf("%c", buff[i]);
		}
	}
	
	
/* FILE SIZE */	   
	offset = prev_bytes + 48;							/*the offset for the file size is 48 + the previous bytes */ 
	
	
	for(i = 0; i < 10; i++)
	{
			file_size_arr[i] = ' '; 	
	}	   	   	   	   	   	   
	
	j = 0; 	
	
	for (i = offset; i < (offset + 7); i++) 	 	 	 /*finds the file size stored at the offset, and coverts it into an int */ 
	{	 
		if(buff[i] != ' ')
		{
			file_size_arr[j] = buff[i];  
			j++;	
		}
		
		file_size = atoi(file_size_arr);
	}
	
	file_size_len = len_int(file_size);  
	
	for (i = 0; i < (7 - file_size_len); i++)
	{
		adj_file_size_arr[i] = ' ';
	}
	
	j = 0; 
	for (i = (7 - file_size_len); i < 7; i++) 
	{
		adj_file_size_arr[i] = file_size_arr[j]; 
		j++;
	}
	
	printf(" %s ", adj_file_size_arr);

	
/* DATE/TIME */
	

	offset = prev_bytes + 16; 
	j = 0; 
	
	for (i = offset; i < (offset + 12); i++) 	  	  	  /*finds the raw time stored at the offset, and coverts it into a formatted date */
	{	 
	
			raw_time_arr[j] = buff[i];  

			j++;
	
	}
	
	raw_time_arr[j] = '\0'; 
	
	
	raw_time_int = atoi(raw_time_arr);


	
	print_time(raw_time_int); 
	
	
	


/* FILE NAME */
	offset = prev_bytes; 								/*the offset for the file name is 0, so it is just = to previous bytes */

	for (i = offset; i < offset + 16; i++)
	{
		if(buff[i] != '/' && buff[i] != ' '  && buff[i] != '\n')
		{
			printf("%c", buff[i]);
		}
	}
	
		printf("\n");

	
	prev_bytes = (prev_bytes + 60 + file_size); 
	
	/*printf("/nPrev-bytes %d / Arch-size %d", prev_bytes, arch_size); */
	
	
}
	
	close(handle); 

}

void print_time(int epoch_time)
{
/*
	printf("I'm in the function, bitches");
*/


	time_t epoch; 
    
	struct tm ts;
    
	char buf[80];
    
	
	epoch = epoch_time;
	
	
	ts = *localtime(&epoch);
   
	strftime(buf, sizeof(buf), "%b %d %H:%M %Y", &ts);
    
	
	if( epoch_time != 1234567890)
	{
	printf(" %s ", buf);
	}
	

}
/*****************************************
******************************************
TABLE 
******************************************
******************************************/



void table(int argc, char **argv)
{

	
	
	char *archive = argv[2];
	char *buff;
	
	struct stat fileStat;
	
	 
	int prev_bytes; 
	int handle; 
	int arch_size; 
	int bytes_read; 
	int offset;
	char file_size_arr[10]; 
	int file_size;  
	int i = 0;
	int j = 0; 
	 

	handle = open(archive, O_RDONLY, S_IREAD); 	/* creates a file number identifier called handle*/
	
	if (handle == -1) 
	{
		printf("Error opening file. Exiting\n"); 
		exit(1); 
		
	}
	
	stat(argv[2], &fileStat); 							/*creates a struct of file stats called fileStat */
	arch_size = (int) fileStat.st_size; 				/*finds out the size of the entire archive using stat, calls it arch_size) */
	buff = malloc(sizeof(arch_size + 1)); 				/*allocates a buffer (that will hold the archive data for manipulation, of the size of the archive */
	bytes_read = read(handle, buff, arch_size); 		/*reads the data of the archive into the buffer buff, and returns the number of bytes read */
	
	prev_bytes = 8; 									/*this is the number of bytes of the magic header at the top of the file. All offsets are based on the previous bytes */	   

														/*now we are navigating through the archive file */ 
	
	while(prev_bytes < (arch_size -50))
	{
	
	
		offset = prev_bytes; 								/*the offset for the file name is 0, so it is just = to previous bytes */
	
		for (i = offset; i < offset + 16; i++)
		{
			while(buff[i] != '/' && buff[i] != ' ' && buff[i] != '\n')
			{
				printf("%c", buff[i]);
				i++;
			}
		}
		
		
		offset = prev_bytes + 48;							/*the offset for the file size is 48 + the previous bytes */ 
			 	 	 	 	 	 
		
		for(i = 0; i < 10; i++)
		{
			file_size_arr[i] = ' '; 	
		}
		
		j = 0; 
		
		for (i = offset; i < (offset + 10); i++) 	  	  	  /*finds the file size stored at the offset, and coverts it into an int */ 
		{	 
			if(buff[i] != ' ')
			{
				file_size_arr[j] = buff[i];  
				j++;	
			}
			
			/*printf("File size arr: .%s.\n", file_size_arr); */
			
			file_size = atoi(file_size_arr); 
	
		
		}
		
		/*printf("\nFile Size: %d", file_size);*/ 
	
			printf("\n");
		
		
		prev_bytes = (prev_bytes + 60 + file_size); 
		
		
	
		
	}
	
	/*printf("\nExiting with Prev-bytes %d / Arch-size %d", prev_bytes, arch_size);*/
	close(handle); 

}
/************************************************
*************************************************
APPEND FUNCTION
**************************************************
**************************************************/


void append(int argc, char **argv)
{
	
	int i;
	
	
	char plain_new_line[] = "\x0A"; 
	
	char *archive = argv[2];
	char *file_to_add = malloc(sizeof(char)*15); 
	char *buff;
	char *file_head = malloc(sizeof(char) * 61);
	
	char *files_array = malloc(sizeof(char) * (15 * (argc - 2))); 
	char *new_line = plain_new_line; 
	
	
	
	
	struct stat write_stat;
	struct stat read_stat; 
	
	int write_handle; 
	int read_handle;
	int arch_size;
	int file_size; 
	int bytes_writ = 0;
	int bytes_read; 
	 
	
	off_t write_spot; 
	
	char header_buff[9] = "!<arch>\n";
	
		/*open a file to write.*/
	write_handle = open(archive, O_CREAT | O_RDWR, S_IWRITE | S_IREAD); /*returns -1 if the file does not exist */

	
		/*figure out whether that file has stuff in it already*/ 
		
	stat(argv[2], &write_stat); 	  	  	  	  	  	  /*creates a struct of file stats called write_stat */
	arch_size = (int) write_stat.st_size; 	  	  	  	  /*finds out the size of the entire archive using stat, */

	
	if(arch_size == 0)
	{


		bytes_writ = write(write_handle, header_buff, 8); 
		if(bytes_writ == -1)
		{
		
			printf("Error: %s \n", strerror(errno)); 
		}
		printf("myar: creating %s \n", argv[2]); 
	
	}
	
	
	stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
	arch_size = (int) write_stat.st_size; 	  	  
	write_spot = arch_size; 
	
	/*
	for(i = 0; i < argc - 3; i++) 
	{
		printf("Argument %d is %s \n", i+3, argv[i+3]); 
		
		for(j = 0; j < 15; j++) 
		{
			
		}
	
	
	}
	printf("Printing Out the arguments:\n\n%s, %s, %s\n\n", argv[3], argv[4], argv[5]); 
	
	*/
	i = 3; 
	
	while(i < argc)
	{
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		/*printf("Current Arch Size: %d \n", arch_size); */
	   	   	   	   	   	   	   	   	   	   	   	   	   
		file_to_add = argv[i];
		
		i++;
	
		
		read_handle = open(file_to_add, O_RDONLY, S_IREAD); 			
		
		if (read_handle == -1) 
		{
			printf("Trying to open %s", file_to_add); 
			printf("Error opening file to read. Exiting\n"); 
			exit(1); 
			
		}
		
		stat(file_to_add, &read_stat);						/*the statistics for the file to be added*/	   	   	   	   	   	   	   	   	   
		file_size = (int) read_stat.st_size; 				/*the size of the file that is being added to the archive*/ 
		
		/*printf("Current File Size: %d \n", file_size); */
	
		
		statify(read_stat, file_to_add, file_head);  	   /*creates the archive header for the file being added */	
		
		/*printf("File head: %s \n", file_head);   */   
	
		
	
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		
		
		bytes_writ = pwrite(write_handle, file_head, 60, write_spot); 	/*writes the 60 bytes of file header to the archive*/
		/*printf("bytes writ %d\n", bytes_writ); */	 
		
		if(bytes_writ == -1)
		{
		
			printf("Error: %s \n", strerror(errno)); 
		}
		
		bytes_read = 0; 
		bytes_writ = 0; 
		
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		
		/*write_spot = write_spot + bytes_writ; 	  	  	  	  	  	  /*updates the place to start writing by adding the size of the header */
						
		buff = malloc(sizeof(file_size + 1)); 							/*creates a buffer for the file data*/
			  	  
		bytes_read = read(read_handle, buff, file_size); 				/*reads the contents of the file into the buffer*/
		
		/*printf("bytes read %d\n", bytes_read); */	   
	
		bytes_writ = pwrite(write_handle, buff, file_size, write_spot); /*writes the file data to the archive, after the header */
		
		if(bytes_writ == -1)
		{
		
			printf("Error: %s \n", strerror(errno)); 
		}
	
	
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */	  	  	  	  	  	 
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		
		/*printf("Current Arch Size: %d \n", arch_size); */
		
		
		if(file_size % 2 != 0) 
		{
			bytes_writ = pwrite(write_handle, new_line, 1, write_spot); /*writes the file data to the archive, after the header */
			
			if(bytes_writ == -1)
			{
			
				printf("Error: %s \n", strerror(errno)); 
			}
			
		/*	  printf("New line attempted inserted, bytes written %d File size %d\n", bytes_writ, file_size); */
			
		}
		/*printf("bytes writ %d\n", bytes_writ); 	
		
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
	
		
	}
	
	close(write_handle); 
	close(read_handle); 



}

/********************************************
*********************************************
DELETE
*********************************************
*********************************************/ 
void delete(int argc, char **argv)
{


	char *archive = argv[2]; /**/			/**/ /* = extract function */
	char *buff;			/**/
	char *file_buff; 	/**/
	char file_name_list[100][15];	 	 	 /*this is going to cause an error if more than 100 files are in the archive */ 
	char new_ar_list[100][15];
	char *add_file_name; 
	
		
	struct stat arch_stat;	/**/  
 
	int i = 0;
	int j = 0;
	int k = 0;
	
	int prev_bytes;   /**/
	int handle; 	/**/
	int arch_size;  /**/
	int bytes_read; /**/
	int offset; 	/**/
	int file_size; 	/**/
	int keep_on = (argc - 3); /**/
	int list_ctr = 0;
	int delete_me = 0; 
	
	char file_size_arr[11]; 
	char file_name[16]; 
	
	/************Append Variables v / Extract Variables ^ **************/
	
	char plain_new_line[] = "\x0A"; 
	
	char *file_to_add = malloc(sizeof(char)*15); 
	char *file_head = malloc(sizeof(char) * 61);
	
	char *files_array = malloc(sizeof(char) * (15 * (argc - 2))); 
	char *new_line = plain_new_line; 
	
	
	
	
	struct stat write_stat;
	struct stat read_stat; 
	
	int write_handle; 
	int read_handle;
	int bytes_writ = 0;
	 
	off_t write_spot; 
	
	char header_buff[9] = "!<arch>\n";


	if( argc <= 3)
	{
		printf("Error: Too few arguments/n"); 
		exit(1); 
	}
	
	/****** Extract Function (with added name list)***********/
	


	 

	handle = open(archive, O_RDONLY, S_IREAD); 	/* creates a file number identifier called handle*/
	
	if (handle == -1) 
	{
		printf("Error opening file. Exiting\n"); 
		exit(1); 
		
	}
	
	stat(argv[2], &arch_stat); 	 	 	 	 	 	 	 /*creates a struct of file stats called arch_stat */
	arch_size = (int) arch_stat.st_size; 	 	 	 	 /*finds out the size of the entire archive using stat, calls it arch_size) */
	buff = malloc(sizeof(arch_size + 1)); 				/*allocates a buffer (that will hold the archive data for manipulation, of the size of the archive */
	file_buff = malloc(sizeof(arch_size + 1)); 
	
	bytes_read = read(handle, buff, arch_size); 		/*reads the data of the archive into the buffer buff, and returns the number of bytes read */
	
	prev_bytes = 8; 									/*this is the number of bytes of the magic header at the top of the file. All offsets are based on the previous bytes */	   



														/*now we are navigating through the archive file */ 

	
while(prev_bytes < (arch_size -50))
{ 



	
/* FILE SIZE */	   
	offset = prev_bytes + 48;							/*the offset for the file size is 48 + the previous bytes */ 
	
	
	for(i = 0; i < 10; i++)
	{
			file_size_arr[i] = ' '; 	
	}	   	   	   	   	   	   
	
	j = 0; 	
	
	for (i = offset; i < (offset + 7); i++) 	 	 	 /*finds the file size stored at the offset, and coverts it into an int */ 
	{	 
		if(buff[i] != ' ')
		{
			file_size_arr[j] = buff[i];  
			j++;	
		}
	}

	file_size = atoi(file_size_arr);
	
	/*printf("File Size: %d ", file_size);*/ 
	


/* FILE NAME */
	offset = prev_bytes; 								/*the offset for the file name is 0, so it is just = to previous bytes */
	j = 0;
	for (i = offset; i < offset + 15; i++)
	{
	
		
		if(buff[i] != '/' && buff[i] != ' '  && buff[i] != '\n')
		{
			
			file_name[j] = buff[i];
			file_name_list[list_ctr][j] = buff[i];
			printf("%c", file_name_list[list_ctr][j]);
			
			j++;
		}
		
		 
	}
		file_name_list[list_ctr][j] = '\0';
		list_ctr++;
	
		 
		 
	
		/*printf("File_name: %s", file_name); */
		printf("\n");


		/*
		make_file(argc, argv, offset, file_name, file_size, file_buff, buff);
		*/



	
	prev_bytes = (prev_bytes + 60 + file_size); 
	
	/*printf("/nPrev-bytes %d / Arch-size %d", prev_bytes, arch_size); */
	
	
}

/*	  printf("\nDid anything print??\n");
	
	for(j = 0; j < list_ctr; j++)
	{
	
		printf("%s \n", file_name_list[j]);
		
		
	}*/
	
	close(handle); 	   	

	
  
	
	
	/********** Delete Archive **************/ 
	
	remove(archive); 
	
	/************ Compare name list with delete list **********/ 
	
	for(i = 0; i < list_ctr; i++)
	{
	
		for(j = 3; j < argc; j++)
		{
			if(!(strcmp(file_name_list[i], argv[j])))
			{
				delete_me++; 
				/*printf("%s = %s", file_name_list[i], argv[j]); */
			}
		}
		
		if(delete_me == 0)
		{
			strcpy(new_ar_list[k], file_name_list[i]); 
			k++; 
		}
		
		delete_me = 0; 
	
	}
	
	/*printf("New Ar List: \n"); 
	for(i = 0; i < k; i++)
	{
		printf("%s\n", new_ar_list[i]); 	
	}*/
	
	/*********** Append Function (for difference between lists) **********/ 
	

	
		/*open a file to write.*/
	write_handle = open(archive, O_CREAT | O_RDWR, S_IWRITE | S_IREAD); /*returns -1 if the file does not exist */

	
		/*figure out whether that file has stuff in it already*/ 
		
	stat(argv[2], &write_stat); 	  	  	  	  	  	  /*creates a struct of file stats called write_stat */
	arch_size = (int) write_stat.st_size; 	  	  	  	  /*finds out the size of the entire archive using stat, */

	
	if(arch_size == 0)
	{


		bytes_writ = write(write_handle, header_buff, 8); 
		if(bytes_writ == -1)
		{
		
			printf("Error: %s \n", strerror(errno)); 
		}
		printf("myar: creating %s \n", argv[2]); 
	
	}
	
	
	stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
	arch_size = (int) write_stat.st_size; 	  	  
	write_spot = arch_size; 
	
	/*
	for(i = 0; i < argc - 3; i++) 
	{
		printf("Argument %d is %s \n", i+3, argv[i+3]); 
		
		for(j = 0; j < 15; j++) 
		{
			
		}
	
	
	}
	printf("Printing Out the arguments:\n\n%s, %s, %s\n\n", argv[3], argv[4], argv[5]); 
	
	*/
	i = 0; 
	
	while(i < k)
	{
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		/*printf("Current Arch Size: %d \n", arch_size); */
	   	   	   	   	   	   	   	   	   	   	   	   	   
		file_to_add = new_ar_list[i];
		
		i++;
	
		
		read_handle = open(file_to_add, O_RDONLY, S_IREAD); 			
		
		if (read_handle == -1) 
		{
			printf("Trying to open %s", file_to_add); 
			printf("Error opening file to read. Exiting\n"); 
			exit(1); 
			
		}
		
		stat(file_to_add, &read_stat);						/*the statistics for the file to be added*/	   	   	   	   	   	   	   	   	   
		file_size = (int) read_stat.st_size; 				/*the size of the file that is being added to the archive*/ 
		
		/*printf("Current File Size: %d \n", file_size); */
	
		
		statify(read_stat, file_to_add, file_head);  	   /*creates the archive header for the file being added */	
		
		/*printf("File head: %s \n", file_head);   */   
	
		
	
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		
		
		bytes_writ = pwrite(write_handle, file_head, 60, write_spot); 	/*writes the 60 bytes of file header to the archive*/
		/*printf("bytes writ %d\n", bytes_writ); */	 
		
		if(bytes_writ == -1)
		{
		
			printf("Error: %s \n", strerror(errno)); 
		}
		
		bytes_read = 0; 
		bytes_writ = 0; 
		
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		
		/*write_spot = write_spot + bytes_writ; 	  	  	  	  	  	  /*updates the place to start writing by adding the size of the header */

			  	  
		bytes_read = read(read_handle, buff, file_size); 				/*reads the contents of the file into the buffer*/
		
		/*printf("bytes read %d\n", bytes_read); */	   
	
		bytes_writ = pwrite(write_handle, buff, file_size, write_spot); /*writes the file data to the archive, after the header */
		
		if(bytes_writ == -1)
		{
		
			printf("Error: %s \n", strerror(errno)); 
		}
	
	
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */	  	  	  	  	  	 
		arch_size = (int) write_stat.st_size; 	  	  
		write_spot = arch_size; 
		
		/*printf("Current Arch Size: %d \n", arch_size); */
		
		
		if(file_size % 2 != 0) 
		{
			bytes_writ = pwrite(write_handle, new_line, 1, write_spot); /*writes the file data to the archive, after the header */
			
			if(bytes_writ == -1)
			{
			
				printf("Error: %s \n", strerror(errno)); 
			}
			
			/*printf("New line attempted inserted, bytes written %d File size %d\n", bytes_writ, file_size); */
			
		}
		/*printf("bytes writ %d\n", bytes_writ); 	
		
		stat(argv[2], &write_stat); 	  	  	  	  	  		/*creates a struct of file stats called write_stat */
	
		
	}
	
	close(write_handle); 
	close(read_handle); 	   
	
	
	




	
	/*************** Remove all .txt files from name list ***************/ 



}



void statify(struct stat read_stat, char *file_to_add, char *header)
{
	int i;
	
	char c_timestamp[13];
	char c_owner_id[6];
	char c_group_id[6];
	char c_file_mode[8]; 
	char c_file_size[10]; 
	char c_file_name[16];
	
	char c_byte_size[10];
	
	char *c_ptr; 
	
	/*
	int timestamp;
	int owner_id;
	int group_id;
	int file_mode; 
	int file_size; 
	*/	
	
	int extra_space; 
	
	int timestamp = (int) read_stat.st_mtime;
	int owner_id = (int) read_stat.st_uid;
	int group_id = (int) read_stat.st_gid;
	int file_mode = (int) read_stat.st_mode;
	int file_size = (int) read_stat.st_size;
	
	/**************Go back and turn into loop if time *************/
	
	/*timestamp*/
	snprintf(c_timestamp, 11, "%d", timestamp); /*changed to 11, to try to get rid of special char */
	
	extra_space = 12 - strlen(c_timestamp); 
	
	for(i = 0; i < extra_space; i++)
	{
		strcat(c_timestamp, " "); 
	}
	
	/*byte size*/
	snprintf(c_byte_size, 10, "%d", file_size); 
	
	extra_space = 10 - strlen(c_byte_size); 
	
	for(i = 0; i < extra_space; i++)
	{
		strcat(c_byte_size, " "); 
	}
	
	/*group id */
	snprintf(c_owner_id, 6, "%d", owner_id); 
	
	extra_space = 6 - strlen(c_owner_id); 

	
	for(i = 0; i < extra_space; i++)
	{
		strcat(c_owner_id, " "); 
	}
	

	/*group id */
	snprintf(c_group_id, 6, "%d", group_id); 
	
	extra_space = 6 - strlen(c_group_id); 

	
	for(i = 0; i < extra_space; i++)
	{
		strcat(c_group_id, " "); 
	}
	
	/*file size*/ 
	snprintf(c_file_size, 10, "%d", file_size); 
	
	extra_space = 10 - strlen(c_file_size);  
	
	for(i = 0; i < extra_space; i++)
	{
		strcat(c_file_size, " "); 
	}
	
	
	/*file mode*/
	
	snprintf(c_file_mode, 8, "%o", file_mode); 
	
	extra_space = 8 - strlen(c_file_mode); 
	
	for(i = 0; i < extra_space; i++)
	{
		strcat(c_file_mode, " "); 
	}

	

	
	/*printf("IN FUNCTION File size: .%s. \n", c_file_size); */
	
	/*file name*/
	
	strcpy(c_file_name, file_to_add); 
	
	strcat(c_file_name, "/"); 
	
	extra_space = 16 - strlen(c_file_name);  
	
	for(i = 0; i < extra_space; i++)
	{
		strcat(c_file_name, " "); 
	}
	
	/**************************************************/
	
	
	strcpy(header, c_file_name); 
	strcat(header, c_timestamp); 
	strcat(header, c_owner_id); 
	strcat(header, c_group_id); 
	strcat(header, c_file_mode); 
	strcat(header, c_byte_size); 
	strcat(header, "\x60\x0A"); 
	

}

int len_int(int x)
{
	/*idea +structure from http://stackoverflow.com/questions/3068397/c-how-to-find-the-length-of-an-integer */
	
	if(x>=1000000000) return 10;
    if(x>=100000000) return 9;
    if(x>=10000000) return 8;
    if(x>=1000000) return 7;
    if(x>=100000) return 6;
    if(x>=10000) return 5;
    if(x>=1000) return 4;
    if(x>=100) return 3;
    if(x>=10) return 2;
    return 1;


}

