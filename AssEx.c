#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

// global variables
enum {LCS, ED, SW, NONE} alg_type; // which algorithm to run
char *alg_desc; // description of which algorithm to run
char *result_string; // text to print along with result from algorithm
char *x, *y; // the two strings that the algorithm will execute on
char *filename; // file containing the two strings
int xLen, yLen, alphabetSize; // lengths of two strings and size of alphabet
bool iterBool = false, recNoMemoBool = false, recMemoBool = false; // which type of dynamic programming to run
bool printBool = false; // whether to print table
bool readFileBool = false, genStringsBool = false; // whether to read in strings from file or generate strings randomly
int total_table_entries_lcs, tte_ed;
int *e_arr;
//TODO - USE THE GLOBAL F-ARR IN ALL FUNCTIONS.
int *f_arr;
int count_inserted;
double proportion;

typedef struct Pair{
    int v;
    int p;
}pair;

pair *a;

// functions follow

// determine whether a given string consists only of numerical digits
bool isNum(char s[]) {
	int i;
	bool isDigit=true;
	for (i=0; i<strlen(s); i++)
		isDigit &= s[i]>='0' && s[i]<='9';
	return isDigit;
}

// get arguments from command line and check for validity (return true if and only if arguments illegal)
bool getArgs(int argc, char *argv[]) {
	int i;
	alg_type = NONE;
	xLen = 0;
	yLen = 0;
	alphabetSize = 0;
	for (i = 1; i < argc; i++) // iterate over all arguments provided (argument 0 is name of this module)
		if (strcmp(argv[i],"-g")==0) { // generate strings randomly
			if (argc>=i+4 && isNum(argv[i+1]) && isNum(argv[i+2]) && isNum(argv[i+3])) { // must be three numerical arguments after this
				xLen=atoi(argv[i+1]); // get length of x
				yLen=atoi(argv[i+2]); // get length of y
				alphabetSize = atoi(argv[i+3]); // get alphabet size
				genStringsBool = true; // set flag to generate strings randomly
				i+=3; // ready for next argument
			}
			else
				return true; // must have been an error with -g arguments
		}
		else if (strcmp(argv[i],"-f")==0) { // read in strings from file
			if (argc>=i+2) { // must be one more argument (filename) after this)
				i++;
				filename = argv[i]; // get filename
				readFileBool = true; // set flag to read in strings from file 
			}
			else
				return true; // must have been an error with -f argument 
		}
		else if (strcmp(argv[i],"-i")==0) // iterative dynamic programming
			iterBool = true;
		else if (strcmp(argv[i],"-r")==0) // recursive dynamic programming without memoisation
			recNoMemoBool = true;
		else if (strcmp(argv[i],"-m")==0) // recursive dynamic programming with memoisation
			recMemoBool = true;
		else if (strcmp(argv[i],"-p")==0) // print dynamic programming table
			printBool = true;
		else if (strcmp(argv[i],"-t")==0) // which algorithm to run
			if (argc>=i+2) { // must be one more argument ("LCS" or "ED" or "SW")
				i++;
				if (strcmp(argv[i],"LCS")==0) { // Longest Common Subsequence
					alg_type = LCS;
					alg_desc = "Longest Common Subsequence";
					result_string = "";
				}
				else if (strcmp(argv[i],"ED")==0) { // Edit Distance
					alg_type = ED;
					alg_desc = "Edit Distance";
					result_string = "";
				}
				else if (strcmp(argv[i],"SW")==0) { // Smith-Waterman Algorithm
					alg_type = SW;
					alg_desc = "Smith-Waterman algorithm";
					result_string = "";
				}
				else
					return true; // none of these; illegal choice
			}
			else
				return true; // algorithm type not given
		else
			return true; // argument not recognised
		// check for legal combination of choices; return true (illegal) if user chooses:
		// - neither or both of generate strings and read strings from file
		// - generate strings with length 0 or alphabet size 0
		// - no algorithm to run
		// - no type of dynamic programming
		return !(readFileBool ^ genStringsBool) || (genStringsBool && (xLen <=0 || yLen <= 0 || alphabetSize <=0)) || alg_type==NONE || (!iterBool && !recMemoBool && !recNoMemoBool);
}

