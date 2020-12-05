# SivarajeshA-Cache-Covert-Channel-Attack_-FLUSH-RELOAD-
**This project is a demonstration of Flush and Reload attack on real system.**<br/> <br/>
Creating a covert channel between Gogo and Gollu(two processes) that are running on two separate physical cores. 
They cannot communicate directly for confidentiality reasons and that’s why they need to create a covert channel to communicate so that no other processes could get the information.<br/> <br/>
**Task 1a**<br/> In this task, I have created a Cache Covert Channel using
FLUSH+RELOAD where Gogo reads some information from the terminal and sends it to the Gollu.
You can assume that the information read from the terminal will not be of more than 50 characters.
Covert channel bandwidth and the accuracy of the channel is also reported.

<br/>**Task 1b**<br/> This Task is an extension to Task 1a, here a secret information in a text file is shared. This task is
further divided into the following subtasks:<br/>
1. Gogo first reads a filename from the terminal and communicate the filename to Gollu using the
covert channel. You can assume that the file to be send will be in the current working directory
of Gogo.<br/>
2. After receiving the file name, Gollu creates a new blank file by appending received_ in front of
the filename received from Gogo.
3. After sending the filename, Gogo then read the content of the file and communicate it to Gollu
using covert channel again!
4. Gollu then writes all the received contents into a file that is earlier created by Gollu.

<br/>**Task 3a**<br/>  
In this task, Gogo sends his heart (soul may be, in the form of an image file) to Gogo over the Cache Covert Channel,
created using FLUSH+RELOAD attack. <br/>


<br/>**How to run** <br/>
**To make**<br/>
> make
<br/>
<br/>
**#core 0-3<br/>**
**#To launch a program on a specific core**<br/>
> taskset -c 0 ./t1a-sender <br/>
> taskset -c 1 ./t1a-receiver<br/>
<br/>
<br/>
**#To find PID of process**<br/>
> ps aux | grep -1 ./t1a-sender<br/>
> ps aux | grep -1 ./t1a-receiver<br/>
<br/>
<br/>
**#to find core of a running process**<br/>
> taskset -cp pid<br/>
> taskset -p 1 pid<br/>
<br/>
