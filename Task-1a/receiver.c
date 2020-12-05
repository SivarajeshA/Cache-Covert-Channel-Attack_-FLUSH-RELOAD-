#include "../lib/util.h"
#include<assert.h>

int threshold;

void sync()
{
	uint64_t temp=rdtscp();
	// Synchronization
	// Run loop infinitely(Control waiting), till a certain condition on rdstcp is met
    while(!(temp%100000>10000 && temp%100000<15000)){ 
    	temp=rdtscp();
    }
}

int read_data(unsigned int *map){
	
	int hit_count=0, miss_count=0;
	// This is to make Sender and Receiver to start work at same time	
	sync();    
	
    CYCLES start_t = rdtscp();
    while(rdtscp() - start_t < 50000){ // Accessing block for certain time period (50000=>rdtspc time period)
		CYCLES temp=measure_one_block_access_time(map);
		if(temp > threshold){
			miss_count++;
		}else {
			hit_count++;
		}
	}
	if( miss_count<hit_count)
		return 1;
	else 
		return 0;
}

void read_pattern(unsigned int *map)
{
	uint32_t pattern = 0b010100;//pattern which is used in sender side
	uint32_t six_ones = 0b111111;
	uint32_t received = 0;
	uint32_t bit=0;
	printf("\n***Waiting for sender to send data***\n");
	fflush(stdout); //to clear the output buffer and move the buffered data to console
	while(1){
		bit = read_data(map);
		received = ((uint32_t) received << 1) | bit;//appending each received bit
		if((received & six_ones) == pattern){ //Reading last 6 bits of received data and match with pattern
			break;	//pattern received
		}
	}
}

int main(int argc, char **argv) {

	
	double recv_time;
	double recv_rate;
	clock_t t_recv;
	
	// TODO: Establish your cache covert channel
	
	map_handle_t *handle;
	unsigned int *map;
	map = (unsigned int *) map_file("sample.txt",&handle);
	assert(map!=NULL);
	
	threshold=100;
	//uncomment the below line to calculate the threshold using calibration file
	//threshold=getThreshold();
	
	//Read pattern
	read_pattern(map);	
	
	char bin_rcv[1025];
	int one_count=0,i;
	int binary_msg_len = 0;
	t_recv = clock();
	//Read data
	for(i=0; i<1025; i++){
		binary_msg_len++;
		if(read_data(map))	//Cache hit
		{
			bin_rcv[i] = '1'; 			
 			
 			one_count++;
 				if(one_count>=8){	//To stop reading once receiver detected that sender is no longer sending data
 				break;
 			}
		}
		else	//Cache Miss
		{	
 			bin_rcv[i] = '0';
 			one_count=0;
		}
	}
	bin_rcv[binary_msg_len-8]='\0';	//To remove last 8 bits of 1 (useless bits)
	//printf("\nBinary Received: %s\n", bin_rcv);
	
	char *msg=binary_to_string(bin_rcv);

	t_recv = clock() - t_recv;
	int msg_len = strlen(msg);
	recv_time = ((double) t_recv) / CLOCKS_PER_SEC;
	recv_rate = (double) (msg_len * 8) / recv_time;

	printf("\n[Receiver] Received data : %s\n", msg);
	printf("[Receiver] Total data received : %u bytes\n", msg_len);
	printf("[Receiver] Time taken to receive data : %lf second\n", recv_time);
	printf("[Receiver] Data receiving rate : %lu bps\n", (unsigned long) recv_rate);
	
	return 0;

}

