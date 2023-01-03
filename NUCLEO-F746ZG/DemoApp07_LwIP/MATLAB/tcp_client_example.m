%% TCP CLIENT

%%% File info
%
% ************************************************************************
%
%  @file    tcp_client.m
%  @author  Adrian Wojcik    adrian.wojcik@put.poznan.pl
%  @version 1.0
%  @date    29-Mar-2022
%  @brief   Simple TCP client example
%
% ************************************************************************

client = tcpclient("192.168.1.66", 7);
write(client, uint8('Hello, Nucleo!'));

% Wait for response 
while client.NumBytesAvailable == 0
end
fprintf(char(read(client)))

% Wait for response 
while client.NumBytesAvailable == 0
end
fprintf(char(read(client)))

clear client