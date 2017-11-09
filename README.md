# tcp-load-generator
Our load generator performs close loop test using N threads.The response time, T , in a closed system is dened to be the time from when a request is submitted until it is received. We are performing a closed loop test where new job arrivals are only triggered
by job completions.In order to simulate N concurrent users, we create N threads in our load generator. Each of the N threads emulate one user by issuing one request, waiting for it to complete, and issues the next request immediately afterwards.There is no think time between requests.

Approach I: Running load generator for specific time Here,user can give total duration for which timer should run and user work(void *threadid) function keep issuing the request after completion till timer goes off. Here, however number of requests send by different users/threads within the stipulated time may not be same as we increase the number of threads because request of different users/threads may take different time for completion.

Approach II: Running load generator for fixed number of requests per user Here,user can give total number of requests per user to execute and user work(void *threadid) function of each thread keep issuing the request till per thread requests defined by user are completed.This approach ensures that load generator generates equal work load(number of requests) for each user(thread).
