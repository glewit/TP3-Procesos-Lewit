#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{	
	int start, status, pid, n, c, s;
	int buffer[1];

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
    /* Parsing of arguments */
  	n = atoi(argv[1]);
    c = atoi(argv[2]);
    s = atoi(argv[3]);
    start = s;
    buffer[0] = c;

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
    int max_pipes = n+1;
    int pipes[max_pipes][2];
    int pids[n];

    for (int i = 0; i < max_pipes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("fallo creando pipes");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0 ; i < n; i++) {
    	pids[i] = fork();
    	if (pids[i] == -1){
            perror("fallo haciendo fork");
            exit(EXIT_FAILURE);
        }

        if(pids[i] == 0) {
            if (i < start) {

                dup2(pipes[i][0], STDIN_FILENO);
                close(pipes[i][0]);

                dup2(pipes[(i+1)%max_pipes][1], STDOUT_FILENO);
                close(pipes[(i+1)%max_pipes][1]);

                for (int j = 0; j < n; j++) {
	                    if (j != i) close(pipes[j][0]);
	                    if (j != (i+1)%max_pipes) close(pipes[j][1]);
	                }

            } else {
                dup2(pipes[(i+1)%(max_pipes)][0], STDIN_FILENO);
                close(pipes[(i+1)%(max_pipes)][0]);

                dup2(pipes[(i+2)%(max_pipes)][1], STDOUT_FILENO);
            	close(pipes[(i+2)%(max_pipes)][1]);

            	for (int j = 0; j < n; j++) {
	                    if (j != (i+1)%max_pipes) close(pipes[j][0]);
	                    if (j != (i+2)%max_pipes) close(pipes[j][1]);
	                }

            }

    	    int msg;
        	scanf("%d", &msg);
        	msg++;
        	printf("%d\n", msg);

        	if (i == start){
        		fprintf(stderr, "Soy el hijo %i, recibi un %i del padre, lo incremento a %i y se lo mando al hijo %i \n", i, msg-1, msg, (i+1)%n );
        	} else if (i == start-1){
        		fprintf(stderr, "Soy el hijo %i, recibi un %i del hijo %i, lo incremento a %i y se lo mando al padre \n", i, msg-1, (i-1+n)%n, msg);
        	} else {
        		fprintf(stderr, "Soy el hijo %i, recibi un %i del hijo %i, lo incremento a %i y se lo mando al hijo %i \n", i, msg-1, (i-1+n)%n, msg, (i+1)%n );
        	}
        	return(0);
        }
    }

    dup2(pipes[start][0], STDIN_FILENO);
	close(pipes[start][0]);

	dup2(pipes[(start+1)%(max_pipes)][1], STDOUT_FILENO);
	close(pipes[(start+1)%(max_pipes)][1]);

    for (int j = 0; j < n; j++) {
        if (j != start) close(pipes[j][0]);
        if (j != (start+1)%max_pipes) close(pipes[j][1]);
    }

    printf("%d\n", buffer[0]);
    int msg;
	scanf("%d", &msg);

	sleep(0.5); // para que el padre siempre printee ultimo

	fprintf(stderr, "Soy el padre y recibi %i\n", msg);

    for (int i = 0; i < n; i++) {
        waitpid(pids[i], &status, 0);
    }

    return(0);

}
