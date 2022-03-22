#include "myString.h"

int my_strlen(const char * str){
    if(str == NULL)
        return -1;
    int count = 0;
    for(;str[count] != '\0';count++){

    }
    return count;
}
char* my_strcpy(char * to,char* from){
    return strReplace(to,0,my_strlen(to),from);
}
int strcpy_index(char * to,int start1,int end1,char* from,int start2){
    if(start1 >= end1){
        return -1;
    }
    for(int i=start1;i<end1;i++){
        to[i] = from[start2++];
    }
    return 0;
}

int my_strcmp(const char * str1,const char* str2){
    if(str1 == str2){
        return 0;
    }
    if(str1 == NULL)
        return -1;
    else if(str2 == NULL)
        return 1;
    
    for(int i=0;;i++){
        int val = str1[i] - str2[i];
        if(val != 0){
            return val; 
        }
        if(str1[i] == '\0'){
            if(str2[i] == '\0'){
                return 0;
            } else{
                return -1;
            }
        }
        else if(str2[i] == '\0'){
            if(str1[i] == '\0'){
                return 0;
            } else{
                return 1;
            }
        }
    }

}
/*
    if searched element is in the str , returns the start index of it in str 
    else if input is valid but str does not contain searched, returns -1
    returns -2 if one or both of the given parameters is/are NULL
*/
int my_strcontains(const char *str,const char* searched){
    if(searched == NULL || str == NULL){
        return -2;
    }
    if(str[0] == '\0' && searched[0] == '\0'){
        return 0;
    }
    int searched_len = my_strlen(searched);
    for(int i=0;str[i] != '\0';i++){
        for(int j=0;j<searched_len+1;j++){
            
            if(j == searched_len){
                return i;
            }
            if(str[i+j] != searched[j]){
                break;
            }
        }
        
    }
    return -1;
}
int my_strcontains_ins(const char *str,const char* searched){
    if(searched == NULL || str == NULL){
        return -2;
    }
    if(str[0] == '\0' && searched[0] == '\0'){
        return 0;
    }
    int searched_len = my_strlen(searched);
    for(int i=0;str[i] != '\0';i++){
        for(int j=0;j<searched_len+1;j++){
            
            if(j == searched_len){
                return i;
            }
            // if the characters are not same and
            //      if the characters are not alphabetic then break
            //      else check if it is caused by case difference
            if(str[i+j] != searched[j] && !sameCharacterButCaseDifference(str[i+j], searched[j])){
                break;
            }
        }
        
    }
    return -1;
}
int sameCharacterButCaseDifference(char c1,char c2){
    return (alphabetic(c1) && alphabetic(c2) && (changeCase(c2) == c1));
}
int upperCase(char c){
    return (c >= 'A' && c<='Z');
}
int lowerCase(char c){
    return (c >= 'a' && c<='z');
}
int alphabetic(char c){
    return (lowerCase(c) ||  upperCase(c));
    
}
char changeCase(char c){ // hatalı alphabetic mi ona bak
    if(upperCase(c)){
        return c - 'A' +'a';
    } else if(lowerCase(c)){
        return c - 'a' + 'A';
    } else{ // not alphabetic 
        return c;
    }
}

