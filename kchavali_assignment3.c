/**
 * @kchavali_assignment3
 * @author  krishnakant chavali <kchavali@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
 #include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
 #include <time.h>
 #include <sys/time.h>
#include <errno.h>
 #include <libgen.h>
#include "../include/global.h"
#define LENGTH 1024
 #define inf 65535
#include "../include/global.h"
#include "../include/logger.h"
#include <stdint.h>
 //[PA3] Update Packet Start
#pragma pack(push,1) ////http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0491e/CJAFEEDG.html
typedef struct
{
	uint32_t server_IP;
	uint16_t server_port;
	uint16_t zeros;
	uint16_t server_ID;
	uint16_t cost;
} serverpkt;
typedef struct
{
	uint16_t updatefields;
	uint16_t serverport;
	uint32_t serverIP;
	serverpkt servers[5]; 
} packet;
#pragma pack(pop)
//[PA3] Update Packet End
typedef struct
{
	uint16_t serverid;
	char serverip[50];
	uint16_t portno;
} servertable;
typedef struct
{
	uint16_t destination[5][5];
	uint16_t cost[5][5];
} fullTable;
//[PA3] Routing Table Start
typedef struct
{
	uint16_t destinationID[5];
	int16_t nexthopID[5];
	int mincost[5];
} routingtable;
//[PA3] Routing Table End


int createTable(char *, char *);//extract values from topology file
int updateTable(int , int , int);
int sendPackets(); // send packets to all my servers
int display();
int createRoutingTable();
int disableLink();
int crash(char *);
int dumpTable();
char *myip();
int bellmanFord(packet , int );
int mincost(int costs[5][5]);
/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

servertable server_Table[5];
packet routingpacket;
packet recvdpkt;
fullTable table;
routingtable route;
int costFromTable[5];
int sender;
int crashed = 0;
 int noofpackets = 0;
 int fdmax, listener;
 struct timeval tv;
fd_set master,read_fds;
char *filename,*fr;
int noofservers,noofedges;
int neighbours[4][2];
int src;
int node[5], started[5];
int done[5];
char *token, *token1, *token2, *token3, *token4, *token5;
int listeningport;
int counter = 0;
int costs[5][5];
int main(int argc, char **argv)
{

	int i;
	struct timeval tv;
	// Checking the commandLine parameters being entered
	if (argc < 4)
	{
		fprintf(stderr,"Insufficient parameters: input commands -t <topology filepath> -i <interval>\n");
		exit(0);
	}
	if(strcasecmp(argv[1],"-t") == 0)
	{
		filename = argv[2];
		token = strtok(filename, ".");
		token1 = strtok(NULL,".");
		if (strcasecmp(token1,"txt") != 0)
		{
			printf("Wrong file extension\n");
			exit(0);
		}
		else
		{
			strcat(filename, ".txt");
		}
	}
	else
	{
		fprintf(stderr,"Insufficient parameters: input commands -t <topology filepath> -i <interval>\n");
		exit(0);
	}
	if (strcasecmp(argv[3],"-i") == 0)
	{
		if (atoi(argv[4])<0)
		{
			printf("Enter the right interval\n");
			exit(0);
		}
		else
		{
			tv.tv_sec = atoi(argv[4]);
			tv.tv_usec = 0;

		}
		
	}
listeningport = createTable(filename, myip());//extract values from topology file and create routing packet
createRoutingTable();// Creating the routing packet



	/*Init. Logger*/
	cse4589_init_log();
	
	/*Clear LOGFILE and DUMPFILE*/
	fclose(fopen(LOGFILE, "w"));
	fclose(fopen(DUMPFILE, "wb"));
char cmd[50];
int loop=0;
int yes = 1;
int j;
for (i = 0; i < noofservers; i++)
{
	for (j = 0; j < noofservers; j++)
	{
		costs[i][j] = inf;
	}
}

listener = socket(AF_INET, SOCK_DGRAM , 0); // Servers listening socket
if(listener<0)
{
	perror("Listener error\n");
}
struct sockaddr_in serv_addr;
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	inet_aton(myip(), &serv_addr.sin_addr);
	serv_addr.sin_port=htons(listeningport);
	
	//Reusing an already used socket
	if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR, &yes,sizeof(int)) == -1)
	{
		perror("setsockopt\n");
        	exit(1);
	}
		// bind it to the port we pass into sockaddr_in
	if(bind(listener, (struct sockaddr *)&serv_addr, sizeof serv_addr)<0)
	{
		perror("Error in bind\n");
	}		

