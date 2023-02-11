#include "../header/Date.h"
#include <string.h>


void initializeDate(Date * date,char* str){
    memset(date,0,sizeof(Date));
    // DD-MM-YYYY
    
    // convert '-' to '\0'
    str[2] = '\0';
    str[5] = '\0';
    // take first positions
    char * day = str;
    char * month = &str[3];
    char * year = &str[6];
    // convert to int and save
    date->day = atoi(day);
    date->month = atoi(month);
    date->year = atoi(year);

    /*char const * sep = "-";
    char * day_str = strtok(str,sep);
    date->day = atoi(day_str);

    char * day_str = strtok(NULL,sep);
    date->day = atoi(day_str);*/

}
int dateCompare(Date *d1,Date *d2){
    if(d1->year != d2->year){
        return d1->year - d2->year;
    }
    if(d1->month != d2->month){
        return d1->month - d2->month;
    }
    return d1->day - d2->day;
    //return ((d1->day > d2->day) ? 1:-1;
}