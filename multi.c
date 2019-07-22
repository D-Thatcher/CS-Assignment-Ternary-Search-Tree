#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <pthread.h>
#define MAX 50

// Ternary Search Tree (TST) insert, traverse
// and search operations

// A node of ternary search tree
struct Node
{
    char data;

    // True if this character is last character of one of the words
    unsigned isEndOfString: 1;

    struct Node *left, *eq, *right;
};

// A utility function to create a new ternary search tree node
struct Node* newNode(char data)
{
    struct Node* temp = (struct Node*) malloc(sizeof( struct Node ));
    temp->data = data;
    temp->isEndOfString = 0;
    temp->left = temp->eq = temp->right = NULL;
    return temp;
}

// Function to insert a new word in a Ternary Search Tree
void insert(struct Node** root, char *word)
{
    // Base Case: Tree is empty
    if (!(*root))
        *root = newNode(*word);

    // If current character of word is smaller than root's character,
    // then insert this word in left subtree of root
    if ((*word) < (*root)->data)
        insert(&( (*root)->left ), word);

        // If current character of word is greate than root's character,
        // then insert this word in right subtree of root
    else if ((*word) > (*root)->data)
        insert(&( (*root)->right ), word);

        // If current character of word is same as root's character,
    else
    {
        if (*(word+1))
            insert(&( (*root)->eq ), word+1);

            // the last character of the word
        else
            (*root)->isEndOfString = 1;
    }
}

// A recursive function to traverse Ternary Search Tree
void traverseTSTUtil(struct Node* root, char* buffer, int depth)
{
    if (root)
    {
        // First traverse the left subtree
        traverseTSTUtil(root->left, buffer, depth);

        // Store the character of this node
        buffer[depth] = root->data;
        if (root->isEndOfString)
        {
            buffer[depth+1] = '\0';
            printf( "%s\n", buffer);
        }

        // Traverse the subtree using equal pointer (middle subtree)
        traverseTSTUtil(root->eq, buffer, depth + 1);

        // Finally Traverse the right subtree
        traverseTSTUtil(root->right, buffer, depth);
    }
}

// The main function to traverse a Ternary Search Tree.
// It mainly uses traverseTSTUtil()
void traverseTST(struct Node* root)
{
    char buffer[MAX];
    traverseTSTUtil(root, buffer, 0);
}

// Function to search a given word in TST
int searchTST(struct Node *root, char *word)
{
    if (!root)
        return 0;

    if (*word < (root)->data)
        return searchTST(root->left, word);

    else if (*word > (root)->data)
        return searchTST(root->right, word);

    else
    {
        if (*(word+1) == '\0')
            return root->isEndOfString;

        return searchTST(root->eq, word+1);
    }
}

// End of Ternary Search Tree


// Checks if a string contains only alphabetical characters
int checkString( const char s[] )
{
    unsigned char c;

    while ( ( c = *s ) && ( isalpha( c ) || isblank( c ) ) ) ++s;

    return *s == '\0';
}


// Declare thread locking variables
pthread_mutex_t lock;
int j;

// Target function for the first thread
// Argument: Pointer to the Ternary Search Tree of commonwords.txt that is shared between threads
// Job: Writes the first half of the matches.txt file
void doFirstProcess(struct Node * root)
{
    // To avoid corrupting the output file, we need to lock the threads
    pthread_mutex_lock(&lock);
    
    // Open a file for writing the matches
    static const char writeFile[] = "matches.txt";

    FILE * matchFile = fopen(writeFile,"w+");
    if ( matchFile == NULL )
    {perror ( writeFile );}



    // Open the allwords file
    static const char allwordsFileName[] = "allwords.txt";
    FILE *allwordsFile = fopen ( allwordsFileName, "r" );
    int startLine = 0;
    if ( allwordsFile != NULL )
    {
        char line [ 128 ]; /* or other suitable maximum line size */
        while (( fgets ( line, sizeof line, allwordsFile ) != NULL )&& startLine < 150000) /* read a line */
        {
            startLine++;
            
            // Remove new-line delimiter
            line[strcspn(line, "\n")] = 0;
        
            // Store a string of matching common words delimited by commas. This is no longer used as it the substrings must be sorted after
            char matchingWords[2000];
            strcpy(matchingWords, "");
//
            char comma[2];
            strcpy(comma, ",");
            
            // Build the substring array
            char str[25][25],temp[25];
            int loc = 0;
            int n = strlen(line);
            for(int i = 0; i < n; i++)
            {   for(int j = i; j < n; j++)
                {   /* print substring from i to j */
                    char substring[50];
                    strcpy(substring, "");

                    for(int k = i; k <= j; k++)
                    {
                        char res = line[k];
                        char cpy[2];
                        memcpy(cpy, &res, sizeof(cpy));
                        cpy[1] = '\0';
                        strcat(substring, cpy);
                    }

                    // If the commonwords Ternary Search Tree contains this substring, add it to our substring char array
                    if(searchTST(root, substring)){
                        strcpy(str[loc], substring);
                        loc++;

                        strcat(matchingWords, substring);
                        strcat(matchingWords, comma);

                    }
                }
            }


            // Sort the substring char array alphabetically
            int start,mid;

            for(start=0;start<=loc;start++){
                for(mid=start+1;mid<=loc;mid++){
                    if(strcmp(str[start],str[mid])>0){
                        strcpy(temp,str[start]);
                        strcpy(str[start],str[mid]);
                        strcpy(str[mid],temp);
                    }
                }
            }
//
            // Reduce the char array down to a comma-delimited string
            char reduced[50];
            strcpy(reduced, "");
            for(int s=0;s<=loc;s++){
                if((strlen(str[s])>0)&&(checkString(str[s]))) {
                    strcat(reduced, str[s]);
                    strcat(reduced, comma);
                }
            }
            reduced[strlen(reduced)-1] = 0;
            
            // Clear the array from memory for re-use
            for(start=0;start<=loc;start++){
                strcpy(str[start], "");
            }


            matchingWords[strlen(matchingWords)-1] = 0;
            fputs(line, matchFile);
            fputs(": ", matchFile);
            fputs(reduced, matchFile);
            fputs("\n", matchFile);

            memcpy(reduced, "", sizeof(reduced));

        }
        fclose ( allwordsFile );
    }
    else
    {
        perror ( allwordsFileName );
    }
    pthread_mutex_unlock(&lock);
}