FD_SET(listener,&master);
fdmax = listener;

printf("$ ");
fflush(stdout);
while(loop!=1)
{
	
	read_fds = master;
	FD_SET(0,&read_fds);
	int n = select(fdmax+1, &read_fds, NULL, NULL, &tv);
	if(n<0)
	{
		perror("error in select\n");
	}
	else if(n==0)
	{
		printf("Timeout\n");
		sendPackets(); // send packets to all my neighbouring servers
	//	display();
		tv.tv_sec = atoi(argv[4]);
		tv.tv_usec = 0;
	}
	int check;
	memset(cmd, 0, sizeof cmd);
	if(FD_ISSET(0,&read_fds))     					//Handling the input commands from the user
	{
		fgets(cmd, 50, stdin); // Taking the input from the user from the commandline
		token2 = strtok(cmd," ");
			
		for(i=0; i<strlen(token2);i++)
		{
			if(strcmp(&token2[i],"\n")==0)
			{
				token2[strlen(token2) - 1] = 0;
			}	
		}
		token3 = strtok(NULL, " ");
		token4 = strtok(NULL, " ");
		token5 = strtok(NULL, " ");
		//CommandLine inputs are handled
		if(strcasecmp(token2,"update") == 0)
		{
			if (token5 != NULL)
			{
				token5[strlen(token5) - 1] = 0;
				check = updateTable(atoi(token3), atoi(token4), atoi(token5));
				if (check == 1)
				{
					cse4589_print_and_log("%s:SUCCESS\n", token2);
				}
				else
				{
					cse4589_print_and_log("%s:%s\n", token2, "Failed to execute");
				}
				
			}
			else
			{
				printf("Enter the right number of parameters\n");
			}
		
			
		}
		else if(strcasecmp(token2 ,"packets") == 0)
		{
			cse4589_print_and_log("%s:SUCCESS\n", token2);
			printf("The number of updates received: %d\n", noofpackets);
			noofpackets = 0;
		}
		else if(strcasecmp(token2,"step") == 0)
		{
			check = sendPackets();
			if (check == 1)
				{
					cse4589_print_and_log("%s:SUCCESS\n", token2);
				}
				else
				{
					cse4589_print_and_log("%s:%s\n", token2, "Failed to execute");
				}
		}
		else if(strcasecmp(token2,"display") == 0)
		{
			check = display();
			if (check == 1)
				{
					cse4589_print_and_log("%s:SUCCESS\n", token2);
				}
				else
				{
					cse4589_print_and_log("%s:%s\n", token2, "Failed to execute");
				}
		}
		else if(strcasecmp(token2,"disable") == 0)
		{
			if (token3 != NULL)
			{
				token3[strlen(token3) - 1] = 0;
				check = disableLink(atoi(token3));
				if (check == 1)
				{
					cse4589_print_and_log("%s:SUCCESS\n", token2);
				}
				else
				{
					cse4589_print_and_log("%s:%s\n", token2, "Failed to execute");
				}
			}
			else
			{
				printf("Enter the right number of parameters\n");
			}
		
		}
		else if(strcasecmp(token2, "crash") == 0)
		{
			check = crash(token2);
			if (check == 1)
			{
				cse4589_print_and_log("%s:SUCCESS\n", token2);
			}
			else
			{
				cse4589_print_and_log("%s:%s\n", token2, "Failed to execute");
			}
		}
		else if(strcasecmp(token2,"dump") == 0)
		{
			int dumpsuc = cse4589_dump_packet(&routingpacket, sizeof routingpacket);
			if (dumpsuc < 0)
			{
				cse4589_print_and_log("%s:%s\n", token2, "Failed to execute");
			}
			else
			{
				cse4589_print_and_log("%s:SUCCESS\n", token2);
			}
		
		}
		else if(strcasecmp(token2,"academic_integrity") == 0)
		{
			cse4589_print_and_log("I have read and understood the course academic integrity policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity");
		}
		else
		{
			printf("Enter the right commands\n");
		}
	}
	else if(FD_ISSET(listener, &read_fds)) //handling data from different servers Reference: http://stackoverflow.com/questions/3571038/how-to-identify-senders-ip-address-of-a-udp-packet-received-on-ios-device
	{
		if(crashed == 0)
		{
		uint32_t temp3;
		uint16_t recvedID,j;
		char ip4[INET_ADDRSTRLEN];
		char ip4V2[INET_ADDRSTRLEN];
		struct sockaddr_in receiver_addr;
		socklen_t addrlen = sizeof receiver_addr;
		int recvbytes = recvfrom(listener, &recvdpkt, sizeof recvdpkt, 0, (struct sockaddr *)&receiver_addr, &addrlen);
		if (recvbytes<0)
		{
			perror("Error in recvfrom\n");
			exit(1);
		}
		
		inet_ntop(AF_INET, &receiver_addr.sin_addr.s_addr, ip4, sizeof ip4);
	
		for (i = 0; i < noofservers; i++)
		{
			temp3 = (routingpacket.servers[i].server_IP);
			inet_ntop(AF_INET, &temp3, ip4V2, sizeof ip4V2);
			if (strcasecmp(ip4, ip4V2)==0)
			{
				for (j = 0; j < noofservers; j++)
				{
					if (ntohs(routingpacket.servers[i].server_ID) == neighbours[j][1])
					{
						started[i] = 1; 							//Starting the counter when the server receives a packet from the neighbour
					}
				}
				
				node[i] = 0;
				recvedID = ntohs(routingpacket.servers[i].server_ID);
				
			}
		}
		for (i = 0; i < noofservers; i++)
		{
			if (recvedID == neighbours[i][1])
			{
				noofpackets++;
				cse4589_print_and_log("RECEIVED A MESSAGE FROM SERVER %d\n", recvedID);
				bellmanFord(recvdpkt, recvedID);
				for (i = 0; i < noofservers; i++)
				{
					cse4589_print_and_log("%-15d­%-15d­\n", ntohs(recvdpkt.servers[i].server_ID), ntohs(recvdpkt.servers[i].cost));
				}
			}
		}
		//display();
		}
	}
}
	return 0;
}


