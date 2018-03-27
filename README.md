# Chat Room written in C

Simple chatroom application written in C based on client-server model.
The server can sustain multiple connections at once using the select() system call.

# How to get

Open terminal window .
Navigate to the directory you want to clone the project into.
Paste in the following command and you're good to go!
```git
git clone https://github.com/nlacko97/chat-room.git
```

# How to use

Build the project by running the `make` command.
Do not use the server.c file.
Run the server with ./select-server
From a separate command window, run the client with ./client
You can run multiple clients at once.
