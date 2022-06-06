# How to set up and run my code
1. first copy the code from the QTI program
2. then add the else if statement if the car detect 0b1110 and 0b0111 which are my marker to turn left and right
3. I set a variable right to store status, when detect 0b1110 right will be 2, detect 0b01111 right will be 1
4. If detect 0b1111, it will check the variable right, and if it is 2 then turn left, if it is 1 then turn right
5. I also set a variable count to check if it is a error detect, if continuous detect the same marker, count will add up and to an amount it will verify that this is a right detect
6. and add a else if statement if the car detect 0b0000 to stop
7. For the ping, first write a selfturn function, which let servo0's speed equals the speed of servo1
8. Then at the top of main function add a else if statement to check whether detect the obstacles or not
9. when detect then selfturn and keep finish the map
10. For erpc, first create a erpc file, and copy some of them to final program
11. then create a python to make command, print out command and let the user enter d or s for distance and speed
12. then add a thread in the final program, the thread is to start the erpc, and in the erpc function it will start the rpc_server.run()
13. then it will call the distance and Speed function, this two funciton is to print out the speed and distance, which uses the data collect by encode.

# What are the result
1. the car can smoothly go through the map
2. it will detect the marker and turn correctly
3. when it detect obstacles it will selfturn for half circle and when it detect black line again it will stop turning and go on
4. when I start python and press d or s it will return distance or speed
5. it will stop at blank space
