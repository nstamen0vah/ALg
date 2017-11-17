#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

int *it;

int max2(int a, int b){
	if(a>b) return a;
	else return b;
}

int get_num_digits(int num){
	int count = 1;
	while(num > 9){
		num /= 10;
		count += 1;
	}
	return count;
}

//3.
/* Prints a table*/
void table_print(int *s_arr, char *x, char* y){
  int n = strlen("CBBCBC") + 1; //rows
  int m = strlen("BBCCAB") + 1; // columns

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

// 1.
/*Returns the length of the 
Longest Common Subsequesnce between strings x and y.*/
int iterative_lcs(char *x, char *y){
	int n = strlen("CBBCBC") + 1; //rows
	int m = strlen("BBCCAB") + 1; // columns

	int i, j;
	int longest_common_subseq_len;
	it = (int*)malloc(sizeof(int)*n*m);

	//f[i][0] == f[0][j] == 0
	for(i = 0; i < n; i++){
		it[i] = 0;
		for(j = 0; j < m; j++){
			it[i*m + j] = 0;
		}
	}

	// Check for match or missmatch n
	for(i = 1; i < n; i++){
		for(j = 1; j < m; j++){
			if(x[i - 1] == y[j - 1]){
				it[i*m + j] = 1 + it[(i - 1)*m + (j - 1)];
			}else{
				it[i*m + j] = max2(it[(i - 1)*m + j], it[i*m + (j - 1)]);
			}
		}
	}
	/* n - 2, because the row starts from index 0 
	and the length of the row is n - 1.*/
	//longest_common_subseq_len = it[(m - 1)*(n - 1) + n + m - 2];
	longest_common_subseq_len = it[(n-1)*m + m-1];
	table_print(it, x, y);
	//free(it);
	return longest_common_subseq_len;
}

//6. Optimal alignment of x and y in LCS
void optimal_alignment(char *x, char *y){
	printf("this");
	int n = strlen(x) + 1; //rows
	int m = strlen(y) + 1; // columns
	int l = it[(n-1)*m + m - 1];
	char *aligned_x = malloc(m + n - l* sizeof(char));
	char *aligned_y = malloc(m + n - l* sizeof(char));
	char *aligned_space = malloc(m + n - l*sizeof(char));
	size_t length = (max2(strlen(x),strlen(y)));
	int position = (n-1)*m + m-1;
	int i;
	int j_x,k_y;
	j_x = 1;
	k_y = 1;
	int index = 0;

	while(((((n - 1 - j_x)*m + m - k_y) != 0) || (((n - j_x)*m + m - 1 - k_y) != 0)))
	{

		if(x[n - 1 - j_x] == y[m - 1 - k_y]){

			aligned_x[index] = x[n - 1 - j_x];
			aligned_y[index] = y[m - 1 - k_y];
			aligned_space[index] = '|';
			index += 1;
			j_x += 1;
			k_y += 1;
		}else{
			// up > left
			if(it[(n - 1 - j_x)*m + m - k_y ] > it[(n - j_x)*m + m - 1 - k_y]){
				aligned_x[index] = x[n - 1 - j_x];
				aligned_x[index] = '-';
				aligned_space[index] = ' ';
				j_x += 1;
				index += 1;
			}else{
				//left
				aligned_x[index] = '-';
				aligned_y[index] = y[m - 1 - k_y];
				aligned_space[index] = ' ';
				k_y += 1;
				index += 1;
			}
		}
	}
	printf("%s\n", aligned_x);
	printf("%s\n", aligned_space);
	printf("%s\n", aligned_y);
}

int main(void){
	int lcs_len = iterative_lcs("CBBCBC", "BBCCAB");
	printf("\nLCS length: %d\n", lcs_len);
	optimal_alignment("CBBCBC", "BBCCAB");

	return 0;	
}

