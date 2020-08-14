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
	//Sending 01010100 to inform receiver that sender stops data transmission
	send_data(0, map);
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(0, map);
	send_data(1, map);
	send_data(0, map);
	send_data(0, map);
}


int main(int argc, char **argv) {

	char filename[51];
	unsigned int file_size = 0;
	clock_t t_send;
	double trans_time;
	double trans_rate;
	char read_buffer[50];
	
	// TODO: Establish your cache covert channel

	printf("\nEnter filename(with extension) to be send: ");
	fgets (filename, 50, stdin);
	char *filename2=filename;
	
	int file_nme_len =strlen(filename);
	filename[file_nme_len-1]='\0';
	
	map_handle_t *handle;
	unsigned int *map;
	map = (unsigned int *) map_file("share_mem.txt",&handle);
	assert(map!=NULL);

	/* TODO:
	 * Send filename to the receiver
	 * Open text file in read mode.
	 * Store the size of file in file_size variable.
	*/
	char *bin=string_to_binary(filename); 
	char *t=bin;
		
	//Send start pattern to receiver
	send_pattern(map);
	
	// TODO: Transmit message over the cache covert channel
	t_send = clock();
	//Sending file name
	while(*t){
		if(*t=='0'){
			send_data(0, map);
		}
		else{
			send_data(1, map);
		}
		t++;
	}
	
	printf("\nFile name sent => %s\n",filename2);
	sleep(1);
	
	FILE *fp; 
    fp  = fopen (filename, "r");
	assert(fp!=NULL);
  	printf("\nSending file data...");  	
  	
  	// TODO: Transmit file contents to the Receiver over the cache covert channel
   	while( fgets ( read_buffer, 50, fp ) != NULL ) { 
   		//if( fgets ( read_buffer, 50, fp ) != NULL ){
    	bin=string_to_binary(read_buffer); 
    	t=bin;
    	send_pattern(map);
        while(*t){
			if(*t=='0'){
				send_data(0, map);
			}
			else{
				send_data(1, map);
			}
			t++;
		} 
		printf("\n.");
		sleep(1);
		
    } 
    
    fseek(fp, 0, SEEK_END);     
    file_size = ftell(fp); 	//To calculate file size
    fclose (fp); 
    
    //Send end pattern
    send_pattern_2(map);
            
    printf("\nFile sent\n");

	t_send = clock() - t_send;
	trans_time = ((double) t_send) / CLOCKS_PER_SEC;
	trans_rate = (double) (file_size * 8) / trans_time;

	printf("\n[Sender] File (%s) data Send : %u bytes\n", filename2, file_size);
	printf("[Sender] Time to send file data : %lf second\n", trans_time);
	printf("[Sender] Data transmission rate : %lu bps\n", (unsigned long) trans_rate);

	return 0;
}