int ** startandEndIndexes(char* str,int str_size,char *searched,char caseInsensitive,char bool_first_readed){
    if(searched == NULL || str == NULL){
        return NULL;
    }
    
    // will hold start indexes of found words
    int * wideArrayStart = (int*) malloc(sizeof(int) * str_size);
    if(wideArrayStart == NULL){
        perror("malloc error");
        return NULL;
    }
    // will hold end(exclusive) indexes of found words
    int * wideArrayEnd = (int*) malloc(sizeof(int) * str_size);
    if(wideArrayEnd == NULL){
        free(wideArrayStart);
        perror("malloc error");
        return NULL;
    }
   
    int searched_len = my_strlen(searched);
    int count = 0;
    char onlyAtLineStarts = (searched[0] == '^');
    char onlyAtLineEnds = (searched[searched_len-1] == '$');
    char lastCharacterFound; // will be used for * operator
    for(int i=0;i<str_size;i++){
        int lengthOfReplacedElement = 0;
        
        int indexInStr = i;

        for(int j=0;j<searched_len+1;j++){
            
            
            if(j == searched_len){ // word is found.
                // save start index
                if(bool_first_readed == 0 && onlyAtLineStarts){
                    //wideArrayStart[count] = i+1; // \n should not be replaced, ignore first character which is \n, 
                      lengthOfReplacedElement--;                           //if first_readed string is being processed, then there is no ignore operation
                } 
                wideArrayStart[count] = i;
                // save end index
                wideArrayEnd[count] = i+lengthOfReplacedElement; // end index 
                i = wideArrayEnd[count]-1;

                count++;
                // searching of next element will be started from at the end of found element.
                break;
            }

            if(indexInStr>=str_size){ // buffer may cut the wanted word from half
                fprintf(stderr,"Buffer may be cut the searched word from half.Unfortunately, this feature is not implemented as mentioned in the report.\n");
            }
            
            
            if(str[indexInStr] != searched[j]){
                if(searched[j] == '*'){
                    // if there is only 1 character:
                    if(str[indexInStr] != lastCharacterFound){
                        //j++; // do not search for this element
                        indexInStr--; // ask for character which is not in replaced string but in text 
                        lengthOfReplacedElement--;
                    } else{
                        indexInStr++;
                        while( str[indexInStr]!='\0' && (str[indexInStr] == lastCharacterFound || (caseInsensitive == 1 && 
                        sameCharacterButCaseDifference(lastCharacterFound,str[indexInStr])))){
                            indexInStr++;
                            lengthOfReplacedElement++;
                        }
                        indexInStr--;
                    }
                    // increment until different element is found, if case insensitive check for other case too

                }
                // handle square bracket
                
                else if(searched[j] == '['){
                    
                    j++;
                    char foundBTWBrackets = 0;
                    for(;searched[j] != ']';j++){
                        /*
                        if(searched[j] == '\0'){
                            perror("input is not correct, it must have been detected earlier");
                            return NULL;
                        } else*/ 
                        if(searched[j] == str[indexInStr] || (caseInsensitive == 1 && sameCharacterButCaseDifference(searched[j], str[indexInStr]))){
                            // letter found between square brackets
                            lastCharacterFound = str[indexInStr];
                            foundBTWBrackets = 1; 
                            // program should continue as if no square bracket is encountered.
                            // j should be incremented to index of ]
                            for(;searched[j] != ']';j++);
                            
                            // then next element should be searched.
                            break;

                            
                        }

                    }
                    if(!foundBTWBrackets){
                        if(searched[j + 1] != '*'){
                            break;
                        }
                        j++; // do not search for this element
                        indexInStr--; // ask for character which is not in replaced string but in text 
                        lengthOfReplacedElement--; // * is not in the text so do not increment replaced string's size
                        //break;
                    }
                } else if(j == 0 && onlyAtLineStarts && ((bool_first_readed == 1 && indexInStr == 0) || (indexInStr != 0 && str[indexInStr-1] == '\n'))){
                    if(bool_first_readed == 1){
                        // ^ is not matched with \n so the last character in str must be tried to be matched again with the next searched character 
                        //indexInStr--;
                        lengthOfReplacedElement--; // if it is first readed string length of replacedElement should not be incremented
                    }
                    indexInStr--;
                    // \n character should not be replaced
                    // we will take care of it when indexes saved which is in the first if statement of for statement.
                }
                else if(j == searched_len-1 && onlyAtLineEnds && ((str[indexInStr] == '\n' || str[indexInStr] == EOF))){
                    
                    lengthOfReplacedElement--;

                } 
                // same character different case is found
                else if(caseInsensitive == 1 && sameCharacterButCaseDifference(str[indexInStr],searched[j])){
                   lastCharacterFound = str[indexInStr];

                    
                }
                // if it is not special operator and replace operation is case insensitive check if only case is different
                else if(caseInsensitive == 0 || !sameCharacterButCaseDifference(str[indexInStr],searched[j])){
                    // if letter is not found and next element is not *, then break;
                    if(searched[j + 1] != '*'){
                        break;
                    }
                    j++; // do not search for this element
                    indexInStr--; // ask for character which is not in replaced string but in text 
                    lengthOfReplacedElement--; // * is not in the text so do not increment replaced string's size

                    
                }
                else{ // same character different case is found
                    
                    lastCharacterFound = str[indexInStr];
                }
                
            } else{ // exact letter is found
                
                lastCharacterFound = str[indexInStr];
            }
            indexInStr++;
            lengthOfReplacedElement++;
            
        }
        
    }

    if(count==0){
        free(wideArrayStart);
        free(wideArrayEnd);
        return NULL;
    } 
    int * indexesStart = (int*) malloc(sizeof(int) * (count+1));
    if(indexesStart == NULL){
        perror("malloc error:");
        return NULL;
    }
    int * indexesEnd = (int*) malloc(sizeof(int) * (count+1)); // there is no -1 at the end
    if(indexesEnd == NULL){
        free(indexesStart);
        perror("malloc error:");
        return NULL;
    }


    for(int i=count-1;i>=0;i--){
        indexesStart[count-1-i] = wideArrayStart[i];
        indexesEnd[count-1-i] = wideArrayEnd[i];

    }
    
    free(wideArrayStart);
    free(wideArrayEnd);
    indexesStart[count] = -1; // end of array
    indexesEnd[count] = -1; // end of array

    int ** indexes = (int**) malloc(sizeof(int*) * 2); // start indexes and end indexes
    if(indexes == NULL){
        free(indexesStart);
        free(indexesEnd);
        perror("malloc error:");
        return NULL;
    }
    indexes[0] = indexesStart;
    indexes[1] = indexesEnd;

    
    return indexes;
}
char * strReplace(char* source,int start, int end,char* inserted){
    int len = my_strlen(inserted);
    int replacedLen = end-start;
    int difference = replacedLen- len;
    int sourceSize = my_strlen(source);
    if(replacedLen >= len){ // no need to extend source
        for(int i=0;i<len;i++){
            source[start+i] = inserted[i];
        }
        int i=0;
        
        for(i=start+len;source[i] != '\0';i++){
            source[i] = source[i+difference];
        }
        return source;
    
    } else{ // replacedLen < len, so bigger space is needed
        int sizeNeeded = sourceSize - replacedLen + len; 
        // +1 is needed for \0
        char * created = (char *) malloc(sizeof(char)*(sizeNeeded+1)); // initializes the space no need to put \0 at the end
        // malloc error check
        if(created == NULL){
            perror("malloc error: ");
            return NULL;
        }
        created[sizeNeeded] = '\0';
        // until start point,put same elements
        for(int i=0;i<start;i++){
            created[i] = source[i]; 
        }
        // from start point, until start+len, 
        //insert the value wanted to be inserted
        for(int i=start;i<len+start;i++){ // from start point, until start+len, insert the value wanted to be inserted
            created[i] = inserted[i-start];
        }
        
        for(int i=len+start;source[end] !='\0';i++){
            created[i] = source[end++];
        }
        return created;
    }
}
int my_numberOf(char* str,char searched){
    if(str == NULL){
        return 0;
    }
    int count=0;
    for (int i = 0; str[i]!='\0'; i++)
    {
        if(str[i] == searched)
            count++;
    }
    return count;
    
}