// read strings from file; return true if and only if file read successfully
bool readStrings() {
	// open file for read given by filename
	FILE * file;
	file = fopen(filename, "r");
	// firstly we will measure the lengths of x and y before we read them in to memory
	if (file) { // file opened successfully
		// first measure length of x
		bool done = false;
		int i;
		do { // read from file until newline encountered
			i = fgetc(file); // get next character
			if (i==EOF) { // EOF encountered too early (this is first string) 
				// print error message, close file and return false
				printf("Incorrect file syntax\n");
				fclose(file);
				return false; 
			}
			if ((char) i=='\n' || (char) i=='\r') // newline encountered
				done = true; // terminate loop
			else // one more character
				xLen++; // increment length of x
		} while (!done);
		// next measure length of y
		if ((char) i=='\r')
			fgetc(file); // get rid of newline character
		done = false;
		do { // read from file until newline or EOF encountered
			int i = fgetc(file); // get next character
			if (i==EOF || (char) i=='\n' || (char) i=='\r') // EOF or newline encountered
				done = true; // terminate loop
			else // one more character
				yLen++; // increment length of y
		} while (!done);
		fclose(file);
		// if either x or y is empty then print error message and return false
		if (xLen==0 || yLen==0) {
			printf("Incorrect file syntax\n");
			return false;
		}
		// now open file again for read
		file = fopen(filename, "r");
		// allocate memory for x and y
		x = malloc(xLen * sizeof(char));
		y = malloc(yLen * sizeof(char));
		// read in x character-by-character
		for (i=0; i<xLen; i++)
			x[i]=fgetc(file);
		i = fgetc(file); // read in newline between strings and discard
		if ((char) i=='\r')
			fgetc(file); // read \n character and discard if previous character was \r
		// read in y character-by-character
		for (i=0; i<yLen; i++)
			y[i]=fgetc(file);
		// close file and return boolean indicating success
		fclose(file);
		return true;
	}
	else { // notify user of I/O error and return false
		printf("Problem opening file %s\n",filename);
		return false;
	}
}

// generate two strings x and y (of lengths xLen and yLen respectively) uniformly at random over an alphabet of size alphabetSize
void generateStrings() {
	// allocate memory for x and y
	x = malloc(xLen * sizeof(char));
	y = malloc(yLen * sizeof(char));
	// instantiate the pseudo-random number generator (seeded based on current time)
	srand(time(NULL));
	int i;
	// generate x, of length xLen
	for (i = 0; i < xLen; i++)
		x[i] = rand()%alphabetSize +'A';
	// generate y, of length yLen
	for (i = 0; i < yLen; i++)
		y[i] = rand()%alphabetSize +'A';
}

// free memory occupied by strings
void freeMemory() {
	free(x);
	free(y);
}

int get_num_digits(int num){
	int count = 1;
	while(num > 9){
		num /= 10;
		count += 1;
	}
	return count;
}

int min3(int a, int b, int c) {
	if (a<b && a<c) return a;
	else if (b<c) return b;
	else return c;
}

int max2(int a, int b){
	if(a>b) return a;
	else return b;
}

void reverse(char *x, int begin, int end)
{
   char c;
   if (begin >= end)
      return;
   c = *(x+begin);
   *(x+begin) = *(x+end);
   *(x+end)   = c;
   reverse(x, ++begin, --end);
}

//3.
//TODO -- PRINT BETTER WITH FINDING THE BIGEST NUMBER IN SIZE & AND KEEPING IT AS OFFSET.
/* Prints a table*/

void table_print(int *s_arr){
  int n = xLen + 1; //rows
  int m = yLen + 1; // columns

  int i, j,k = 0;
  int clone_j;

//Horizontal indexes. removing the two-digit ones.
  printf("       ");
  for(i = 0; i < n; i++){
    for(j = 0; j < m; j++){
      if(i == 0){
        if(j > 9){
          clone_j = j%10;
          printf("  %d    ", clone_j);
        }else{
          printf("  %d    ", j);
        }
      }
    }
  }
  printf("\n");
  printf("       ");
// Horizontal chars 
  for(i = 0; i < m; i++){
    printf("  %c    ", y[i - 1]);
  }
  printf("\n");
  printf("       ");
  for(i = 0; i < m - 1; i++){
    printf("%s", "_______ ");
  }

  printf("\n");

  // The values in the table and the vertical chars in x
  int curr_num;

  for(i = 0; i < n;i++){
  	if(i==0) printf(" %d   |",i%10 );
  	else printf(" %d %c |",i%10,x[i - 1] );
  	for(j = 0; j < m;j++){
  		curr_num = s_arr[i*m + j];
  		for(k=0; k < 7 - get_num_digits(curr_num);k++) printf(" ");
  		printf("%d", curr_num);
  	}
  	printf("\n");
  }
}
void print_struct_table(pair* arr) {
	int SIZE = (xLen+1) * (yLen + 1);
	int int_arr[SIZE];
	int i;
	for (i = 0; i<SIZE; i++ ) {
		int_arr[i] = arr[i].v;
	}
	table_print(int_arr);
}

