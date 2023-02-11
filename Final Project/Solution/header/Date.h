#ifndef __DATE_H_
#define __DATE_H_
#include <stdlib.h>

typedef struct Date
{
    int day;
    int month;
    int year;
}Date;

// str must be in form : DD-MM-YYYY
void initializeDate(Date * date,char* str);
int dateCompare(Date *d1,Date *d2);
#endif