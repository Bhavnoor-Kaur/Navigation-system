#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <vector>

#define MAX_SIZE 1024

using namespace std;

/*
    Description: This function creates named pipes 
                 and opens them for read or write
                 operations respectively
    Arguments:
            const char * pname: name of the pipe to be created
            int mode: The mode in which the pipe is to be opened
    Returns:
            (int) the pipe descriptor
*/
int create_and_open_fifo(const char * pname, int mode) {
    // creating a fifo special file in the current working directory
    // with read-write permissions for communication with the plotter
    // both proecsses must open the fifo before they can perform
    // read and write operations on it
    if (mkfifo(pname, 0666) == -1) {
        cout << "Unable to make a fifo. ";
        cout << "Ensure that this pipe does not exist already!" << endl;
        exit(-1);
    }

    // opening the fifo for read-only or write-only access
    // a file descriptor that refers to the open file description is
    // returned
    int fd = open(pname, mode);

    if (fd == -1) {
        cout << "Error: failed on opening named pipe." << endl;
        exit(-1);
    }

    return fd;
}

int main(int argc, char const *argv[]) {
    const char *inpipe = "inpipe";
    const char *outpipe = "outpipe";

    int in = create_and_open_fifo(inpipe, O_RDONLY);
    cout << "inpipe opened..." << endl;
    int out = create_and_open_fifo(outpipe, O_WRONLY);
    cout << "outpipe opened..." << endl;

    // storing the port number and the IP address from the command line
    int PORT = stoi(argv[1]);
    string IP = argv[2];

    // declaring char arrays to store the messages to be sent to server
    // and the plotter
    char serverMsg[MAX_SIZE] = {0};
    char plotterMsg[MAX_SIZE] = {0};

    // sockaddr_in is the address sturcture used for IPv4
    // sockaddr is the protocol independent address structure
    struct sockaddr_in addr, connAddr;

    // zero out the structor variable because it has an unused part
    memset(&addr, '\0', sizeof(addr));

    // assign the sockaddr_in structure variables
    connAddr.sin_family = AF_INET;
    connAddr.sin_port = htons(PORT);
    connAddr.sin_addr.s_addr = inet_addr(IP.c_str());


    // create the socket
    int socketFd;
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1) {
        cerr << "Listening socket creation failed!\n";
        return 1;
    }

    // establish a connection with the server socket
    if (connect(socketFd, (struct sockaddr *)&connAddr,
                                    sizeof(connAddr)) == -1) {
        cerr << "Cannot connect to the host!\n";
        close(socketFd);
        return 1;
    }
    cout << "Connection established with " << inet_ntoa(connAddr.sin_addr);
    cout << ":" << ntohs(connAddr.sin_port) << "\n";

    // coords vector stores the coordinates of the start and end points
    vector<double> coords = {};

    while (true) {
        // read in the coordinates from the plotter inpipe and store in a vector
        while (coords.size() < 4) {
            int rInpipe = read(in, serverMsg, MAX_SIZE);
            // if plotter sends "Q", the quit message, close all the pipes
            // and socket and quit the program
            if (serverMsg[0] == 'Q') {
                string quitMsg = "Q";
                send(socketFd, quitMsg.c_str(), quitMsg.size(), 0);
                close(socketFd);
                close(in);
                close(out);
                unlink(inpipe);
                unlink(outpipe);
                return 0;
            }

            // extract the coordinates, convert into double and store in the
            // vector "coords"
            char coord[24] = {0};
            int j = 0;
            for (int i = 0; i < MAX_SIZE; i++) {
                if (serverMsg[i] == '\n' || serverMsg[i] == ' ') {
                    coord[j] = '\0';
                    coords.push_back(atof(coord));
                    j = 0;
                    continue;
                }
                if (serverMsg[i] == '\0') {
                    if (j != 0) {
                        coord[j] = '\0';
                        coords.push_back(atof(coord));
                        j = 0;
                    }
                    break;
                }
                coord[j] = serverMsg[i];
                j++;
            }
            // set to zero
            memset(serverMsg, 0, MAX_SIZE);
            memset(coord, 0, 24);
        }

        // sending the route to the server by first converting it into a string
        // then converting it into a char array.
        string route = "R";
        for (int i = 0; i < 4; i++) {
            route += " " + to_string(static_cast<long long> (coords[i]*100000));
        }
        // sending via socket
        send(socketFd, route.c_str(), route.size(), 0);

        // receiving the message from the socket, giving the num of waypoints
        recv(socketFd, plotterMsg, MAX_SIZE, 0);
        int numWaypts = atoi(plotterMsg + 2);

        // if there is no path between the start and end point specified by user
        if (numWaypts == 0) {
            string noPath = "E\n";
            write(out, noPath.c_str(), noPath.size());
            coords = {};
            continue;
        }

        vector<string> pathCoords;
        while (true) {
            char acknowledge[] = {'A'};
            // acknowledge the message from the socket
            send(socketFd, acknowledge, strlen(acknowledge) + 1, 0);
            memset(plotterMsg, 0, MAX_SIZE);
            // recieve the coordinates of the waypoints via socket
            recv(socketFd, plotterMsg, MAX_SIZE, 0);

            // when it is the end of the path, server sends "E"
            if (plotterMsg[0] == 'E') {
                pathCoords.push_back("E\n");
                coords = {};
                break;
            }

            // convert the coordinate of the waypoints into a string
            // and store it into a vector
            string str = "";
            vector<long long> received;
            for (int i = 2; i < MAX_SIZE; i++) {
                if (plotterMsg[i] == ' ') {
                    received.push_back(stoll(str));
                    str = "";
                    continue;
                }
                if (plotterMsg[i] == '\0') {
                    received.push_back(stoll(str));
                    str = "";
                    break;
                }
                str += plotterMsg[i];
            }
            // converting the coordinates of the waypoint into a string message
            string point = "";
            point += (to_string((static_cast<double> (received[0]))/100000));
            point += " ";
            point += (to_string((static_cast<double> (received[1]))/100000));
            point += "\n";

            // N 1 case
            if (numWaypts == 1) {
                point += point;
            }

            // add the string with the coordinates to the path
            pathCoords.push_back(point);
        }

        // write the waypoints of the path to the outpipe of the plotter
        for (int i = 0; i < pathCoords.size(); i++) {
            write(out, pathCoords[i].c_str(), pathCoords[i].size());
        }
    }

    // close socket & pipes
    close(socketFd);
    close(in);
    close(out);
    unlink(inpipe);
    unlink(outpipe);
    return 0;
}
