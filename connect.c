#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

char** game; //this acts as the board for the game 
int boardWidth;
int boardHeight;
char playerName[2];
//an integer that flips between 0 and 1 to indicate if you can go or not
int canGo;
//global server_info to free in the teardown
struct addrinfo *server_info;
//the other player's socket
int other_socket;
void init(int argc, char** argv){
   //set up server
   printf("%d\n", argc);
   if(argc == 2)
   {
		int server_socket;
		struct addrinfo server_hints, *p;
		memset(&server_hints, 0, sizeof(server_hints));
		//set up hints
		server_hints.ai_flags = AI_PASSIVE;
		server_hints.ai_family = PF_INET;
		server_hints.ai_socktype = SOCK_STREAM;
		server_hints.ai_protocol = IPPROTO_TCP;
		
		getaddrinfo(NULL, argv[1], &server_hints, &server_info);
		
		//server steps
		for(p = server_info; p != NULL; p->ai_next)
		{
		   //socket
			if ((server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			{
				perror("socket error\n");
				exit(1);
			}
			//bind
			if (bind(server_socket, p->ai_addr, p->ai_addrlen) < 0)
			{
				perror("bind error\n");
				close(server_socket);
				exit(1);
			}
			break;
		}
		
		//print address and port
		printf("Address: %s\n", inet_ntoa(((struct sockaddr_in *) server_info->ai_addr)->sin_addr));
		printf("Port: %s\n", argv[1]);
		
		//listen
		if (listen(server_socket, 1) < 0)
		{
			perror("listen error\n");
			exit(1);
		}
		
		struct sockaddr_storage client_address_storage;
		int size = sizeof(client_address_storage);
		
		printf("Waiting for connection...\n");
		//accept and set other_socket to player 2
		if((other_socket = accept(server_socket, (struct sockaddr*) &client_address_storage, &size)) < 0)
		{
			perror("accept error\n");
			exit(1);
		}
		printf("Connected!\n");
		
		canGo = 1;
   }
   //set up client
   else if(argc > 2)
   {
      //print address and port
		printf("Address: %s\nPort: %s\n", argv[1], argv[2]);
		
		struct addrinfo server_hints, *p;
		memset(&server_hints, 0, sizeof(server_hints));
		//set up hints
		server_hints.ai_family = PF_INET;
		server_hints.ai_socktype = SOCK_STREAM;
		server_hints.ai_protocol = IPPROTO_TCP;
		
		getaddrinfo(argv[1], argv[2], &server_hints, &server_info);
		
		//client steps
		for(p = server_info; p != NULL; p->ai_next)
		{
		   //socket and set other_socket to player 1
			if((other_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			{
				perror("socket error\n");
				exit(1);
			}
			printf("Attempting to connect...\n");
			//connect
			if(connect(other_socket, p->ai_addr, p->ai_addrlen) < 0)
			{
				perror("connect error\n");
				close(other_socket);
				exit(1);
			}
			printf("Connected!\n");
			break;
		}
		canGo = 0;
   }
   //if incorrect arguments, exit
   else
   {
      perror("Missing Port Number!");
      exit(0);
   }
   boardWidth = 7; //gives the board 7 vertical slots
   boardHeight = 6; //and this gives it 6 horizontal slots
   game = (char**)malloc(boardWidth * sizeof(char*));
   int xAxis;
   int yAxis;
   for (xAxis=0; xAxis<boardWidth; xAxis++){
       game[xAxis] = (char*)malloc(boardHeight * sizeof(char));
       for(yAxis=0;yAxis<boardHeight;yAxis++){
           game[xAxis][yAxis] = '.';
       }
   } 
   playerName[0] = '!'; 
   playerName[1] = '@'; 
   return; //the "!" and "@" symbols act as the discs 
}
void display(){
   int xAxis, yAxis;
   for(xAxis=0;xAxis<boardHeight;xAxis++){
       printf("%d\t",xAxis+1);
   }
   printf("\n");
   for(xAxis=0;xAxis<boardWidth;xAxis++){
       for(yAxis=0;yAxis<boardHeight;yAxis++){
           printf("%c\t",game[xAxis][yAxis]);
       }
       printf("\n");
   }
   return;
}
void teardown(){ //will indicate that the game should break down, given certain parameters
   int xAxis;
   for(xAxis=0;xAxis<boardWidth;xAxis++){
       free(game[xAxis]);
   }
   free(game);
   freeaddrinfo(server_info);
   return; //helps to allocate memory for free space
}
void gameEnd(int win){ //varying results are given depending on which numbers are inserted
   if(win == 0){
       printf("Player 0 is the winner!\n");
   }
   if(win == 1){
       printf("Player 1 is the winner!\n");
   }
   if(win == 2){
       printf("Sorry bucko, that's the wrong move. Now you ended the game.\n"); //message that occurs if put in an incompatible number that doesn't fit within the size of the board.
   }
   display();
   teardown();
   exit(0);
}
int input(int x,int y){ //this allows the discs to be inserted in horizontal line

   int xAxis = x;
   int yAxis = y;
   int daBoard = 0; //blank template for the game board
   while(yAxis < boardHeight && game[xAxis][yAxis] == game[x][y]) daBoard++,yAxis++;
   yAxis = y-1;
   while(yAxis >= 0 && game[xAxis][yAxis] == game[x][y]) daBoard++,yAxis--;
   if(daBoard >= 4) return 1; //should 4 discs be matched in a hortizontal line, the player wins
  

   xAxis = x-1; //and this allows them to be assorted vertically
   yAxis = y;
   daBoard = 0;
   xAxis = x-1;
   while(xAxis >= 0 && game[xAxis][yAxis] == game[x][y]) daBoard++,xAxis--;
   xAxis = x;
   while(xAxis < boardWidth && game[xAxis][yAxis] == game[x][y]) daBoard++,xAxis++;
  
   if(daBoard >= 4){ //the same win condition, 4 discs matching vertically means you win
       return 1;
   }
  
   
   xAxis = x; //this incorporates diagonal lining for the discs, in this case it's diagonally left
   yAxis = y;
   daBoard = 0;
   while(xAxis < boardWidth && yAxis < boardHeight && game[xAxis][yAxis] == game[x][y]) daBoard++,xAxis++,yAxis++;
   xAxis = x-1;
   yAxis = y-1;
   while(xAxis >= 0 && yAxis >= 0 && game[xAxis][yAxis] == game[x][y]) daBoard++,xAxis--,yAxis--;
   if(daBoard >= 4) return 1;
  
   
   xAxis = x; //and for this it allows for diagonally right matching, which will give you the win if you match 4 of them
   yAxis = y;
   daBoard = 0;
   while(xAxis < boardWidth && yAxis >= 0 && game[xAxis][yAxis] == game[x][y]) daBoard++,xAxis++,yAxis--;
   xAxis = x-1;
   yAxis = y+1;
   while(xAxis >= 0 && yAxis < boardHeight && game[xAxis][yAxis] == game[x][y]) daBoard++,xAxis--,yAxis++;
   if(daBoard >= 4) return 1;
  
   
   return 0; //if there's no matching discs, the game just continues on as normal
}
void update(int player){ 
   printf("*Player %d's' turn*\n",player + 1); 
   int disc;
   //if it is your turn, accept input and then send the input to the other player
   if(canGo == 1)
   {
      printf("Put in a number to insert the disc: ");
      scanf("%d",&disc);
      if(send(other_socket, &disc, sizeof(disc), 0) < 0)
		{
			perror("send error\n");
			exit(1);
		}
   }
   //if it is not your turn, recieve input the other player made
   else
   {
      printf("Waiting for other player to make a move...\n");
      if(recv(other_socket, &disc, sizeof(disc), 0) < 0)
		{
			perror("recieve error\n");
			exit(1);
		}
   }
   //continue as normal
   disc--;
   if((disc < 0 || disc >= boardHeight) || game[0][disc] != '.'){
       gameEnd(2); //incorrect move, so the game just immediately ends
}
   int xAxis; 
   for(xAxis=boardWidth-1;xAxis>=0;xAxis--){
       if(game[xAxis][disc] == '.'){
           game[xAxis][disc] = playerName[player];
           int fin = input(xAxis,disc);
           if(fin == 1){
               gameEnd(player); 
           }
           break; //breaks since the game is technically over, everything has to reset
       }
   }
   return;
}
int main(int argc, char** argv){ //allows turns to operate correctly
   init(argc, argv);
   display();
   int turn;
   while(1){
       update(turn);
       display();
       turn = 1 - turn;
       //flip flop canGo
       canGo = 1 - canGo;
   }
   display();
   return 0;
}