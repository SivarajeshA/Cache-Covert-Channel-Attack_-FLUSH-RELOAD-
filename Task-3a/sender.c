#include "../lib/util.h"
#include<assert.h>

/*
int mConvertFile(char filename[])
{
	FILE *fpJPG;
	FILE *fpTXT;
	int c;
	
	fpJPG = fopen(filename,"rb");
	fpTXT = fopen("temp_sender.txt","w"); //this is the file that will be transmitted

	assert(fpJPG != NULL);

	while((c = fgetc(fpJPG)) != EOF)
    { //Convert each char into 8 bit binary format
        for(int i = 0; i <= 7; i++)
        {
            if(c & (1 << (7-i))) 
            {
                fputc('1',fpTXT);
            }
            else
            {
                fputc('0',fpTXT);
            }
        }
    }

    fclose(fpJPG);
    fclose(fpTXT);

    return 0;
}
*/

	
void sync()
{
	uint64_t temp = rdtscp();
    while(!((temp % 100000 > 10000) && (temp % 100000 < 15000)))
    { 
    	temp = rdtscp();
    }
}

void send_data(int action, unsigned int *map) 
{
	sync();
    
	uint64_t start_t = rdtscp();
	if (action) 
	{
		while (rdtscp() < start_t + 50000) {}		
	} 
	else 
	{
		while (rdtscp() < start_t + 50000) 
		{ 
			clflush(map);
		}
	}
}

void send_pattern(unsigned int *map)
{
	//Sending 10101011 to inform receiver that sender is going to send data
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(1, map);
}

void send_pattern_2(unsigned int *map)
{
	//Sending 11011011011011 to inform receiver that sender has stopped sending data
	send_data(1, map);
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(1, map);
}


int main(int argc, char **argv) 
{

	char filename[100];
	unsigned int file_size = 0;
	clock_t t_send;
	double trans_time;
	double trans_rate;
	char read_buffer[300];
	int c;


	printf("Enter filename(with extension) to be sent: ");
	fgets (filename, 50, stdin);
	filename[strlen(filename)-1] = '\0';

	map_handle_t *handle;
	unsigned int *map;
	map = (unsigned int *) map_file("share_mem.txt",&handle);
	char *binary = string_to_binary(filename); 
		

	send_pattern(map);

	char *t = binary;
	
	while(*t)
	{
		if(*t == '0')
		{
			send_data(0, map);
		}
		else
		{
			send_data(1, map);
		}
		t++;
	}

	printf("\nFile name sent.\n");
	sleep(1);
	printf("\nSending file data...\n");
	//assert(mConvertFile(filename) == 0);

	t_send = clock();
	/*FILE *fp = fopen("temp_sender.txt", "r");
	assert(fp != NULL);

	while( fgets ( read_buffer, 300, fp ) != NULL ) 
	{ */
	FILE *fp;
	fp = fopen(filename,"rb");	
	assert(fp != NULL);	
	
	FILE *fp2;
	fp = fopen("","rb");	
	assert(fp != NULL);	
	while( fgets ( read_buffer, 100, fp ) != NULL ) 
	{//while( (c=fgetc(fp)) != NULL ) 
		printf("\n%s\n",read_buffer);
		binary = string_to_binary(read_buffer); 
		t = binary;
		printf("\n%s\n",t);
		send_pattern(map);
		while(*t)
		{
			if(*t == '0')
			{
				send_data(0, map);
			}
			else
			{
				send_data(1, map);
			}
			t++;			
		} 
		//printf("%s");
		sleep(1);
	}
	 
	send_pattern_2(map);
		
	printf("\nFile sent.\n");

	fseek(fp, 0L, SEEK_END); 
	file_size = ftell(fp); 
	fclose(fp);
		 
	t_send = clock() - t_send;
	trans_time = ((double) t_send) / CLOCKS_PER_SEC;
	trans_rate = (double) (file_size * 8) / trans_time;

	printf("[Sender] File (%s) data Sent : %u bytes\n", filename, file_size);
	printf("[Sender] Time to send file data : %lf second\n", trans_time);
	printf("[Sender] Data transmission rate : %lu bps\n", (unsigned long) trans_rate);

	return 0;
}
