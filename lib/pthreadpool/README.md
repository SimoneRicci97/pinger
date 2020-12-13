# pthreadpool  
  
### A simple thradpool implementation based on pthread  
  
pthreadpool is a C library developed on (and for) Unix-based systems, that allow you to implement a threadpool, with a few lines of code and a significative optimization (in terms of time and machine resources).  
pthreadpool use e fixed and pre-decided number of thread to execute a various (potentially infinite) number of tasks.   
A task is represented, the same as for posix threads, by a ```void* (void*)``` function pointer. When you want a task to be executed by your threadpool, through the ```ptp_add_task```, you have to pass to the threadpool the funvtion pointer and a ```void*``` arguments that will be used to exec the task. You have no guarantees about **when** the task will be executed, but it will surely run.   
  
N.B.: the tasks passed to threadpool are executed with **FIFO policy**  
  
### Build  
```sh  
$ git clone https://github.com/SimoneRicci97/pthreadpool.git  
$ cd pthreadpool   
$ mkdir build && cd build  
$ make  
```  
The above commands allow you to create the file **build/libpthreadpool.a** which can be linked to your own project.  
You can also comile pthreadpool inside your project directly, including it in your Makefile or CMakefile.txt.  
```sh  
$ git clone https://github.com/SimoneRicci97/pthreadpool.git  
$ cp -r pthreadpool /your/project/path/  
$ <commands-to-build-your-project>  
```  
  
# Test  
Using build commands an executable file will be created to show you the time spent to sum the items in a 10000000 integers array. To see the powerful of pthreadpool library you just have to run  
```sh  
$ ./build/test/sum/test_pthreadpool  
```  
The test creates an array of size 10000000, populated with random numbers.   
Then execs the sum using only one thread.  
Then execs different test creating pthreadpool with 2, 4 or 8 threads and passing them tasks wich execute the sum of a segment of array. The results will be retrieved by the main thread and added together.  
Testing it on my machine (quad-core architecture 2.5 GHz, with 8 Gb memory, OS Ubuntu 16.04) i've obtained the following results  
  
> Single thread  
> 	Sum: 95003699  
> 	Execution time: 3.50 micros  
>   
> Test with 2 threads and chunksize = 100000  
> N tasks: 100  
> 2 threads  
> Chunksize: 100000  
> 	Sum: 95003699  
> 	Execution time: 3.89 micros  
>   
> Test with 2 threads and chunksize = 1000000  
> N tasks: 10  
> 2 threads  
> Chunksize: 1000000  
> 	Sum: 95003699  
> 	Execution time: 2.59 micros  
>   
> Test with 4 threads and chunksize = 100000  
> N tasks: 100  
> 4 threads  
> Chunksize: 100000  
> 	Sum: 95003699  
> 	Execution time: 3.16 micros  
>   
> Test with 4 threads and chunksize = 1000000  
> N tasks: 10  
> 4 threads  
> Chunksize: 1000000  
> 	Sum: 95003699  
> 	Execution time: 4.21 micros  
>   
> Test with 8 threads and chunksize = 100000  
> N tasks: 100  
> 8 threads  
> Chunksize: 100000  
> 	Sum: 95003699  
> 	Execution time: 2.74 micros  
>   
> Test with 8 threads and chunksize = 1000000  
> N tasks: 10  
> 8 threads  
> Chunksize: 1000000  
> 	Sum: 95003699  
> 	Execution time: 2.73 micros  
  
As you can see, the execution time depends on the number of threads in threadpool and and the size of array segment handled by a task.  
  
# Project using pthreadpool  
> [Coming soon](coming-soon)  
  
# Hint  
Feel free to suggest features, report bug fixes or contribute.  
  
  
  