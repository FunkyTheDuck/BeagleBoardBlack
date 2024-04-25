#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//gets time from UTC-2
char* time_get() {
    time_t currentTime;
    struct tm *localTime;
    static char timeString[6]; 
    putenv("TZ=UTC-2");
    time(&currentTime);
    localTime = localtime(&currentTime);
    strftime(timeString, sizeof(timeString), "%H:%M", localTime);
    return timeString;
}