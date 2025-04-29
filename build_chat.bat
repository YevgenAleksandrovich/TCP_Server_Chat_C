@echo off
gcc TCP_server_chat.c -o server_chat.exe -lws2_32
gcc TCP_client_chat.c -o client_chat.exe -lws2_32
echo buil is successful!
pause
