Implementation of a client-server navigation system using dijkstra's algorithm with an interactive map to find the shortest distance between any two points

# Running Instructions:

Server Instructions

Compile the server executable by running the "make server" command from the terminal in the main working directory. This will create the executable "server" in the server directory.
To run, the server, type the following in the terminal from the main directory:

./server/server [PORT]

where PORT is the port number you would like to use (use a higher port number in the range of 5000 to 64000), for example:

./server/server 8888

Client Instructions

Compile the client executable by running the "make client" command from the terminal in the main working directory. This will create the executable "client" in the client directory.
To run the client, type the foolowing in the terminal from the main directory:

./client/client [PORT] [IP ADDRESS]

where PORT is the port number (same as client) and the IP ADDRESS correspond to the server's IP address. For example:

./client/client 8888 127.0.0.1

Plotter

After executing server and client respectively, from the main working directory, execute the plotter by typing the following in the terminal:

./plotter

This should open up the visual Edmonton map, where you can scroll around, zoom in/zoom out and select start and end points for any trip.

Select any start and end points and the plotter displays the least cpst path between the two. You can select multiple start and end points.

**Note: The plotter file is not included in the repository because the executable was too large
