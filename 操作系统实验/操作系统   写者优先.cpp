/*
* 	写者优先
*/

//设计思路 
/* 
写者优先与读者优先类似。不同之处在于一旦一个写者到来，它应该尽快对文件进行写操作，如果有一个写者在等待，则新到来的读者不允许进行读操作。
为此应当添加一个整型变量write_count，用于记录正在等待的写者的数目，当write_count=0时，才可以释放等待的读者线程队列。

为了对全局变量write_count实现互斥，必须增加一个互斥对象mutex2。

为了实现写者优先，应当添加一个临界区对象read，当有写者在写文件或等待时，读者必须阻塞在read上。同样，有读者读时，写者必须等待。于是，必须有一个互斥对象RW_mutex来实现这个互斥。

有写者在写时，写者必须等待。

读者线程要对全局变量read_count实现操作上的互斥，必须有一个互斥对象命名为mutex1。
*/

//信号量解析

/*
代码中信号量解析：
设置五个信号量，分别是RWMutex, mutex1, mutex2, mutex3, wrt，两个全局整型变量writeCount, readCount

信号量mutex1在写者的进入区和退出区中使用，使得每次只有一个写者对其相应进入区或推出区进行操作，
主要原因是进入区和退出区存在对变量writeCount的修改，每个写者其进入区中writeCount加1，退出区中writeCount减1。
信号量RWMutex则是读者和写者两个之间的互斥信号量，保证每次只读或者只写。

写者优先中，写者的操作应该优先于读者，则信号量一直被占用着，直到没有写者的时候才会释放，
即当writeCount等于1的时候，申请信号量RWMutex，其余的写者无需再次申请，但是写者是不能同时进行写操作的，
则需要设置一个信号量wrt来保证每次只有一个写者进行写操作，当写者的数量writeCount等于0的时候，则证明此时没有没有读者了，
释放信号量RWMutex。信号量mutex2防止一次多个读者修改readCount。当readCount为1的时候，为阻止写者进行写操作，申请信号量wrt，则写者就无法进行写操作了。
信号量mutex3的主要用处就是避免写者同时与多个读者进行竞争，读者中信号量RWMutex比mutex3先释放，则一旦有写者，写者可马上获得资源。


*/ 



# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <sys/types.h>
# include <pthread.h>
# include <semaphore.h>
# include <string.h>
# include <unistd.h>

//semaphores
sem_t RWMutex, mutex1, mutex2, mutex3, wrt;
int writeCount, readCount;


struct data {
	int id;
	int opTime;
	int lastTime;
};

//读者
void* Reader(void* param) {
	int id = ((struct data*)param)->id;
	int lastTime = ((struct data*)param)->lastTime;
	int opTime = ((struct data*)param)->opTime;

	sleep(opTime);
	printf("Thread %d: waiting to read\n", id);	

	sem_wait(&mutex3);
	sem_wait(&RWMutex);
	sem_wait(&mutex2);
	readCount++;
	if(readCount == 1)
		sem_wait(&wrt);
	sem_post(&mutex2);
	sem_post(&RWMutex);
	sem_post(&mutex3);

	printf("Thread %d: start reading\n", id);
	/* reading is performed */
	sleep(lastTime);
	printf("Thread %d: end reading\n", id);

	sem_wait(&mutex2);
	readCount--;
	if(readCount == 0)
		sem_post(&wrt);
	sem_post(&mutex2);

	pthread_exit(0);
}

//写者
void* Writer(void* param) {
	int id = ((struct data*)param)->id;
	int lastTime = ((struct data*)param)->lastTime;
	int opTime = ((struct data*)param)->opTime;

	sleep(opTime);
	printf("Thread %d: waiting to write\n", id);
	
	sem_wait(&mutex1);
	writeCount++;
	if(writeCount == 1){
		sem_wait(&RWMutex);
	}
	sem_post(&mutex1);
	
	sem_wait(&wrt);
	printf("Thread %d: start writing\n", id);
	/* writing is performed */
	sleep(lastTime);
	printf("Thread %d: end writing\n", id);
	sem_post(&wrt);

	sem_wait(&mutex1);
	writeCount--;
	if(writeCount == 0) {
		sem_post(&RWMutex);
	}
	sem_post(&mutex1);
	
	pthread_exit(0);
}

int main() {
	//pthread
	pthread_t tid; // the thread identifier

	pthread_attr_t attr; //set of thread attributes

	/* get the default attributes */
	pthread_attr_init(&attr);

	//initial the semaphores
    sem_init(&mutex1, 0, 1);
    sem_init(&mutex2, 0, 1);
    sem_init(&mutex3, 0, 1);
    sem_init(&wrt, 0, 1);
    sem_init(&RWMutex, 0, 1);

    readCount = writeCount = 0;

	int id = 0;
	while(scanf("%d", &id) != EOF) {

		char role;		//producer or consumer
		int opTime;		//operating time
		int lastTime;	//run time

		scanf("%c%d%d", &role, &opTime, &lastTime);
		struct data* d = (struct data*)malloc(sizeof(struct data));

		d->id = id;
		d->opTime = opTime;
		d->lastTime = lastTime;

		if(role == 'R') {
			printf("Create the %d thread: Reader\n", id);
			pthread_create(&tid, &attr, Reader, d);

		}
		else if(role == 'W') {
			printf("Create the %d thread: Writer\n", id);
			pthread_create(&tid, &attr, Writer, d);
		}
	}

	sem_destroy(&mutex1);
	sem_destroy(&mutex2);
	sem_destroy(&mutex3);
	sem_destroy(&RWMutex);
	sem_destroy(&wrt);

	return 0;
}

