//
//  SMD_SocketOps.h
//  SMD_Control_Server
//
//  Created by John Davis on 10/3/15.
//  Copyright © 2015 3ML LLC. All rights reserved.
//

#ifndef SMD_SocketOps_h
#define SMD_SocketOps_h

void open_server_socket();
int parse_socket_input(char *input, int cl);
void parse_smd_response_to_client_input(int smd_response, char *input, int fd, int cl);

#endif /* SMD_SocketOps_h */
