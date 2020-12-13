# pinger

A simple C project using [pthreadpool](https://github.com/SimoneRicci97/pthreadpool) and many other C skills (such as multiprocess, signal callback, file handling).  
pinger is an easy _net-monitor_ that simply exec ping towards one pr more hosts specified by command line collecting some stats (min-max-avg interval of pings) and produces a log file each n seconds (where n is a program argument, by default n=10). Every n1 seconds the collected stats ar cleaned and creaed reports are compressed in a tar file (n1 is a program argument too, by default n1=3600).
pinger has not been created to be an efficient and secure net-monitor or net-sniffer, but just to show my C language skills. Enjoy.