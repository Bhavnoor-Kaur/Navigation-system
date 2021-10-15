// -----------------------------------------
// Name: Bhavnoor Kaur
// ID:1623727
// CMPUT 275, Winter 2021
//
// Assignment 1: Navigation System (Part 2)
// -----------------------------------------

#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <list>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "wdigraph.h"
#include "dijkstra.h"

/*
  Description: A struct that stores latitude and longitude of a Point
*/
struct Point {
  long long lat, lon;
};

/*
  Description: calculates the manhattan distance between two points
  Arguments: 
      const Point& pt1: const reference to point
      const Point& pt2: const reference to point
  Returns:
      long long: manahattan distance between two points
*/
long long manhattan(const Point& pt1, const Point& pt2) {
  long long dLat = pt1.lat - pt2.lat, dLon = pt1.lon - pt2.lon;
  return abs(dLat) + abs(dLon);
}

/*
  Description: Finds the closest vertex to the given one.
  Arguments:
            const Point& pt: constant reference to the point whose closest
                             vertex is to be found
            const unordered_map<int, Point>& points: the map which contains all
                                                     the points on the map
  Return:
        (int): the ID of the closest vertex
*/
int findClosest(const Point& pt, const unordered_map<int, Point>& points) {
  pair<int, Point> best = *points.begin();

  for (const auto& check : points) {
    if (manhattan(pt, check.second) < manhattan(pt, best.second)) {
      best = check;
    }
  }
  return best.first;
}

/*
  Description: Reads the graph from the file and stores the ID and corresponding
         latitude and longitude in the unordered_map points.
  Arguments:
      string filename: name of the file describing the road network
      Wdigraph& graph: reference to the instance of WDigraph class
        unordered_map<int, Point>& points: map to store the ID and corresponding
                           latitiudes and longitudes
  Returns:
      None
*/
void readGraph(const string& filename, WDigraph& g,
                          unordered_map<int, Point>& points) {
  ifstream fin(filename);
  string line;

  while (getline(fin, line)) {
    // split the string around the commas, there will be 4 substrings either way
    string p[4];
    int at = 0;
    for (auto c : line) {
      if (c == ',') {
        // starting a new string
        ++at;
      } else {
        // appending a character to the string we are building
        p[at] += c;
      }
    }

    if (at != 3) {
      // empty line
      break;
    }

    if (p[0] == "V") {
      // adding a new vertex
      int id = stoi(p[1]);
      // sanity check: asserts if some id is not 32-bit
      assert(id == stoll(p[1]));
      points[id].lat = static_cast<long long>(stod(p[2])*100000);
      points[id].lon = static_cast<long long>(stod(p[3])*100000);
      g.addVertex(id);
    } else {
      // adding a new directed edge
      int u = stoi(p[1]), v = stoi(p[2]);
      g.addEdge(u, v, manhattan(points[u], points[v]));
    }
  }
}

/*
  Description: This function returns the first char that is read from the socket
  Arguments:
            int socket: the socket descriptor from where we are reading from
  Return:
          (char) the first character that is read from the socket
*/
char readAck(int socket) {
  char buffer[512] = {0};
  read(socket, buffer, 512);
  return buffer[0];
}


int main(int argc, char* argv[]) {
  WDigraph graph;
  unordered_map<int, Point> points;
  int PORT = stoi(argv[1]);

  // build the graph
  readGraph("edmonton-roads-2.0.1.txt", graph, points);

  // declaring socket descriptors
  int serverFd, connFd;

  // declaring socket address struct and initialising the variables
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port = htons(PORT);

  int addrLen = sizeof(address);

  // listening socket
  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd == -1) {
    cerr << "Listening socket creation failed!" << endl;
    return 1;
  }

  // binding the socket
  if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) == -1) {
    cerr << "Binding failed!" << endl;
    close(serverFd);
    return 1;
  }

  cout << ("Binding was successful\n");

  // listening to the socket
  if (listen(serverFd, 10) == -1) {
    cerr << "Cannot listen to the specified socket" << endl;
    close(serverFd);
    return 1;
  }
  cout << "Listening to port " << PORT << endl;

  // accept the connection from the server side
  connFd = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrLen);

  if (connFd == -1) {
    cerr << "Connection socket creation failed!" << endl;
    return 1;
  }
  cout << "Connection request accepted from " << inet_ntoa(address.sin_addr);
  cout << ":" << ntohs(address.sin_port) << "\n";

  // packet reads in the message from the server socket
  char packet[512] = {0};
  bool finish = false;
  while (!finish) {
    int routeReq = read(connFd, packet, 512);
    if (routeReq == 0) {
      break;
    }

    // extracting the start and end coordinates and storing them in a vector
    // after converting into long long
    vector<long long> coords;
    if (packet[0] == 'R') {
      string coord = "";
      for (int i = 2; i < 512; i++) {
        if (packet[i] == ' ') {
          coords.push_back(stoll(coord));
          coord = "";
        }
        if (packet[i] == '\0') break;
        coord += packet[i];
      }
      coords.push_back(stoll(coord));

      // create the start and end points from the extracted coordinates
      Point sPoint, ePoint;
      sPoint.lat = coords[0];
      sPoint.lon = coords[1];
      ePoint.lat = coords[2];
      ePoint.lon = coords[3];

      // get the points closest to the two points we read
      int start = findClosest(sPoint, points);
      int end = findClosest(ePoint, points);

      // run dijkstra's
      unordered_map<int, PIL> tree;
      dijkstra(graph, start, tree);

      // no path
      if (tree.find(end) == tree.end()) {
        string noPath = "N 0";
        write(connFd, noPath.c_str(), noPath.size() + 1);
      } else {
        // read off the path by stepping back through the search tree
        list<int> path;
        while (end != start) {
          path.push_front(end);
          end = tree[end].first;
        }
        path.push_front(start);

        // output the path
        // sends N X message, where X > 0
        string numPts = "N " + to_string(path.size());
        write(connFd, numPts.c_str(), 512);

        for (int v : path) {
          // read in the acknowledgement message, when acknowledges write
          // to the socket
          int ack = readAck(connFd);
          // if quit message is recieved
          if (ack == 'Q') {
            finish = true;
            close(connFd);
            break;
          } else if (ack == 'A') {
              string wayPts = "W " + to_string(points[v].lat) +
                    " " + to_string(points[v].lon);
              write(connFd, wayPts.c_str(), 512);
          } else {
              break;
          }
        }

        // take in the last acknowledgement
        int last = readAck(connFd);
        // if the user quits, then close the socket and end process, else write
        // to the socket
        if (last == 'Q') {
          finish = true;
          close(connFd);
          break;
        } else if (last == 'A') {
            // sends End message
            string end = "E";
            // cout << "E" << endl;
            write(connFd, end.c_str(), end.size()+1);
        } else {
            continue;
        }
      }
    } else if (packet[0] == 'Q') {
        finish = true;
        close(connFd);
        continue;
    } else {
        continue;
    }
  }

  // close socket
  close(serverFd);

  return 0;
}
