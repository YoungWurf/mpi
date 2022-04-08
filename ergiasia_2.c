#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <mpi.h>
#include <math.h>

int *createMatrix (int nrows,int ncols,int m[]) { //αλλάζει το μονοδιάστατο πινάκα σε δυο διαστάσεις και επιστροφει το νέο πινάκα 
    int *matrix[nrows];
    int h, i, j;
    for (int i = 0; i < nrows; i++)
    {
        matrix[i] = (int*)malloc(ncols * sizeof(int));
    }
    
    //matrix = (int *)malloc(nrows*ncols*sizeof(int));//δεσμεύει χώρο για το νέο πινάκα
    for (i = 0; i < nrows; i++)
      for (j = 0; j < ncols; j++)
         *(*matrix + i*ncols + j) =m[(i*ncols)+j];//μεταφορά των στοιχειών στο νέο πινάκα στο σωστό κελί 
    return *matrix;//επιστροφή του νέου πινάκα με τις δυο διαστάσεις 
}

void printArray (int *row, int nElements) //εκτύπωση του πινάκα στην οθόνη 
{
    int i;
    for (i=0; i<nElements; i++) {
        printf("%d ", row[i]);
    }
    printf("\n");
}

int abs(int value)//μετατροπή ενός αριθμού στην  απολυτή τιμή του 
{
    if (value < 0)
        return -value;
    return value;
}

int *abs_array(int array[],int size)
{
    for(int i=0;i<size;i++)
    {
        array[i]=abs(array[i]);
    }
    return array;
}

int isDDM(int n, int l, int **m,int rank ) //έλεγχος εάν ο πινάκας είναι αυστηρά διαγώνια δεσπόζων
{
    //για κάθε γραμμή 
    for (int i = 0; i < n; i++)
    {
        // για κάθε στήλη
        int sum = 0;
        for (int j = 0; j < l; j++)
            { sum += abs(m[i][j]);}
            // αθροίζουμε όλα τα στοιχειά και αφαιρούμε τη διαγώνιο   
        sum -= abs(m[i][i+rank]); //η διαγώνιος του πινάκα για κάθε thread είναι ίσο με πινάκα [στήλη] [στήλη + το ρανκ του thread ]
        if (abs(m[i][i+rank]) < sum)//ελέγχουμε εάν ο πινάκας είναι αυστηρά διαγώνια δεσπόζων και επιστρέφουμε την σωστή τιμή για το flag 
        {
            //printf("The rank %d : isDDM: false\n",rank);
            return 1;
        }
    }
    //printf("isDDM: true\n");
    return 0;
}

int print_DDm(bool array[],int size) //ελέγχει εάν όλα τα threads έχουν αυστηρά διαγώνια δεσπόζων πινάκα
{
    int counter = 0;
            for (int i = 0; i <size; i++) //εάν ο πινάκας είναι  όλος true τότε το counter θα είναι ίσο με τo world size 
            {
             if (array[i] == 0)
                {
                    counter++;
                }
            }
            sleep(2);
            if ((counter) ==size)
            {
                printf("YES :The Array is strictly diagonally dominant \n");
                return 0;// και επιστρέφει ανάλογο flag 
            }
            else
            {
                printf("NO:The Array is not strictly diagonally dominant\n");
                return 1;// και επιστρέφει ανάλογο flag 
            }
}

int print_max(int array[],int size) // εμφανίζει μήνυμα με το μεγαλύτερο στοιχειό του πινάκα 
{
    int max_from_all=abs(array[0]);
        for(int i=1;i<size;i++)
        {   
            if(max_from_all<abs(array[i]))
                max_from_all=abs(array[i]);
        }
        printf("The Bigest Absolute Number is %d \n",max_from_all);
        return max_from_all;
}

