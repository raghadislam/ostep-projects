#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH 500
#define MAX_LINES 500

char* filecmp(char **string)
{
    char delim[20] = "/"; 
    char* o_Ptr;
    char* out_ptr;  
    while ((o_Ptr = strsep(string, delim)) != NULL){ 
        out_ptr = o_Ptr ;
    }
    return out_ptr;
}

int main(int argc, char *argv[]) {
    /* Check for correct usage of the program */
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    FILE *fp_input, *fp_output;

    /* If no input file is provided, use stdin */
    if (argc == 1) {
        fp_input = stdin;
        fp_output = stdout;
    } else {
        /* input file */
        fp_input = fopen(argv[1], "r");
        if (fp_input == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }

        /* If no output file is provided, use stdout */
        if (argc == 2) {
            fp_output = stdout;
        } else {
            /* Check if input and output files are the same */
		char *s1 = strdup(argv[1]);
		char *s2 = strdup(argv[2]);
             if ((strcmp(argv[1],argv[2])) == 0 || strcmp (filecmp(&s1), filecmp(&s2)) == 0) {
                fprintf(stderr, "reverse: input and output file must differ\n");
                fclose(fp_input);
                exit(1);
            }

            /* output file */
            fp_output = fopen(argv[2], "w");
            if (fp_output == NULL) {
                fprintf(stderr, "reverse: cannot open file '%s'\n", argv[2]);
                fclose(fp_input);
                exit(1);
            }
        }
    }

    /* Initialize buffer */
    char *lines[MAX_LINES];
    char buffer[MAX_LENGTH];
    int iterator = 0;

    /* Read input file */
    while (fgets(buffer, MAX_LENGTH, fp_input) != NULL) {
        lines[iterator] = strdup(buffer); // Allocate and copy each line
        iterator++;
    }

    /* Write lines in reverse order to the output */
    while (iterator--) {
        fprintf(fp_output, "%s", lines[iterator]);
        free(lines[iterator]); 
    }

    /* close files */
    fclose(fp_input);
    fclose(fp_output);

    return 0;
}