// 1.
/*Returns the length of the 
Longest Common Subsequesnce between strings x and y.*/
int iterative_lcs(char *x, char *y){
	int n = xLen + 1; //rows
	int m = yLen + 1; // columns

	int i, j;
	int longest_common_subseq_len;
	f_arr = (int*)malloc(sizeof(int)*n*m);

	//f[i][0] == f[0][j] == 0
	for(i = 0; i < n; i++){
		f_arr[i] = 0;
		for(j = 0; j < m; j++){
			f_arr[i*m + j] = 0;
		}
	}

	// Check for match or missmatch n
	for(i = 1; i < n; i++){
		for(j = 1; j < m; j++){
			if(x[i - 1] == y[j - 1]){
				f_arr[i*m + j] = 1 + f_arr[(i - 1)*m + (j - 1)];
			}else{
				f_arr[i*m + j] = max2(f_arr[(i - 1)*m + j], f_arr[i*m + (j - 1)]);
			}
		}
	}
	/* n - 2, because the row starts from index 0 
	and the length of the row is n - 1.*/
	longest_common_subseq_len = f_arr[(n-1)*m + m-1];
	//printf("Dynamic programming table: \n");
	//table_print(f_arr);
	free(f_arr);
	return longest_common_subseq_len;
}

//Recursion for LCS without memoization
int sub_lcs(int i, int j, int *dp_arr){
	if (i == 0 || j == 0){
		return 0;
	}
    else if (x[i - 1] == y[j - 1]){
    	//diagonally
    	total_table_entries_lcs += 1;
    	dp_arr[(i-1)*(yLen + 1) + j-1] += 1;
    	return 1 + sub_lcs(i - 1, j - 1, dp_arr);
    }
    else{
    	//up and left
    	total_table_entries_lcs += 2;
    	dp_arr[(i-1)*(yLen + 1) + j] += 1;
    	dp_arr[i*(yLen + 1) + j - 1] += 1;
    	return max2(sub_lcs(i, j-1, dp_arr), sub_lcs(i-1, j, dp_arr));
    }

}

//4.
int recursive_lcs_no_mem(){
	int n = xLen + 1; //rows
	int m = yLen + 1; // columns
	total_table_entries_lcs = 0;
	int *dp_arr;
	dp_arr = (int*)calloc(m*n, sizeof(int));
	sub_lcs(xLen, yLen, dp_arr);
    //table_print(dp_arr);
  	free(dp_arr);
}


bool isEvaluated(int i, int j, int count_inserted, int m, pair* a, int *b){
	if((a[i*m + j].p) < 1 || (a[i*m + j].p) > count_inserted){
		return false;
	}else{
		return (b[(a[i*m + j].p)] == i*m + j);
	}
}

int sub_lcs_recursive(int i, int j, pair* a, int *b, int* count_inserted){
	int n = xLen + 1;
	int m = yLen + 1;
	int computed = 0;
	if(!isEvaluated(i,j,*count_inserted,yLen+1,a,b)){
		if (i == 0 || j == 0){
			a[i*m + j].v = 0;
			*count_inserted += 1;
			a[i*m + j].p = *count_inserted;
			b[*count_inserted] = i*m + j;
		}
	    else if (x[i - 1] == y[j - 1]){
	    	// diagonally
	    	a[i*m + j].v = 1 + sub_lcs_recursive(i - 1, j - 1,a,b,count_inserted);
			*count_inserted += 1;
			a[i*m + j].p = *count_inserted;
			b[*count_inserted] = i*m + j;
	    }
	    else{
	    	//up and left
	    	a[i*m + j].v = max2(sub_lcs_recursive(i, j-1,a,b,count_inserted), sub_lcs_recursive(i-1, j,a,b,count_inserted));
			*count_inserted += 1;
			a[i*m + j].p = *count_inserted;
			b[*count_inserted] = i*m + j;
	    }
	}
	return a[i*m + j].v;
}
//5.
//result of lcs
//number of table entries computed
// proportion of table computed
int recursive_lcs_with_mem(int i, int j){
	int n = i + 1; //rows
	int m = j + 1; // columns
	int lcs = 0;
	count_inserted = 0;
	
	int k;

	a = malloc(m*n*sizeof(pair));
	
	int *b;
	b = malloc(m*n*sizeof(int));
	
	sub_lcs_recursive(i, j, a, b, &count_inserted);
	proportion = count_inserted/((xLen)*(yLen+1) + yLen + 1.0)*100;
	lcs = a[m*(n - 1) + m-1].v;
    //print_struct_table(a);
  	free(a);
  	free(b);
  	
  	return lcs;
}


