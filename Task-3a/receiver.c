#include "../lib/util.h"
#include<assert.h>

int threshold;


char bytefromtext(unsigned char* text)
{  
  char result = 0;
  for(int i = 0;i<8;i++)
  {
    if(text[i] == '1')
    {
      result |= (1 << (7-i));
    }
  }
  return result;
}

int mConvertFile(char filename[])
{
	FILE *fpTXT;
	FILE *fpJPG;
	unsigned char buf[8];
	int c;
	int j = 0;
	char temp;

	fpTXT = fopen("temp_receiver.txt","r");
	fpJPG = fopen(filename,"wb");


	if(!fpTXT)
	{
		printf("Unable to open file\n");
		return 1;
	}

	while((c = fgetc(fpTXT)) != EOF)
  {
    buf[j++] = c;
    if(j == 8)
    {
      
      //printf("%c\t", temp);
      if(temp == '1' || temp == '0')
      {
      	temp = bytefromtext(buf);
      	fputc(temp,fpJPG);
      }
      j = 0;
    }
  }


  fclose(fpTXT);
  fclose(fpJPG);
  //printf("Writing done\n");
  return 0;
}

void sync()
{
	uint64_t temp = rdtscp();
	while(!((temp%100000>10000) && (temp%100000<15000)))
	{ 
		temp = rdtscp();
	}
}

int read_data(unsigned int *map)
{
	int hit_count = 0, miss_count = 0;
	threshold = 100;
		
	sync();  

	CYCLES start_t = rdtscp();
	while(rdtscp() - start_t < 50000)
	{
		CYCLES temp = measure_one_block_access_time(map);
		if(temp > threshold)
		{
			miss_count++;
		}
		else 
		{
			hit_count++;
		}
	}
	if( miss_count<hit_count)
		return 1;
	else 
		return 0;
}

int read_pattern(unsigned int *map)
{
	uint32_t pattern = 0b101011;
	uint32_t pattern_2 = 0b11011011011011;
	uint32_t six_ones = 0b111111;
	uint32_t fourteen_ones= 0b11111111111111;
	uint32_t received = 0;
	uint32_t bit = 0;
	
	fflush(stdout); 
	while(1)
	{
		bit = read_data(map);
		received = ((uint32_t) received << 1) | bit;
		
		if((received & fourteen_ones) == pattern_2)
		{ 
			return 0;	//pattern_2 received
		}
		if((received & six_ones) == pattern)
		{ 
			return 1;	
		}
		
	}
}

int main(int argc, char **argv) 
{

	char received_file[61] = "received_";
	unsigned int content_length = 0;
	clock_t t_recv;
	double recv_time;
	double recv_rate;

	
	map_handle_t *handle;
	unsigned int *map;
	map = (unsigned int *) map_file("share_mem.txt",&handle);
	
	char bin_rcv[25000];
	unsigned int one_count = 0,i;
	unsigned int binary_msg_len = 0;
	
	if(read_pattern(map));	
	//Read data
	for(i = 0; ; i++)
	{
		binary_msg_len++;
		if(read_data(map))//hit
		{
			bin_rcv[i] = '1'; 			
 			one_count++;
 			if(one_count >= 8)
 			{
 				break;
 			}
		}
		else
		{	
 			bin_rcv[i] = '0';
 			one_count = 0;
		}
	}
	
	bin_rcv[binary_msg_len-8] = '\0';
	
	
	char *filename = binary_to_string(bin_rcv);
	strcat(received_file, filename);
	
	//To avoid creating invalid extension 
/*	int file_nme_len = strlen(received_file)-1;*/
/*	received_file[file_nme_len] = '.';*/
/*	received_file[++file_nme_len] = 'j';*/
/*	received_file[++file_nme_len] = 'p';*/
/*	received_file[++file_nme_len] = 'g';*/
/*	received_file[++file_nme_len] = '\0';*/
	
	//Code to correct the extension
	char *pointer = strchr(received_file, '.');
	int index = (int)(pointer - received_file);
	if(index != 0)
	{
		received_file[++index] = 'j';
		received_file[++index] = 'p';
		received_file[++index] = 'g';
		received_file[++index] = '\0';
	}
	else
	{
		int len = strlen(received_file);
		received_file[++len] = '.';
		received_file[++len] = 'j';
		received_file[++len] = 'p';
		received_file[++len] = 'g';
		received_file[++len] = '\0';
	}
		
	printf("\nCreated file name : %s",received_file);
		
	t_recv = clock();
	FILE *fp;
	fp  = fopen ("temp_receiver.txt", "w");
	char *msg;
		
	//Read file data
	while(1)
	{
		one_count = 0;
		binary_msg_len = 0;
		if(read_pattern(map) == 0)//End pattern received
		{
			printf("\nEnd pattern received\n");
			break;
		}	
		
		//Read data
			for(i = 0; ; i++)
			{
				binary_msg_len++;
				if(read_data(map))//hit
				{
					bin_rcv[i] = '1'; 			
		 			one_count++;
		 			if(one_count >= 8)
		 				break;
				}
				else
				{	
		 			bin_rcv[i] = '0';
		 			one_count = 0;
				}
		  }
	
		bin_rcv[binary_msg_len-8] = '\0';
		//msg = binary_to_string(bin_rcv);
		fputs(bin_rcv, fp);	
		//printf("\n%s\n",msg);
	}
	
	assert(mConvertFile(received_file) == 0);
	
	
	fseek(fp, 0L, SEEK_END); 
	content_length = ftell(fp);
	fclose (fp);
	
	t_recv = clock() - t_recv;
	
	recv_time = ((double) t_recv) / CLOCKS_PER_SEC;
	recv_rate = (double) (content_length * 8) / recv_time;

	printf("[Receiver] File (%s) received : %u bytes\n", received_file, content_length);
	printf("[Receiver] Time taken to receive file : %lf second\n", recv_time);
	printf("[Receiver] Data receiving rate : %lu bps\n", (unsigned long) recv_rate);
	
	return 0;

}
