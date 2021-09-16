//读入写入者实验伪代码(读者优先） 

int readcount=0;
semaphore writeblock,mutex;
writeblock=1;mutex=1;//初始条件可以读入写入
Cobegin


Process ReaderThread()
{
	P(mutex);    //获取进入临界区权限
    readcount++; //阅读者人数加一
	
	if(readcount==1)      //阅读者人数等于一表示之前没有人阅读，需要获取阅读权限
	    P(writeblock);     //写入者无法写入
		V(mutex);          //释放权限，允许其他阅读者进入
		
		/*读文件*/ 
		
		P(mutex);       //阅读完毕获取修改阅读人数的权限 
		readcount--;    //阅读人数减一 
		if(readcount==0)            //表示他是最后一个阅读者，阅读者释放临界区，允许写入者写入 
		      V(writeblock);
		V(mutex);
 } 
 
 
 Process ReaderThread()     //写入进程 
 {
 	P(writeblock);          //写入者写入，阅读者无法进入临界区 
 	/*写文件*/
	 
	V(writeblock);          //开放写入 
 }
 
 coend