//6. Optimal alignment of x and y in LCS
void optimal_alignment(char *x, char *y){
	int n = xLen + 1; //rows
	int m = yLen + 1; // columns
	
	int l = f_arr[(n-1)*m + m - 1];
	char *aligned_x = malloc((xLen + yLen - l + 1)* sizeof(char));
	char *aligned_y = malloc((xLen + yLen - l + 1)* sizeof(char));
	char *aligned_space = malloc((xLen + yLen - l + 1)*sizeof(char));
	
	int i,j;
	i = 1;
	j = 1;
	int index = 0;

	while(i <= n && j <= m)
	{
		if(i==n){
			aligned_x[index] = '-';
			aligned_y[index] = y[m - 1 - j];
			aligned_space[index] = ' ';
			j += 1;
			index += 1;
		}
		else if(j == m){
			aligned_x[index] = x[n - 1 - i];
			aligned_y[index] = '-';
			aligned_space[index] = ' ';
			i += 1;
			index += 1;
		}
		else if(x[n - 1 - i] == y[m - 1 - j]){
			aligned_x[index] = x[n - 1 - i];
			aligned_y[index] = y[m - 1 - j];
			aligned_space[index] = '|';
			index += 1;
			i += 1;
			j += 1;
			
		}else{
			// up > left
			if(f_arr[(n - 1 - i)*m + m - j ] > f_arr[(n - i)*m + m - 1 - j]){
				//up
				aligned_x[index] = x[n - 1 - i];
				aligned_y[index] = '-';
				aligned_space[index] = ' ';
				i += 1;
				index += 1;
			}else{
				//left
				aligned_x[index] = '-';
				aligned_y[index] = y[m - 1 - j];
				aligned_space[index] = ' ';
				j += 1;
				index += 1;
			}
		}
	}
	aligned_x[xLen + yLen - l] = '\0';
	aligned_space[xLen + yLen - l] = '\0';
	aligned_y[xLen + yLen - l] = '\0';
	reverse(aligned_x,0,strlen(aligned_x) - 1);
	reverse(aligned_y,0,strlen(aligned_x) - 1);
	reverse(aligned_space,0,strlen(aligned_x) - 1);
	printf("%s\n", aligned_x);
	printf("%s\n", aligned_space);
	printf("%s\n", aligned_y);
}

// 2.
//Returns the Edit Distance between strings x and y.
int ed(char *x, char *y){
	int n = xLen + 1; //rows
	int m = yLen + 1; // columns

	int i, j;
	int edit_distance;
	int *d_arr;
	d_arr = (int*)malloc(sizeof(int)*n*m);
	for(i = 0; i < n; i++){
		d_arr[i] = j;
		for(j = 0; j < m; j++){
			d_arr[i*m + j] = i;
		}
	}

	// Check for match or missmatch n
	for(i = 1; i < n; i++){
		for(j = 1; j < m; j++){
			if(x[i - 1] == y[j - 1]){
				d_arr[i*m + j] = d_arr[(i - 1)*m + (j - 1)];
			}else{
				d_arr[i*m + j] = min3(d_arr[(i - 1)*m + j] + 1,
					d_arr[i*m + (j - 1)] + 1,
					d_arr[(i - 1)*m + (j - 1)] + 1);
			}
		}
	}
	edit_distance = d_arr[(m - 1)*(n - 1) + n - 2];
	table_print(d_arr);
	free(d_arr);
	return edit_distance;
}

int sub_ed(int i, int j){
	if( i == xLen && j == yLen){
		tte_ed += 1;
		e_arr[i*(yLen + 1) + j] += 1;
	}

	if (i == 0){
		return j;
	}
	else if(j == 0){
		return i;
	}
    else if (x[i - 1] == y[j - 1]){
    	//diagonally
    	tte_ed += 1;
    	e_arr[(i-1)*(yLen + 1) + j-1] += 1;
    	return sub_ed(i - 1, j - 1);
    }
    else{
    	//up and left
    	tte_ed += 3;
    	e_arr[(i-1)*(yLen + 1) + j] += 1;
    	e_arr[i*(yLen + 1) + j - 1] += 1;
    	e_arr[(i-1)*(yLen + 1) + j-1] += 1;
    	return 1 + min3(sub_ed(i, j-1), sub_ed(i-1, j),sub_ed(i-1,j-1));
    }

}

