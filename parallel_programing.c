#include <stdio.h>
#include "mpi.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv)
{
    MPI_Status status;
    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // tags για να στείλουμε και να λάβουμε δεδομένα μεταξύ του και των επεξεργαστών
    int tag_flag=100;
    int tag_a=10;
    int tab_b=20;
    int tag_d=40;
    int tag_e=50;
    int tag_f=60;
    int tag_z=70;

    if (world_rank == 0) //κώδικάς του communicator 
    {
        int i, index, size, Elements[100], flag = 1; 
        //δημιουργία μενού επιλογών και διάβασμά επιλογής χρήστη
        char choice[20];
        printf("*If you want to calculate AVERAGE,VARIANCE and δ(i) of an array,press '1'\n");
        printf("*If you want to exit type 'exit'\n=>");
        scanf("%s", choice);

        while (strcmp(choice, "exit"))//όσο ο χρήστης δεν έχει διαλέξει το εχιτ
        {
            for (i = 1; i < world_size; i++)//στέλνουμε το flag στου υπολοίπους επεξεργαστές
            {
                MPI_Send(&flag, 1, MPI_INT, i, tag_flag, MPI_COMM_WORLD);
            }

            //εάν ο χρήστης επιλέξει το 1 τότε ζητάμε το μέγεθος του πίνακά και τα στοιχεία του
            //και τα διαβάζουμε από το πληκτρολόγιο 
            if (!strcmp(choice, "1"))
            {
                printf("Give me the size of the array:\n");
                scanf("%d", &size);
                int elements_per_process = size / world_size;

                int local_min, local_max, tmp_max, tmp_min, D_elements[100];

                printf("Type the elements of the array:\n");
                for (i = 0; i < size; i++)
                {
                    scanf("%d", &Elements[i]);
                }

                //χωρίζουμε το πινάκα σε τμήματα και τα στέλνουμε σε κάθε επεξεργαστή 
                if (world_size > 1)
                {
                    for (i = 1; i < world_size-1; i++)
                    {
                        index = i * elements_per_process;
                        MPI_Send(&elements_per_process, 1, MPI_INT, i, tag_a, MPI_COMM_WORLD);
                        MPI_Send(&Elements[index], elements_per_process, MPI_INT, i, tag_a, MPI_COMM_WORLD);
                    }
                //στέλνουμε ξεχωριστά τα τελευταία στοιχειά από τα υπόλοιπα ώστε να κατανεμηθεί όσο καλύτερα γίνετε το βάρος της εκτέλεσης του προγράμματος 
                    index = i * elements_per_process;
                    int elements_left = size - index;
                    MPI_Send(&elements_left, 1, MPI_INT, i, tag_a, MPI_COMM_WORLD);
                    MPI_Send(&Elements[index], elements_left, MPI_INT, i, tag_a, MPI_COMM_WORLD);
                }

                //αρχή α) αρώτηματος 
                //υπολογίζουμε το sum για τον communicator και περιμένουμε να λάβουμε τα υπόλοιπα αθροίσματα από τους επεξεργαστές 
                int sum = 0, tmp = 0;
                float aver = 0, var = 0;
                for (i = 0; i < elements_per_process; i++)
                {
                    sum += Elements[i];
                }
                //αφού λάβουμε τα υπόλοιπα αθροίσματα τα προσθέτουμε και υπολογίζουμε το μέσο ορό και εμφανίζουμε σχετικό μήνυμα 
                for (i = 1; i < world_size; i++)
                {
                    MPI_Recv(&tmp, 1, MPI_INT, i, tab_b, MPI_COMM_WORLD, &status);
                    sum += tmp;
                }
                aver = sum / size;
                sum = 0;
                printf("The Average of array is : %f\n", aver);
                //αρχή β) αρώτηματος
                //στέλνουμε στους υπόλοιπους επεξεργαστές τον μέσο ορό μέσω το aver 
                if (world_size > 1)
                {
                    for (i = 1; i < world_size; i++)
                    {
                        MPI_Send(&aver, 1, MPI_INT, i, tag_d, MPI_COMM_WORLD); 
                    }
                } 
                tmp = 0;
                //υπολογίζουμε τη διασπορά για τα στοιχειά που έχει ο communicator 
                for (i = 0; i < elements_per_process; i++)
                {
                    var += pow((Elements[i] - aver), 2);
                }
                //περιμένουμε να λάβουμε την διασπορά για τα υπόλοιπα στοιχειά που έχει κάθε επεξεργαστής 
                //και προσθέτουμε τα αποτελέσματα ,τελος εμφανίζομε σχετικό μήνυμα στην οθόνη με την απάντηση 
                for (i = 1; i < world_size; i++)
                {
                    MPI_Recv(&tmp, 1, MPI_INT, i, tag_e, MPI_COMM_WORLD, &status);
                    var += tmp;
                }
                var = var / size;
                printf("The Variance of array is : %f\n", var);
                //αρχή γ) αρώτηματος
                //υπολογίζουμε το max και το min για τα στοιχειά που έχει ο communicator 
                var = 0;
                tmp = 0;
                local_max = local_min = Elements[0];
                for (i = 0; i < elements_per_process; i++)
                {
                    if (Elements[i] > local_max)
                        local_max = Elements[i];
                }
                for (i = 0; i < elements_per_process; i++)
                {
                    if (Elements[i] < local_min)
                        local_min = Elements[i];
                }
                //περνούμε  τα min και max για τους υπόλοιπους  επεξεργαστές 
                //και βρίσκουμε το ολικό min και max 
                for (i = 1; i < world_size; i++)
                {
                    MPI_Recv(&tmp_max, 1, MPI_INT, i, tag_f, MPI_COMM_WORLD, &status);
                    if (tmp_max > local_max)
                        local_max = tmp_max;
                    MPI_Recv(&tmp_min, 1, MPI_INT, i, tag_z, MPI_COMM_WORLD, &status);
                    if (tmp_min < local_min)
                        local_min = tmp_min;
                }
                //υπολογίζουμε το νέο διάνυσμα και εμφανίζουμε σχετικό μήνυμα στην οθόνη 
                for (i = 0; i < size; i++)
                    D_elements[i] = ((Elements[i] - local_min) / (local_max - local_min)) * 100;
                printf("\t The D_Array is :\n");
                for (i = 0; i < size; i++)
                    printf("\t\t\t\tD_Array[%d]=%d\n", i, D_elements[i]);
            }
            //ζητάμε από το χρήστη να διαλέξει μια νέα επιλογή από το μενού 
            printf("Give new choice between 1 and exit\n=>");
            scanf("%s", choice);
        }
        //εάν ο χρήστης πληκτρολογήσει exit το flag γίνετε 0 και ενημερώνουμε τους υπόλοιπους επεξεργαστές για να σταματήσει η λειτουργιά του προγράμματος 
        flag = 0;
        for (i = 1; i < world_size; i++)
        {
            MPI_Send(&flag, 1, MPI_INT, i, 10, MPI_COMM_WORLD);
        }
        printf("You exited the program\n");
        MPI_Finalize();
        return 0;
    }
    else //κώδικας για τους υπόλοιπους επεξεργαστές 
    {
        int tempElements[100], n_elements_received;
        float aver;
        int tmp_flag, local_min, local_max;
        //λαμβάνουμε το flag από τον communicator για να συνεχίσει η λειτουργιά του προγράμματος 
        MPI_Recv(&tmp_flag, 1, MPI_INT, 0, tag_flag, MPI_COMM_WORLD, &status);
        while (tmp_flag == 1)
        {   //κάθε επεξεργαστής λαμβάνει το τμήμα του πινάκα του 
            MPI_Recv(&n_elements_received, 1, MPI_INT, 0, tag_a, MPI_COMM_WORLD, &status);
            MPI_Recv(&tempElements, n_elements_received, MPI_INT, 0, tag_a, MPI_COMM_WORLD, &status);
            int partial_sum = 0;
            float partial_var = 0;
             //α) αρώτημα
            //υπολογισμός του αθροίσματος του πινάκα και αποστολή του αποτελέσματος στον communicator 

            for (int i = 0; i < n_elements_received; i++)
            {
                partial_sum += tempElements[i];
            }
            MPI_Send(&partial_sum, 1, MPI_INT, 0, tab_b, MPI_COMM_WORLD);
            //β) αρώτημα
            // λαμβάνει ο επεξεργαστής τον μέσο ορό και υπολογίζει την διασπορά 
            MPI_Recv(&aver, 1, MPI_INT, 0, tag_d, MPI_COMM_WORLD, &status);

            
            for (int i = 0; i < n_elements_received; i++)
            {
                partial_var += pow((tempElements[i] - aver), 2);
            }
            //ο επεξεργαστής στέλνει το αποτέλεσα πίσω στον communicator 
             MPI_Send(&partial_var, 1, MPI_INT, 0, tag_e, MPI_COMM_WORLD);
            //γ) αρώτημα
            //// βρίσκουμε τα τοπικά min και max 
            for (int i = 0; i < n_elements_received; i++)
            {
                if (tempElements[i] > local_max)
                    local_max = tempElements[i];
            }
            for (int i = 0; i < n_elements_received; i++)
            {
                if (tempElements[i] < local_min)
                    local_min = tempElements[i];
            }
            // στέλνουμε τα min και max στον communicator 
            MPI_Send(&local_max, 1, MPI_INT, 0, tag_f, MPI_COMM_WORLD);
            MPI_Send(&local_min, 1, MPI_INT, 0, tag_z, MPI_COMM_WORLD);
            // ζητάμε ξανά το flag για να συνεχίσει να τρέχει το πρόγραμμα 
            MPI_Recv(&tmp_flag, 1, MPI_INT, 0, tag_flag, MPI_COMM_WORLD, &status);
        }
        MPI_Finalize();
        return 0;
    }
    MPI_Finalize();
    return 0;
}