// Target function for the second thread
// Argument: Pointer to the Ternary Search Tree of commonwords.txt that is shared between threads
// Job: Writes the second half of the matches.txt file
void doSecondProcess(struct Node * root)
{
    // To avoid corrupting the output file, we need to lock the threads
    pthread_mutex_lock(&lock);


    static const char writeFile[] = "matches.txt";

    FILE * matchFile = fopen(writeFile,"a+");
    if ( matchFile == NULL )
    {perror ( writeFile );}




    static const char allwordsFileName[] = "allwords.txt";
    FILE *allwordsFile = fopen ( allwordsFileName, "r" );
    int startLine = 0;
    if ( allwordsFile != NULL )
    {
        char line [ 128 ]; /* or other suitable maximum line size */
        while ( fgets ( line, sizeof line, allwordsFile ) != NULL ) /* read a line */
        {
            startLine++;
            if(startLine>=150000){
                // Remove new-line delimiter
                line[strcspn(line, "\n")] = 0;

                // Store a string of matching common words delimited by commas
                char matchingWords[2000];
                strcpy(matchingWords, "");
//
                char comma[2];
                strcpy(comma, ",");

                char str[25][25],temp[25];
                int loc = 0;
                int n = strlen(line);
                for(int i = 0; i < n; i++)
                {   for(int j = i; j < n; j++)
                    {   /* print substring from i to j */
                        char substring[50];
                        strcpy(substring, "");

                        for(int k = i; k <= j; k++)
                        {
                            char res = line[k];
                            char cpy[2];
                            memcpy(cpy, &res, sizeof(cpy));
                            cpy[1] = '\0';
                            strcat(substring, cpy);
                        }


                        if(searchTST(root, substring)){
                            strcpy(str[loc], substring);
                            loc++;

                            strcat(matchingWords, substring);
                            strcat(matchingWords, comma);

                        }
                    }
                }


                // Sort the char array alphabetically
                int start,mid;

                for(start=0;start<=loc;start++){
                    for(mid=start+1;mid<=loc;mid++){
                        if(strcmp(str[start],str[mid])>0){
                            strcpy(temp,str[start]);
                            strcpy(str[start],str[mid]);
                            strcpy(str[mid],temp);
                        }
                    }
                }
//
                // Reduce the char array down to a comma-delimited string
                char reduced[50];
                strcpy(reduced, "");
                for(int s=0;s<=loc;s++){
                    if((strlen(str[s])>0)&&(checkString(str[s]))) {
                        strcat(reduced, str[s]);
                        strcat(reduced, comma);
                    }
                }
                reduced[strlen(reduced)-1] = 0;

                for(start=0;start<=loc;start++){
                    strcpy(str[start], "");
                }


                matchingWords[strlen(matchingWords)-1] = 0;
                fputs(line, matchFile);
                fputs(": ", matchFile);
                fputs(reduced, matchFile);
                fputs("\n", matchFile);

                memcpy(reduced, "", sizeof(reduced));
            }

        }
        fclose ( allwordsFile );
    }
    else
    {
        perror ( allwordsFileName );
    }
    pthread_mutex_unlock(&lock);

}


int main(){

    // Build the shared Ternary Search Tree
    struct Node *root = NULL;

    static const char filename[] = "commonwords.txt";
    FILE *file = fopen ( filename, "r" );
    if ( file != NULL )
    {
        char line [ 128 ]; /* or other suitable maximum line size */
        while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
        {
            // Remove new-line delimiter
            line[strcspn(line, "\n")] = 0;

            // Insert it
            insert(&root, line);
        }
        fclose ( file );
    }
    else
    {
        perror ( filename );
    }

    //

    int err;
    pthread_t t1, t2;

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("Mutex initialization failed.\n");
        return 1;
    }

    j = 0;

    // Create and start both threads
    pthread_create(&t1, NULL, doFirstProcess, root);
    pthread_create(&t2, NULL, doSecondProcess, root);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}




