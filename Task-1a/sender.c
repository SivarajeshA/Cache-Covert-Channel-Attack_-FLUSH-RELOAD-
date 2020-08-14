#include "../lib/util.h"
#include<assert.h>

//Action for bits
//1 => Do Nothing
//0 => Flush
	
void sync()
{
	uint64_t temp=rdtscp();
	// Synchronization 
	// Run loop infinitely(Control waiting), till a certain condition on rdtscp is met
	// Once condition is met while loop breaks, Sender and Receiver will start work at same time
    while(!(temp%100000>10000 && temp%100000<15000)){ // Waiting for rdtscp's last 5 digit value to fall btw the range 10000 and 15000
    	temp=rdtscp();
    }
    // Number of last digits from rdtscp taken should not be very less since the rdtscp increment rate is so faster 
	// the rdtscp value may never fall btw the smaller range so need to take enough digits to make sure that rdtscp value will  
	// fall btw the particular range, everytime.
    // Also the range can be changed as well => while(!(temp%100000>5555 && temp%100000<9999))
}

void send_data(int action, unsigned int *map) 
{
	// This is to make Sender and Receiver to start work at same time
	sync();
    
	uint64_t start_t = rdtscp();
	if (action) {
		// 1 => Do Nothing
		while (rdtscp() < start_t + 50000) {}		
	} else {
		// 0 => Repeatedly flushing in certain time period (50000=>rdtspc time period) 
		while (rdtscp() < start_t + 50000) { 
			clflush(map);
		}
	}
}
void send_pattern(unsigned int *map)
{
	//Sending 010110100 to inform receiver that sender is going to send data
	//Similar techinque which is used in TCP/IP header in networking
	send_data(0, map);
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(0, map);
	send_data(0, map);
}

int main(int argc, char **argv) 
{
	double trans_time;
	double trans_rate;

	// TODO: Establish your cache covert channel	
	map_handle_t *handle;
	unsigned int *map;
	map = (unsigned int *) map_file("share_mem.txt",&handle);
	assert(map!=NULL);

	printf("\nEnter message to send: ");
	char msg[51];
	fgets (msg, 51, stdin); //Each char is 1 byte
	unsigned int msg_len = strlen(msg);
	
	char *bin=string_to_binary(msg); 
		
	//Send pattern to receiver
	send_pattern(map);
	
	clock_t t_send = clock();
	
	// TODO: Transmit message over the cache covert channel
	char *t=bin;
	while(*t)
	{
		if(*t=='0')
		{
			send_data(0, map);
		}
		else
		{
			send_data(1, map);
		}
		t++;
	}

	t_send = clock() - t_send;
	trans_time = ((double) t_send) / CLOCKS_PER_SEC;
	trans_rate = (double) (msg_len * 8) / trans_time;

	printf("\n[Sender] Total data Send : %u bytes\n", msg_len);
	printf("[Sender] Time to send data : %lf second\n", trans_time);
	printf("[Sender] Data transmission rate : %lu bps\n", (unsigned long) trans_rate);
	return 0;
}

