//
//  SMD_SocketOps.h
//  SMD_Control_Server
//
//  Created by John Davis on 10/3/15.
//  Copyright © 2015 3ML LLC. All rights reserved.
//

#ifndef SMD_SocketOps_h
#define SMD_SocketOps_h

/**
	@fn void open_server_socket()
	@brief Opens the server socket specified by SERVER_PORT
 */
void open_server_socket();

/**
	@fn int parse_socket_input(char *input, int cl)
	@brief Parses raw input from the connected clients
	@param input Raw input string from the client
	@param cl Socket handle for the client socket
	@ return int 0 if successful
 */
int parse_socket_input(char *input, int cl);

/**
	@fn void parse_smd_response_to_client_input(int smd_response, char *input, int fd, int cl)
	@brief Reads the status back from the raw Modbus commands and translates them into return codes that can be related back to the client.
	@param smd_response Response back from the Modbus command
	@param cl Socket handle for the client socket
	@ return int 0 if successful
 */
void parse_smd_response_to_client_input(int smd_response, char *input, int fd, int cl);

#endif /* SMD_SocketOps_h */