//4.
int recursive_ed_no_mem(){
	int n = xLen + 1; //rows
	int m = yLen + 1; // columns
	tte_ed = 0;
	e_arr = (int*)calloc(n*m, sizeof(int));
	sub_ed(xLen, yLen);
    table_print(e_arr);
  	free(e_arr);
}

int recursive_ed_with_mem(){
	
}
// 2.
/*Returns the length of the
Highest Scoring Local Similarity between
strings x and y using the Smith-Waterman alg.*/
int hsls(char *x, char* y){
  int n = xLen + 1; //rows
  int m = yLen + 1; // columns

  //Testing
  //int n = 8;
  //int m = 9;

  int i, j = 0;
  int best_score = 0;
  int *s_arr;
  s_arr = (int*)malloc(sizeof(int)*n*m);

  for(i = 0; i < n; i++){
  	s_arr[i] = 0;
    for(j = 0; j < m; j++){
      s_arr[i*m + j] = 0;
    }
  }

  for(i = 1; i < n; i++){
    for(j = 1; j < m; j++){
      if(x[i - 1] == y[j - 1]){
        s_arr[i*m + j] = s_arr[(i - 1)*m + (j - 1)] + 1;
      }else{
        s_arr[i*m + j] = max2(s_arr[(i - 1)*m + (j - 1)] - 1,
          max2(s_arr[i*m + (j - 1)] - 1,
          max2(s_arr[(i - 1)*m + j] - 1,
          0)));
      }
      if(s_arr[i*m + j] > best_score){
        best_score = s_arr[i*m + j];
      }
    }
  }
  table_print(s_arr);
  free(s_arr);
  return best_score;
}



// main method, entry point
int main(int argc, char *argv[]) {
	clock_t start, end;
	bool isIllegal = getArgs(argc, argv); // parse arguments from command line
	if (isIllegal) {// print error and quit if illegal arguments
		printf("Illegal arguments\n");
	}
	else {
		printf("%s\n%s\n", alg_desc, result_string); // confirm algorithm to be executed
		bool success = true;
		if (genStringsBool)
			generateStrings(); // generate two random strings
		else
			success = readStrings(); // else read strings from file
		if (success) { // do not proceed if file input was problematic
			// confirm dynamic programming type
			// these print commamds are just placeholders for now
			if (iterBool){
				printf("Iterative version\n");
				start = clock();
				if(alg_type == LCS){
					printf("Length of longest common subsequence is: %d\n", iterative_lcs(x,y));
					//printf("Optimal alignment: \n");
				//	optimal_alignment(x,y);
				}else if(alg_type == ED){
					printf("Edit distance is: %d\n", ed(x,y));
				}else if(alg_type == SW){
					printf("Length of a highes scoring local similarity is: %d\n", hsls(x,y));
				}
				end = clock();
				printf("Time taken: %.2fseconds\n", (double)(end - start)/CLOCKS_PER_SEC);
			}if (recMemoBool){
				if(alg_type==LCS){
					start = clock();
					int lcs = recursive_lcs_with_mem(xLen, yLen);
					end = clock();
					printf("Time taken: %.2fseconds\n", (double)(end - start)/CLOCKS_PER_SEC);
					printf("Recursive version with memoisation\n");
					printf("Length of longest common subsequence is: %d\n", lcs);
					printf("Number of table entries computed: %d\n", count_inserted);
  					printf("Proportion of table entries computed: %.1f\n", proportion);
				}else if(alg_type==ED){

				}
			}
				
			if (recNoMemoBool){
				if(alg_type==LCS ){
					printf("Recursive version without memoisation\n");
					start = clock();
					recursive_lcs_no_mem();
					end = clock();
					printf("Time taken: %.2fseconds\n", (double)(end - start)/CLOCKS_PER_SEC);
					printf("Total number of times a table entry computed: %d\n", total_table_entries_lcs);
				}else if(alg_type==ED){
					printf("Recursive version without memoisation\n");
					recursive_ed_no_mem();
					printf("Total number of times a table entry computed: %d\n", tte_ed);
				}	
			}
			freeMemory(); // free memory occupied by strings
		}
	}
	return 0;
}

