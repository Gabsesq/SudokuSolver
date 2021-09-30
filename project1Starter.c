#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef int bool;
#define TRUE 1
#define FALSE 0

int sudokuPuzzle[9][9];

int colArray[9];
int rowArray[9];
int subgridArray[9];

void *colChecker(void *param);
void *rowChecker(void *param);
void *subgridChecker(void *param);

typedef struct{
    int topRow;
    int bottomRow;
    int leftColumn;
    int rightColumn;
} parameters;

int main(){
    pthread_t threads[27]; //workers threads
    bool puzzle_validity = TRUE; //check overall puzzle
    parameters * thread_parameter[27]; //parameters for threads
    
    for(int i=0;i<27;i++){
        thread_parameter[i] = (parameters *) malloc(sizeof(parameters)); //making memmory for threads
    }
    
    printf("-----------------------------------------------\n");
    printf("----------- S U D O K U   P U Z Z L E----------\n");
    printf("-----------------------------------------------\n");
    
    //reading the file
    FILE* fp = fopen("/Users/gabrielesquibel/Desktop/SudokuPuzzle.txt","r");
    char buffer[50];
    int idx = 0;
    while(fgets(buffer,50,fp) != NULL){
        sscanf(buffer, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",&sudokuPuzzle[idx][0],&sudokuPuzzle[idx][1],&sudokuPuzzle[idx][2],&sudokuPuzzle[idx][3],&sudokuPuzzle[idx][4],&sudokuPuzzle[idx][5],&sudokuPuzzle[idx][6],&sudokuPuzzle[idx][7],&sudokuPuzzle[idx][8]);
        idx++;
    }
    for(int i=0;i<9;i++)
    {
        for(int j=0;j<9;j++)
        {
            if( sudokuPuzzle[i][j] > 9 || sudokuPuzzle[i][j] < 1)//if there is an invalid entry, exit
            {
                printf("INVALID ENTRY \n");
                exit(1);
            }
        }
    }
    
    
    //displaying the puzzle
    printf("-----------------------------------------------\n");
    for(int i=0;i<9;i++){
        for(int j=0;j<9;j++){
            printf("%d    ",sudokuPuzzle[i][j]);
        }
        printf("\n");
    }
    printf("-----------------------------------------------\n");
    
    //init the rows and columns
    
    for(int i=0;i<9;i++){
        thread_parameter[i]->topRow = 0;
        thread_parameter[i]->bottomRow = 8;
        thread_parameter[i]->leftColumn = i;
        thread_parameter[i]->rightColumn = i;
        pthread_create(&threads[i],NULL, colChecker, thread_parameter[i]); //creating the threead
        
        thread_parameter[9+i]->topRow = i;
        thread_parameter[9+i]->bottomRow = i;
        thread_parameter[9+i]->leftColumn = 0;
        thread_parameter[9+i]->rightColumn = 8;
        pthread_create(&threads[9+i],NULL, rowChecker, thread_parameter[9+i]);
    }
    
    idx = 0;
    for(int i=0;i<9;i+=3){ //row, skipping 3 rows for the next grid
        for(int j=0;j<9;j+=3,idx++){ //col, skipping 3 cols for next grid
            thread_parameter[18+idx]->topRow = i;
            thread_parameter[18+idx]->bottomRow = i+2;
            thread_parameter[18+idx]->leftColumn = j;
            thread_parameter[18+idx]->rightColumn = j+2;
            pthread_create(&threads[18+idx],NULL, subgridChecker, thread_parameter[18+idx]);
        }
    }
    
    //joining threads
    for(int i=0;i<27;i++){
        pthread_join(threads[i],NULL);
    }
    
    //checking the results
    
    for(int i=0;i<9;i++){
        if(puzzle_validity == TRUE && colArray[i] == FALSE){ //if single invalid entry is found
            puzzle_validity = FALSE; //makes the puzzle false
        }
        if(colArray[i] == TRUE){ //if column entry is true
            printf("column %d %lx valid\n",i,threads[i]); //display it as valid
        }
        else{
            printf("column %d %lx invalid\n",i,threads[i]); //display it as invalid
        }
    }
    
    for(int i=0;i<9;i++){
        if(puzzle_validity == TRUE && colArray[i] == FALSE){
            puzzle_validity = FALSE;
        }
        
        if(rowArray[i] == TRUE){
            printf("row %d %lx valid\n",i,threads[i]);
        }
        else{
            printf("row %d %lx invalid\n",i,threads[i]);
        }
    }
    
    for(int i=0;i<9;i++){
        if(puzzle_validity == TRUE && colArray[i] == FALSE){
            puzzle_validity = FALSE;
        }
        if(subgridArray[i] == TRUE){
            printf("sub_grid %d %lx valid\n",i,threads[i]);
        }
        else{
            printf("sub_grid %d %lx invalid\n",i,threads[i]);
        }
    }
    
    //displaying overall puzzle stats
    if(puzzle_validity == TRUE){
        printf("sudoku puzzle: valid\n");
    }
    else{
        printf("sudoku puzzle: invalid\n");
    }
    
    for(int i=0;i<27;i++){
        free(thread_parameter[i]);
    }
    
    return 0;
}


void *colChecker(void *param)
{
    parameters * params = (parameters *) param; //parameters from void to struct
    pthread_t thread_id = pthread_self(); //getting the current thread id
    for(int i=0;i<10;i++)
    {
        bool digit_occured = FALSE;
        for(int j=params->topRow;j <= params->bottomRow;j++)
        {
            if(i == sudokuPuzzle[j][params->leftColumn])
            {
                if(digit_occured == TRUE) //digit was already found
                {
                    colArray[params->leftColumn] = FALSE;
                    return NULL;
                }
                digit_occured = TRUE;
            }
        }
    }
    
    colArray[params->leftColumn] = TRUE;
    return NULL;
}

void *rowChecker(void *param)
{
    parameters * params = (parameters *) param;
    pthread_t thread_id = pthread_self();
    for(int i=0;i<10;i++)
    {
        bool digit_occured = FALSE;
        for(int j=params->leftColumn;j <= params->rightColumn;j++)
        {
            if(i == sudokuPuzzle[params->topRow][j])
            {
                if(digit_occured == TRUE)
                {
                    rowArray[params->topRow] = FALSE;
                    return NULL;
                }
                digit_occured = TRUE;
            }
        }
    }
    
    rowArray[params->topRow] = TRUE;
    return NULL;
}

void *subgridChecker(void *param)
{
    parameters * params = (parameters *) param;
    pthread_t thread_id = pthread_self();
    int grid_index = ((params->topRow / 3) * 3 + (params->leftColumn / 3)); //calculating the grid id
    for(int i=0;i<10;i++)
    {
        bool digit_occured = FALSE;
        for(int j=params->topRow;j <= params->bottomRow;j++)
        {
            for(int k = params->leftColumn;k<=params->rightColumn;k++)
            {
                if(i == sudokuPuzzle[j][k])
                {
                    if(digit_occured == TRUE)
                    {
                        subgridArray[grid_index] = FALSE;
                        return NULL;
                    }
                    digit_occured = TRUE;
                }
            }
        }
    }
    
    subgridArray[grid_index] = TRUE;
    return NULL;
}
