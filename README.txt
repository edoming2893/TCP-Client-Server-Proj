The server program is done in c language. It should be compile with "gcc".

NOTE: the agen.c file was provided by the instructor // Texas State University DR.Mina Guirguis

// ************ Compile Instructions ****************

gcc server.c


// ************** Run the file ***************

The the server file is ran the log text file will be out put to the same directory that the server.c file is stored in.

NOTE: In order to run the program on 3 personal computers the following libraries needed to be added to the agent.c file in order to handle "close(sd)" line.

#include <fcntl.h>
#include <unistd.h>

However the agent file was able to run without them in eros and zeus without any issues. It is preferable to use the agent.c file provided with the server file since they are already included.


/.a.out port#



Example: /.a.out 20000