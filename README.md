A server client model that uses pipes and sockets for communication. Lets you choose different points on the interactive map and shows the shortest path 
between the two points.

##Running Instructions
---------------------------------Server Instructions-------------------------------------------------

Compile the server executable by running the "make server" command from the terminal in the main working directory. This will create the executable "server" in the server directory.
To run, the server, type the following in the terminal from the main directory:

./server/server [PORT]

where PORT is the port number you would like to use (use a higher port number in the range of 5000 to 64000), for example:

./server/server 8888

---------------------------------Client Instructions-------------------------------------------------

Compile the client executable by running the "make client" command from the terminal in the main working directory. This will create the executable "client" in the client directory.
To run the client, type the foolowing in the terminal from the main directory:

./client/client [PORT] [IP ADDRESS]

where PORT is the port number (same as client) and the IP ADDRESS correspond to the server's IP address. For example:

./client/client 8888 127.0.0.1

--------------------------------------Plotter--------------------------------------------------------

After executing server and client respectively, from the main working directory, execute the plotter by typing the following in the terminal:

./plotter

This should open up the visual Edmonton map, where you can scroll around, zoom in/zoom out and select start and end points for any trip.

Select any start and end points and the plotter displays the least cpst path between the two. You can select multiple start and end points.

----------------------------------Sample Running Instructions----------------------------------------

Type the following commands in the terminal from the main working directory in the given order:

make server
make client
./server/server 8888

////////////////In new terminal Window///////////////////
./client/client 8888 127.0.0.1

////////////////In new terminal Window///////////////////
./plotter

Alternatively, the following commands can also be used:

make
./server/server 8888

////////////////In new terminal Window//////////////////
./client/client 8888 127.0.0.1

////////////////In new terminal Window//////////////////
./plotter

Here, the "make" command compiles and builds both the executables "client" and "server".

--------------------------------- Running everything Together----------------------------------------

To execute the program, that is to run the server, client and plotter, all by one command, type the following in the terminal from the main working directory:

make run

This Makefile command compiles and creates the executables "server" and "client" and runs the "server", "client" and "plotter" in three different windows with a default value of PORT as 8888 and default value of IP ADDRESS as 127.0.0.1.
-----------------------------------------------------------------------------------------------------

To clean all the executables, pipes and the object file, simply run the "make clean" command.

-----------------------------------------------------------------------------------------------------

##Makefile Commands
  - make all: runs the server and client targets to generate all the executables.
	- make server: links all the object files for the server and builds an executable called server
				   in the respective directory
	- make server.o: compiles the "server.cpp" into an object file "server.o"
	- make dikstra.o: compiles the "dijkstra.cpp" into an object file "dijkstra.o"
	- make digraph.o: compiles the "digraph.cpp" into an object file "digraph.o"
	- make client: builds an executable called client in the respective directory.
	- make client.o: compiles the "client.cpp" into an object file "client.o"
	- make run: runs the server and client targets to create all executables. It also opens three different terminals to run the program. One terminal executes the server with a default port of 8888, another terminal executes the client with the default port of 8888 and default IP address of 127.0.0.1. Finally the last terminal executes the plotter to open the scrollable map of edmonton.
	- make clean: removes all the executables, objects and named pipes.

##Notes and Assumptions

  - All the Makefile targets are written to work from the main directory itself.
	- There is an additional Makefile target that allows to run the complete program (i.e server,
	  client and plotter) with a default value of PORT and IP ADDRESS.
	- edmonton-roads-2.0.1.txt should be included in the main working directory while testing.
