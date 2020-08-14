#include "../lib/util.h"
#include<assert.h>

int threshold;

void sync()
{
	uint64_t temp=rdtscp();
	// Synchronization 
	// Run loop infinitely(Control waiting), till a certain condition is met
    while(!(temp%100000>10000 && temp%100000<15000)){ 
    	temp=rdtscp();
    }
}

int read_data(unsigned int *map){
	
	int hit_count=0, miss_count=0;
	threshold=100;
	
	// This is used to make Sender and Receiver read at same time	
	sync();    

    // Receive data for 50000 ms and take majority of hit and miss
    CYCLES start_t = rdtscp();
    while(rdtscp() - start_t < 50000)
    {
		CYCLES temp=measure_one_block_access_time(map);
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
	uint32_t pattern_2 = 0b010100;
	uint32_t six_ones = 0b111111;
	uint32_t received = 0;
	uint32_t bit=0;
	//printf("\n***Waiting for Sender to send data***\n");
	fflush(stdout); //to clear the output buffer and move the buffered data to console
	while(1)
	{
		bit = read_data(map);
		received = ((uint32_t) received << 1) | bit;//appending each received bit
		if((received & six_ones) == pattern)
		{ //Reading last 6 bits of received data and match with pattern
			return 1;	//pattern received
		}
		if((received & six_ones) == pattern_2)
		{ //Reading last 6 bits of received data and match with pattern
			return 0;	//pattern_2 received
		}
	}
}

int main(int argc, char **argv) {

	char received_file[61] = "received_";
	unsigned int content_length = 0;
	clock_t t_recv;
	double recv_time;
	double recv_rate;
	char *msg;

	//uncomment the below line to calculate the threshold using calibration file
	//threshold=getThreshold();
	
	// TODO: Establish your cache covert channel	
	map_handle_t *handle;
	unsigned int *map;
	map = (unsigned int *) map_file("share_mem.txt",&handle);
	assert(map!=NULL);
	printf("\nWaiting for sender to send data\n");
	
	/* TODO:
	 * Receive the filename from the sender process over covert channel
	 * Use filename buffer to store received file name
	*/
	//Read pattern
	if(read_pattern(map));
	
	char bin_rcv[2048];
	int one_count=0,i;
	int binary_msg_len = 0;
	
	
	//Read file name
	for(i=0; i<1025; i++)
	{
		binary_msg_len++;
		if(read_data(map))	//Cache hit
		{
			bin_rcv[i] = '1'; 			
 			one_count++;
 			if(one_count>=8)   //To stop reading once receiver detected that sender is no longer sending data
 			{
 				break;
 			}
		}
		else   //Cache Miss
		{	
 			bin_rcv[i] = '0';
 			one_count=0;
		}
	}
	
	bin_rcv[binary_msg_len-8]='\0';  //To remove last 8 bits of 1 (useless bits)
	//printf("\nBinary Received: %s\n", bin_rcv);
	
	char *filename=binary_to_string(bin_rcv);	
	strcat(received_file, filename);
	
	//To avoid creating invalid extension when bits are corrupted
	int file_nme_len =strlen(received_file)-4;
	received_file[file_nme_len]='.';
	received_file[++file_nme_len]='t';
	received_file[++file_nme_len]='x';
	received_file[++file_nme_len]='t';
	received_file[++file_nme_len]='\0';
	printf("\nCreated file name => %s",received_file);
		
	// TODO: Create a text file with the filename stored in received_file buffer in write mode.
	FILE *fp;	
    fp  = fopen (received_file, "w");
	assert(fp!=NULL);

	/* TODO:
	 * Receive file contents from the sender process over covert channel
	 * Wrtie them into the file opened earlier
	 * store the length of the file content in content_length variable
	*/
	t_recv = clock();
	while(1)//Read till whole file is received
	{
		one_count=0;
		binary_msg_len = 0;
		if(read_pattern(map)==0)//End pattern received
		{
			break;
		}	
		
		//Read file data
		for(i=0; ; i++)
		{
				binary_msg_len++;
				if(read_data(map))//hit
				{
					bin_rcv[i] = '1'; 			
		 			one_count++;
		 			if(one_count>=8)
		 			{
		 				break;
		 			}
				}
				else
				{	
		 			bin_rcv[i] = '0';
		 			one_count=0;
				}
		}
	
		bin_rcv[binary_msg_len-8]='\0';
		msg=binary_to_string(bin_rcv);
		fputs(msg, fp);	//Writing in file
		printf("\n%s",msg);	
	}
    fseek(fp, 0, SEEK_END);     
    content_length = ftell(fp); 	//To calculate file size
	fclose (fp);
	printf("\n\nReceived data added to the file => %s\n\n",received_file);	
	
	t_recv = clock() - t_recv;
	recv_time = ((double) t_recv) / CLOCKS_PER_SEC;
	recv_rate = (double) (content_length * 8) / recv_time;

	printf("[Receiver] File (%s) received : %u bytes\n", received_file, content_length);
	printf("[Receiver] Time taken to receive file : %lf second\n", recv_time);
	printf("[Receiver] Data receiving rate : %lu bps\n", (unsigned long) recv_rate);

	return 0;

}
