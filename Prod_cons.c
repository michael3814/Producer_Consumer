#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFF_SIZE 10
#define buff_filled *filled
#define buff_empty *empty
#define buff_control *control
// Initalizing files
FILE *infile;
FILE *outfile;

//Initalizing buffers and variables
char buffer[BUFF_SIZE][1000];
char line[1000];

// Initalizing Semaphores
sem_t *control;
sem_t *filled;
sem_t *empty;

void *producer(void *arg);
void *consumer(void *arg);

int main() {
    
    // Creating named semaphores
    empty = sem_open("/buff_empty", O_CREAT, 0644, BUFF_SIZE);
    filled = sem_open("/buff_filled", O_CREAT, 0644, 0);
    control = sem_open("/buff_control", O_CREAT, 0644, 1);
    
    // Initalizing threads to be used
    pthread_t pro_t, con_t;
    
    // Opening Input and Output files
    char file[100];
    printf("Please input the name of the file that is to be read \n");
    scanf("%s", file);
    
    infile = fopen(file, "r");
    outfile = fopen("outfile.txt", "w");
    
    if (infile == NULL) {
        printf("Error opening input file\n");
        return 1;
    }
    
    // Creating threads
    
    if(pthread_create(&pro_t, NULL, producer, NULL)) {
        printf("\nError creating producer thread \n");
        return 1;
    }
    else
        printf("Producer created successfully\n");
    
    if(pthread_create(&con_t, NULL, consumer, NULL)) {
        printf("\nError creating consumer thread \n");
        return 1;
    }
    else
        printf("Consumer created successfully\n");
    
    // Joining threads
    
    if(pthread_join(con_t, NULL) != 0) {
        printf("\nError joining consumer thread \n");
        return 1;
    }
    else
        printf("Consumer joined successfully\n");
    
    // Closing Files
    printf("Program executed successfully \n"
           "File '%s' copied to 'outfile.txt' \n", file);
    
    
return 0;
}

void* producer(void* arg) {
    int i = 0;
    while(!feof(infile)) {
        sem_wait(&buff_empty);
        sem_wait(&buff_control);
        // Producing and Appending
        fgets(line, sizeof(line), infile);
        strcpy(buffer[i], line);
        printf("Produced \n"
                "Buffer[%d]: %s \n", i, buffer[i]);
        
        i = (i + 1) % BUFF_SIZE;

        sem_post(&buff_control);
        sem_post(&buff_filled);
    }
    // When the end of file is reached, repeat one last time to add end of file string
    sem_wait(&buff_empty);
    sem_wait(&buff_control);
   
    strcpy(buffer[i], "end_of_file\r\n");
    printf("Produced \n"
            "Buffer[%d]: %s \n", i, buffer[i]);
   
    
    sem_post(&buff_control);
    sem_post(&buff_filled);
    
    // Exit the thread after putting the end of file string into the buffer
    fclose(infile);
    return NULL;
}

void* consumer (void* arg) {
    int j = 0;
        // Line function
        while(strcmp(buffer[j], "end_of_file\r\n")) {
            
            sem_wait(&buff_filled);
            sem_wait(&buff_control);
        
            // Taking and Appending
            fputs(buffer[j], outfile);
            printf("Consumed \n"
                   "Buffer[%d]: %s \n", j, buffer[j]);
            
            strcpy(buffer[j], "");
            
            j = (j + 1) % BUFF_SIZE;
            
        sem_post(&buff_control);
        sem_post(&buff_empty);
            
    }

    fclose(outfile);
    return NULL;
}

