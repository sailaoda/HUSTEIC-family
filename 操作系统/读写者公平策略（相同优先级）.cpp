// 设计思想
//为使得两者公平竞争，还需满足

//（1）读者写者的优先级相同

//（2）读者、写者互斥访问

//（3）只允许有一个写者访问临界区

//（4）可以有多个读者同时访问临界区的资源

//  实现方法

//（1）设置rw 信号量实现对临界资源的互斥访问。

//（2）设置计数器readcount实现多个读者访问临界资源，通过设置信号量mutex实现对readcount 计数器的互斥访问。

//（3）设置信号量w实现读者和写者的公平竞争


int readcount=0;
semaphore w=1, mutex =1;
Reader()
{
	While(true)
	{
        wait (w);
        wait (mutex);
        if (readcount == 0)
	       wait(rw);
    	readcount++;	
    	signal (mutex);
        signal (w);
		 …
    	   perform read operation
		 …
    	wait (mutex);
    	readcount--;
    	if (readcount == 0)
    		signal (rw);
    	signal (mutex);
	}
}
 
 
Writer()
{     
	While(true)
{
    wait(w);
    wait(rw);
	    …
         perform write operation
	    …
    signal(rw);
signal(w);
}
}