char ** my_split(char* str,char separator){
    int numberOfSeparators = my_numberOf(str,separator);
    // if x number of separators exist, it will have x+1 element. And its last element will be NULL so numberOfSeparators+2 space is needed.
    char ** splitted = (char **) malloc(sizeof(char *) * (numberOfSeparators+2));
    if(splitted == NULL){
        perror("malloc error:");
        return NULL;
    }
    // returned string array has NULL element at last index

    for (int i = 0; i < numberOfSeparators +2; i++)
    {
        splitted[i] = NULL;
    }
    
    //splitted[numberOfSeparators+1] = NULL; 
    int count = 0;
    for(int i=0,start = 0;;i++){
        if(str[i] == separator || str[i] == '\0'){
            int size = i-start; // +1 is for \0
            // if separator is used continuosly, do not add them
            if(i-start == 0){ 
                if(str[i] == '\0') // if it is end of file then there is no more element so break
                    break;
                else{
                    start++;
                    continue;
                }
            }
            char * element = (char *) malloc(sizeof(char) * (size+1));
            if(element == NULL){
                perror("malloc error:");
                return NULL;
            }
            strcpy_index(element,0,size,str,start);
            element[size] = '\0';
            
            splitted[count] = element;
            count++;
            start = i+1;
            if(str[i] == '\0'){
                break;
            }
        }
    }
    return splitted;
}