int main (int argc, char **argv)
{   
    //έναρξη του mpi 
    MPI_Status status;
    MPI_Init(&argc, &argv);
    int world_rank;//id των threads
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;//πλήθος threads
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);


    int m[9] = {10,-2,1,1,-40,2,-1,2,30};//πινάκας με τα στοιχειά που θεωρούμε ως δοσμένα 3x3 για 3 threads
    //int m[36]={1,0,0,0,0,0, 0,2,0,0,0,0, 0,0,3,0,0,0, 0,0,0,4,0,0, 0,0,0,0,5,0, 0,0,0,0,0,6 };
    //int m[16] ={1,2,3,4,5,6,7,8,9,-10,-11,-12,-13,-14,-15,-16}; //για 4χ4 4 threads
    //int m[25]={1,0,0,0,0,0,2,0,0,0,0,0,3,0,0,0,0,0,4,0,0,0,0,0,5}; //για 5x5 5 threads
    int num_of_items_per_proc=(sizeof(m)/sizeof(int))/world_size; //περιέχει ποσά αντικείμενα θα έχει κάθε thread ανάλογα με το μέγεθος του πινάκα και το world_size
    int size=sqrt(sizeof(m)/sizeof(int));//περιέχει το μέγεθος κάθε σειράς και γραμμής το Ν (για πινακα ΝxN)
    int *Array[size]; //2d πινάκας με όλα τα στοιχειά
    if (world_rank == 0) {//ο βασικός επεξεργαστής δημιουργεί το δισδιάστατο πινάκα  
    for (int i = 0; i < size; i++)
        {
            Array[i] = (int*)malloc(size * sizeof(int));
        }
       *Array = createMatrix(size,size,m); 
       printf("Initial matrix:\n");
       printArray(*Array,size*size); //και τον τυπώνει στην οθόνη
    }
    //πρώτο ερώτημα 
    int *procRow = malloc(num_of_items_per_proc); // δημιουργία πινάκα για τα threads με μέγεθος ίσο με τα αντικείμενα που θα επεξεργαστεί κάθε thread (sizeof(int) *(sizeof(m)/world_size))
    int *sendcount, *displs;
    sendcount = (int *)malloc(world_size*sizeof(int));//πινάκας με περιεχόμενο το μέγεθος των στοιχειών που θα στείλουμε ανά thread
    displs = (int *)malloc(world_size*sizeof(int));//πινάκας που θα αποθηκεύει την απόκληση
    for (int i=0; i<size; i++) {
    sendcount[i] = num_of_items_per_proc;
    displs[i] = i*num_of_items_per_proc;//Υπολογισμός απόκλησης
    }
    MPI_Scatterv(&**Array,sendcount, displs, MPI_INT,&*procRow,size, MPI_INT,0, MPI_COMM_WORLD);//scatterv και αποθήκευση του αποτελέσματος στο procRow για κάθε thread
    sleep(world_rank*1);//για να εμφανίζονται τα στοιχειά σε σωστή σειρά στην οθόνη 
    printf("Process %d received elements: ", world_rank); //εκτύπωση στην οθόνη το πινάκα που έχει κάθε thread με ανάλογο μήνυμα 
    printArray(procRow,num_of_items_per_proc);
    bool flag;//flag για κάθε thread για να ξέρουμε εάν ο πινάκας procRow είναι αυστηρά διαγώνια δεσπόζων
    flag=isDDM((size/world_size),size,&procRow,world_rank); //καλούμε την συνάρτηση isDDm για να γίνει ο έλεγχος και αποθηκεύουμε το αποτέλεσμα στο flag
    bool flags[world_size];//πινάκας οπού ο βασικός επεξεργαστής θα αποθηκεύσει τα flag από κάθε thread 
    MPI_Gather(&flag,1, MPI_C_BOOL, flags,1, MPI_C_BOOL, 0, MPI_COMM_WORLD); //μονό ο βασικός επεξεργαστής με την βοήθεια της gather αποθηκεύει τα flag στο πινάκα flags
    int a_flag;//flag ώστε εάν όλος ο πινάκας με τα flags είναι true να προχωρήσουμε στο δεύτερο ερώτημα 
    if (world_rank == 0)//μονό ο βασικός επεξεργαστής εκτελεί τον παρακάτω κωδικά 
        {   
            a_flag=print_DDm(flags,world_size);//βρίσκουμε εάν ο πινάκας είναι αυστηρά διαγώνια δεσπόζων
            for (int i = 1; i < world_size; i++)//στέλνουμε το flag_a στου υπολοίπους επεξεργαστές
            {
                MPI_Send(&a_flag, 1, MPI_INT, i,0, MPI_COMM_WORLD);
            }
        }
    //δεύτερο ερώτημα 
    if(world_rank!=0)//τα threads λαμβάνουν το a_flag
    {
        MPI_Recv(&a_flag, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
    int max_array[num_of_items_per_proc];//πινάκας για να αποθηκεύσουμε τα μέγιστα από τα threads
    int max;//ολικό μέγιστο
    if(a_flag==0) //εάν ο πινάκας είναι αυστηρά διαγώνια δεσπόζων τότε εκτελούμε το παρακάτω κομμάτι του κώδικα
    {
        int *procRow_abs;
        procRow_abs=abs_array(procRow,num_of_items_per_proc); //βρίσκουμε τις απολυτές τιμές του πινάκα procRow και τις βάζουμε στο νέο πινάκα procRow_abs
        MPI_Reduce(procRow_abs,max_array,num_of_items_per_proc, MPI_INT, MPI_MAX,0, MPI_COMM_WORLD);//κάθε thread βρίσκει το τοπικό μέγιστο και το στελνει στο πινάκα max_array
        if(world_rank==0) //μονό ο βασικός επεξεργαστής εκτελεί τον παρακάτω κωδικά
            {   max=print_max(max_array,world_size); //καλούμε την print_max για να εμφανίσει το ολικό μέγιστο στην οθόνη και να το αποθηκεύσει στο max 
                for (int i = 1; i < world_size; i++)//στέλνουμε το max στου υπολοίπους επεξεργαστές
                {
                    MPI_Send(&max, 1, MPI_INT, i,0, MPI_COMM_WORLD);
                    
                }
            }else{//τα threads λαμβάνουν το max 
                
                MPI_Recv(&max, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            }
        //τρίτο ερώτημα 
            int *new_procRow = malloc(sizeof(int) *(sizeof(m)/world_size)); //δημιουργία νέου πινάκα για κάθε thread
            for(int i=0;i<num_of_items_per_proc;i++)
            {
                new_procRow[i]=max-procRow[i];//αποθηκεύουμε στο νέο πινάκα το αντίστοιχο στοιχειό του 
            }
            sleep(world_rank*1);//για να εμφανίζονται τα στοιχειά σε σωστή σειρά στην οθόνη 
            printArray(new_procRow,num_of_items_per_proc);//εμφανίζομε στην οθόνη το νέο πινάκα 
        //τέταρτο  ερώτημα  
            int local_result[2]; //"τοπική" μεταβλητή που έχει δυο θέσεις με πρώτη θέση την μικρότερη τιμή και δεύτερη τη θέση στο πινάκα 
            int global_result[2]; //"global" μεταβλητή που έχει δυο θέσεις με πρώτη θέση την μικρότερη τιμή και δεύτερη τη θέση στο πινάκα 
            local_result[0] = new_procRow[0]; //βρίσκουμε την μικρότερη τιμή για κάθε thread
            for (int i=1; i<num_of_items_per_proc; i++) //βρίσκουμε την μικρότερη τιμή για κάθε thread
            {
                if (new_procRow[i] < local_result[0]) 
                {
                    local_result[0] = new_procRow[i];
                }
            }
            local_result[1] = world_rank;
            /*βάζουμε στη δεύτερη θέση τις μεταβλητής τη θέση που είναι ίδια με το rank του thread
             ,αυτό γίνετε λόγο ότι το μικρότερο στοιχειό θα είναι πάντα στη διαγώνιο αφού πρέπει να είναι αυστηρά διαγώνια δεσπόζων ο αρχικός πινάκας*/
            MPI_Allreduce(local_result,global_result,1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD); //εύρεση ολικού ελάχιστου 
            if (world_rank == 0) 
            {
                printf("The Lowest Value is %d ,in Position[%d][%d]\n", global_result[0], global_result[1],global_result[1]); //εμφάνιση κατάλληλου μηνύματος στην οθόνη 

            }      
      
    }

    MPI_Finalize();//τερματισμός MPI 
    return 0;
}