//παρακάτω υπάρχει ξεχωριστά κάθε ερώτημα , χωρίς μενού επιλογών χωρίς tags για την επικοινωνία με τον communicator .
/*
#include <stdio.h>
#include "mpi.h"
#include <math.h>
#include <stdlib.h>
int main(int argc, char** argv)  
{   
    MPI_Status status;
    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int size,n_elements_received,Elements[100],tempElements[100];
    float aver;
   //a)
    if (world_rank == 0) {
        int i,index;
        printf("Give me the size of the array:\n");
			scanf("%d", &size);
        int elements_per_process = size /world_size;
		printf("Type the elements of the array:\n"); 
		    for (i=0; i<size; i++)
            {
			    scanf("%d", &Elements[i]);
            }
        if(world_size>1){
            for (i = 1; i < world_size - 1; i++){
                index = i * elements_per_process;
                MPI_Send(&elements_per_process,1, MPI_INT, i, 0,MPI_COMM_WORLD);
                MPI_Send(&Elements[index],elements_per_process,MPI_INT, i, 0,MPI_COMM_WORLD);
            }
            index = i * elements_per_process;
            int elements_left = size - index;
            MPI_Send(&elements_left,1, MPI_INT,i, 0,MPI_COMM_WORLD);
            MPI_Send(&Elements[index],elements_left,MPI_INT, i, 0,MPI_COMM_WORLD);
        }
        int sum = 0;
        for (i = 0; i < elements_per_process; i++)
            sum += Elements[i];

        int tmp;
        for (i = 1; i < world_size; i++) {
            MPI_Recv(&tmp, 1, MPI_INT,i, 0,MPI_COMM_WORLD,&status);
            sum += tmp;
        }
        aver=(float)sum/size;
        printf("The Average of array is : %f\n",aver);
    }
    else{
        MPI_Recv(&n_elements_received,1, MPI_INT, 0, 0,MPI_COMM_WORLD,&status);
        MPI_Recv(&tempElements, n_elements_received,MPI_INT, 0, 0,MPI_COMM_WORLD,&status);
        int partial_sum = 0;
        for (int i = 0; i < n_elements_received; i++)
            partial_sum += tempElements[i];
        MPI_Send(&partial_sum, 1, MPI_INT,0, 0, MPI_COMM_WORLD);
    }

// b)
    if(world_rank==0)
    {   
        float var=0;
        int i,index,elements_per_process = size /world_size;
        if(world_size>1){
            for (i = 1; i < world_size - 1; i++){
                index = i * elements_per_process;
                MPI_Send(&elements_per_process,1, MPI_INT, i, 0,MPI_COMM_WORLD);
                MPI_Send(&aver,1, MPI_INT, i, 0,MPI_COMM_WORLD);
                MPI_Send(&Elements[index],elements_per_process,MPI_INT, i, 0,MPI_COMM_WORLD);
            }
            index = i * elements_per_process;
            int elements_left = size - index;
            MPI_Send(&elements_left,1, MPI_INT,i, 0,MPI_COMM_WORLD);
            MPI_Send(&aver,1, MPI_INT, i, 0,MPI_COMM_WORLD);
            MPI_Send(&Elements[index],elements_left,MPI_INT, i, 0,MPI_COMM_WORLD);
        }
        for (i = 0; i < elements_per_process; i++)
        {
            var+=pow((Elements[i]-aver),2);
        }
        float tmp=0;
        for (i = 1; i < world_size; i++) 
        {
            MPI_Recv(&tmp, 1, MPI_INT,i, 0,MPI_COMM_WORLD,&status);
            var+=tmp;
        }
        var=var/size;
        printf("The Variance of array is : %f\n",var);
    }
    else{
        MPI_Recv(&n_elements_received,1, MPI_INT, 0, 0,MPI_COMM_WORLD,&status);
        MPI_Recv(&aver,1, MPI_INT, 0, 0,MPI_COMM_WORLD,&status);
        MPI_Recv(&tempElements, n_elements_received,MPI_INT, 0, 0,MPI_COMM_WORLD,&status);
       float partial_var=0;
       for (int i = 0; i < n_elements_received; i++)
         {   partial_var+=pow((tempElements[i]-aver),2);}
        MPI_Send(&partial_var, 1, MPI_INT,0, 0, MPI_COMM_WORLD);
    }

    //c)
    if(world_rank==0){
        int i,index,elements_per_process = size /world_size,local_min,local_max,tmp_max,tmp_min,D_elements[100];
        if(world_size>1){
            for (i = 1; i < world_size - 1; i++){
                index = i * elements_per_process;
                MPI_Send(&elements_per_process,1, MPI_INT, i, 0,MPI_COMM_WORLD);
                MPI_Send(&Elements[index],elements_per_process,MPI_INT, i, 0,MPI_COMM_WORLD);
            }
            index = i * elements_per_process;
            int elements_left = size - index;
            MPI_Send(&elements_left,1, MPI_INT,i, 0,MPI_COMM_WORLD);
            MPI_Send(&Elements[index],elements_left,MPI_INT, i, 0,MPI_COMM_WORLD);
        }
        local_max=local_min=Elements[0];
        for (i = 0; i < elements_per_process; i++)
        {
                if(Elements[i]>local_max)
				local_max=Elements[i];
        }
         for (i = 0; i < elements_per_process; i++)
        {
                if(Elements[i]<local_min)
				local_min=Elements[i];
        }
        for (i = 1; i < world_size; i++) 
        {
            MPI_Recv(&tmp_max, 1, MPI_INT,i, 0,MPI_COMM_WORLD,&status);
            if(tmp_max>local_max)
                local_max=tmp_max;  
            MPI_Recv(&tmp_min, 1, MPI_INT,i, 0,MPI_COMM_WORLD,&status);
                if(tmp_min<local_min)
                local_min=tmp_min; 
        }
        for(i=0; i<size; i++)
				D_elements[i]=((Elements[i]-local_min)/(local_max-local_min))*100;
        printf("\t The D_Array is :\n");
        for(i=0; i<size; i++)
				printf("\t\t\t\tD_Array[%d]=%d\n",i, D_elements[i]);
    }
    else{
        int i,local_max,local_min;
        MPI_Recv(&n_elements_received,1, MPI_INT, 0, 0,MPI_COMM_WORLD,&status);
        MPI_Recv(&tempElements, n_elements_received,MPI_INT, 0, 0,MPI_COMM_WORLD,&status);
        for (i = 0; i < n_elements_received; i++)
        {
                if(tempElements[i]>local_max)
				local_max=tempElements[i];
        }
         for (i = 0; i < n_elements_received; i++)
        {
                if(tempElements[i]<local_min)
				local_min=tempElements[i];
        }
        MPI_Send(&local_max, 1, MPI_INT,0, 0, MPI_COMM_WORLD);
        MPI_Send(&local_min, 1, MPI_INT,0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}*/