/*Function to print the IP address of the machine which sends a UDP packet to a google dns server and requests for its own ip address and then prints it.*/
char* myip()
{
//Reference: http://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine
	struct addrinfo googleip, *goo;
	int sock;
	memset(&googleip, 0 , sizeof googleip);
	googleip.ai_family = AF_INET;
	googleip.ai_socktype= SOCK_DGRAM;
	getaddrinfo("8.8.8.8", "53", &googleip, &goo); // Connecting to a google dns server
	sock = socket(goo->ai_family, goo->ai_socktype, 0);
 			if(sock<0)
	{
		perror("socket error:");
	}
	int err = connect(sock, goo->ai_addr, goo->ai_addrlen);
	if(err<0)
	{
		perror("connect error:");
	}
	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	int e = getsockname(sock, (struct sockaddr *) &name, &namelen); // Fetching the ip address of the machine
	if(e<0)
	{
		perror("sockname error:");
	}
	
	return inet_ntoa(name.sin_addr);
	close(sock);
} 
//Reference: http://stackoverflow.com/questions/9206091/going-through-a-text-file-line-by-line-in-c
// Reads the topology file and initializes all the different structs
int createTable(char *filename, char *IP) 
{
	int port;
	int lineno = 1;
	int serv = 1;
	int edges = 1;
	FILE* file = fopen(filename, "r"); /* should check the result */
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        if(lineno ==1)
        {
        	noofservers = atoi(line);
        }
        if (lineno == 2)
        {
        	noofedges = atoi(line);
        }
        if (serv<=noofservers && lineno > 2)
        {
        	uint32_t temp;
        	server_Table[serv-1].serverid = atoi(strtok(line, " "));
        	routingpacket.servers[serv-1].server_ID = htons(server_Table[serv-1].serverid);
        	strcpy(server_Table[serv-1].serverip, strtok(NULL, " "));
        	int q = inet_pton(AF_INET, server_Table[serv-1].serverip, &temp);
        	routingpacket.servers[serv-1].server_IP = (temp);
			if (q<0)
			{
				perror("Error in routing packet \n");
			}
        	server_Table[serv-1].portno = atoi(strtok(NULL, " "));
        	routingpacket.servers[serv-1].server_port = htons(server_Table[serv-1].portno);
        	serv++;
        }
        if (edges<=noofedges && lineno > 2 + noofservers)
        {
        	int a;
        	src = atoi(strtok(line, " "));
        	a = atoi(strtok(NULL, " "));
        	 neighbours[a-1][1] = a;
        	route.nexthopID[a-1] = a;
        	neighbours[a-1][2] = atoi(strtok(NULL, " "));
        	routingpacket.servers[a-1].cost = htons(neighbours[a-1][2]);
        	route.mincost[a-1] = neighbours[a-1][2];
        	costFromTable[a-1] = neighbours[a-1][2];
        	edges++;
        }

        lineno++;
    }


    fclose(file);
    int i;
    for (i = 0; i < noofservers; i++)
    {
    	if(strcmp(server_Table[i].serverip, myip())==0)
    	{
    		port = server_Table[i].portno;
    	}
    }
return port;

}

//Initializes the routing packet

int createRoutingTable()
{
	uint32_t temp2;
	routingpacket.updatefields = htons(noofservers);
	routingpacket.serverport = htons(listeningport);
	int y = inet_pton(AF_INET, myip(), &temp2);
	routingpacket.serverIP = (temp2);
	if (y<0)
	{
		perror("Error in routing table\n");
		return 0;
	}

	int i;
	for (i = 0; i < noofservers; i++)
	{
		route.destinationID[i] = i+1;
		if (ntohs(routingpacket.servers[i].cost) == 0 && ntohs(routingpacket.servers[i].server_port) != listeningport)
		{
			routingpacket.servers[i].cost = htons(inf);
			neighbours[i][2] = inf;

		}
	}

		for (i = 0; i < noofservers; i++)
		{

			if (route.mincost[i] == 0 && route.destinationID[i] != src)
			{
				costFromTable[i] = inf;
				route.nexthopID[i] = -1;
				route.mincost[i] = inf;
			}
			if (route.destinationID[i] == src)
			{
				route.nexthopID[i] = src;
			}
		}
	return 1;
}

//Function to update the routing packet


int updateTable(int serverid1, int serverid2, int updatedcost)
{
	int j;
	for (j = 0; j < noofservers; j++)
	{
		if (serverid1 == src && serverid2 == neighbours[j][1])
		{
			neighbours[j][2] = updatedcost;
		}
		else
		{

		}
		if (serverid2 == ntohs(routingpacket.servers[j].server_ID))
		{
			costFromTable[j] = updatedcost;
			route.mincost[j] = updatedcost;
			routingpacket.servers[j].cost = htons(updatedcost);
		}
	}
	return 1;
}
// Function to display the routing table
int display()
{
	int i,j;
	/*printf("Neighbours: \n");
	for (i = 0; i < noofservers; i++)
	{
		printf("%-15d%-15d%-15d%-15d%-15d\n",costs[i][0],costs[i][1],costs[i][2],costs[i][3],costs[i][4] );
	}*/
	printf("destinationID 	nexthopID   mincost\n"); 
	for (i = 0; i < noofservers; i++)
	{
		cse4589_print_and_log("%-15d­ %-15d­ %-15d\n", route.destinationID[i], route.nexthopID[i], route.mincost[i]); 
	}
	return 1;
}

//Funtion to send the routing packet to all the neighbours

int sendPackets()
{
	int empty = 0;
	int IDCrashed;
	sender = socket(AF_INET, SOCK_DGRAM, 0); // sending socket
	if(sender<0)
	{
		perror("sender socket error\n");
		return 0;
	}
	struct sockaddr_in router_addr;
	bzero((char *) &router_addr, sizeof(router_addr));
	router_addr.sin_family = AF_UNSPEC;
	
	int i,k;

	for (i = 0; i < noofservers; i++)
	{
		if (neighbours[i][2] != inf)
		{
			int j;
			for (j = 0; j < noofservers; j++)
			{
				if(server_Table[j].serverid == neighbours[i][1])
				{
					int y = inet_pton(AF_INET, server_Table[j].serverip, &router_addr.sin_addr);
					router_addr.sin_port = htons(server_Table[j].portno);
					int n = sendto(listener, &routingpacket, sizeof(routingpacket), 0, (struct sockaddr *)&router_addr, sizeof (router_addr));
					if (n<0)
					{
						perror("Send to failed:\n");
						return 0;
					}
					if (started[i] == 1)
					{

						node[j]++;
					//	printf("NODE %d %d\n",j+1,node[j] );
						if (node[j]>3)
						{
							IDCrashed = j+1;
							for (k = 0; k < noofservers; k++)
							{
								if (neighbours[k][2] == inf)
								{
									empty = 1;
								}
								else
								{
									empty = 0;
								}
							}
							if (empty == 1)
							{
								for (k = 0; k < noofservers; k++)
								{
									if (k+1 != src)
									{
										route.mincost[k] = inf;
										route.nexthopID[k] = -1;
									}
			
								}
							}
							costFromTable[j] = inf;
							neighbours[j][2] = inf;
							route.mincost[j] = inf;
							route.nexthopID[j] = -1;
							routingpacket.servers[j].cost = htons(route.mincost[j]);
							for (k = 0; k < noofservers; k++)
							{
								costs[IDCrashed-1][k] = inf;
							}
						}
					}
				}
			}
		}
	}
	return 1;
}


//Funtion which applies the bellford algorithm and computes the routing table
int bellmanFord(packet recvdpkt, int receivedID)
{
	counter++;
	int min[5];

	int i,j,k;

	for (i = 0; i < noofservers; i++)
	{
		min[i] = inf;
	}
			
			for (i = 0; i < noofservers; i++)
			{
			//	printf("RECEIVED cost: %d Neighbours cost: %d\n", ntohs(recvdpkt.servers[i].cost), neighbours[receivedID-1][2]);
				if (ntohs(recvdpkt.servers[i].cost) != inf)
				{
					costs[receivedID-1][i] = ntohs(recvdpkt.servers[i].cost) + neighbours[receivedID-1][2];
				}
				else if (ntohs(recvdpkt.servers[i].cost) == inf && counter > 3)
				{
					costs[receivedID-1][i] = inf;
					route.mincost[i] = inf;
					routingpacket.servers[i].cost = htons(inf);
					route.nexthopID[i] = -1;
				}

			}
			for (k= 0; k < noofservers; k++)
			{
				for (j = 0; j < noofservers; j++)
				{
					if(costs[j][k] != 0)
					{
						if (min[k] > costs[j][k])
					 	{
					 		min[k] = costs[j][k];
					 		if (k+1 != src)
					 		{
					 			route.nexthopID[k] = j+1;
					 		}
					 		
					 	}
					}
				}
			}
			for (i = 0; i < noofservers; i++)
			{
				if (i+1 != src)
				{
					if (min[i] < costFromTable[i])
					{
						route.mincost[i] = min[i];
						routingpacket.servers[i].cost = htons(route.mincost[i]);
					}
					else
					{
						route.mincost[i] = costFromTable[i];
						routingpacket.servers[i].cost = htons(route.mincost[i]);
					}
				}
				
				
			}
	return 1;
}


//Funtion to disable the link to a particular server which is a neighbour
int disableLink(int ID)
{
	int i, ya = 0;
	for (i = 0; i < noofservers; i++)
	{
		costs[ID-1][i] = inf;
	}
	for (i = 0; i < noofservers; i++)
	{
		if (neighbours[i][1] == ID)
		{
			neighbours[i][1] = 0;
			neighbours[i][2] = inf;
			routingpacket.servers[ID-1].cost = htons(inf);
			costFromTable[ID-1] = inf;
			route.mincost[ID-1] = inf;
			route.nexthopID[ID-1] = -1;
			ya = 1;
		}
		
	}
	if (ya == 0)
	{
		printf("This server is not a neighbour!\n");
		return 0;
		
	}
	return 1;
}

//Function to crash the server

int crash(char *command)
{
	int i,loop= 0;
	for (i = 0; i < noofservers; i++)
	{
		neighbours[i][2] = inf;
	}
	crashed = 1;
	cse4589_print_and_log("%s:SUCCESS\n", command);
	while(loop == 0)
	{}
	return 1;
}

