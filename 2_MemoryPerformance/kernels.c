/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#define min(a,b) (((a)<(b)) ? (a) : (b))
#define B 16


/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "Team Herbert!!!",              /* Team name */

    "Joo Young Kang",     /* First member full name */
    "justin.kang@mail.utoronto.ca",  /* First member email address */

    "He Zhang",                   /* Second member full name (leave blank if none) */
    "heherbert.zhang@mail.utoronto.ca"                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * ECE 454 Students: Write your rotate functions here:
 */ 

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    int i,j,x,y;

    //Loop interchange, ij to ji (read speed vs write speed)
	for (j = 0; j < dim; j+=32)
	{
		for (i = 0; i < dim; i+=32)
		{
			//Loop tiling (tile size = 32 by 32)
			//Loop unrolling on seoncd inner-most loop by 16times
			for (x=j; x<min(j+32,dim); x+=16)
			{
				int Z= x+(dim*dim)-dim;

				// Loop invariant code motion
				// Transforming Multiplication to Additions using multiple local variables.		
				int Y=x*dim;
				
				int Y2=Y + dim;
				int Y3=Y2 + dim;
				int Y4=Y3 + dim;
				int Y5=Y4 + dim;
				int Y6=Y5 + dim;
				int Y7=Y6 + dim;
				int Y8=Y7 + dim;
				int Y9=Y8 + dim;
				int Y10=Y9 + dim;
				int Y11=Y10 + dim;
				int Y12=Y11 + dim;
				int Y13=Y12 + dim;
				int Y14=Y13 + dim;
				int Y15=Y14 + dim;
				int Y16=Y15 + dim;

				// Most inner loop, RIDX function inlined.
				for (y=i; y<min(i+32,dim); y++)
				{
					dst[Z-(dim*y)] = src[Y+y];
					dst[Z-(dim*y)+1] = src[Y2+y];
					dst[Z-(dim*y)+2] = src[Y3+y];
					dst[Z-(dim*y)+3] = src[Y4+y];
					dst[Z-(dim*y)+4] = src[Y5+y];
					dst[Z-(dim*y)+5] = src[Y6+y];
					dst[Z-(dim*y)+6] = src[Y7+y];
					dst[Z-(dim*y)+7] = src[Y8+y];
					dst[Z-(dim*y)+8] = src[Y9+y];
					dst[Z-(dim*y)+9] = src[Y10+y];
					dst[Z-(dim*y)+10] = src[Y11+y];
					dst[Z-(dim*y)+11] = src[Y12+y];
					dst[Z-(dim*y)+12] = src[Y13+y];
					dst[Z-(dim*y)+13] = src[Y14+y];
					dst[Z-(dim*y)+14] = src[Y15+y];
					dst[Z-(dim*y)+15] = src[Y16+y];
				}
			}
		}
	}
}



char rotate_two_descr[] = "second attempt: Loop unrolling x4 from inner and outer loop (ij sequence)";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
    int i,j;
	int dim2 = dim-1;
	for (i=0;i<dim-3;i+=4)
	{
		for(j=0; j<(dim-3); j+=4)
		{
			dst[RIDX(dim2-j, i, dim)] 	  = src[RIDX(i, j, dim)];
			dst[RIDX(dim2-(j+1), i, dim)] = src[RIDX(i, j+1, dim)];
			dst[RIDX(dim2-(j+2), i, dim)] = src[RIDX(i, j+2, dim)];
			dst[RIDX(dim2-(j+3), i, dim)] = src[RIDX(i, j+3, dim)];
	
			dst[RIDX(dim2-j, i+1, dim)] 	= src[RIDX(i+1, j, dim)];
			dst[RIDX(dim2-(j+1), i+1, dim)] = src[RIDX(i+1, j+1, dim)];
			dst[RIDX(dim2-(j+2), i+1, dim)] = src[RIDX(i+1, j+2, dim)];
			dst[RIDX(dim2-(j+3), i+1, dim)] = src[RIDX(i+1, j+3, dim)];

			dst[RIDX(dim2-j, i+2, dim)] 	= src[RIDX(i+2, j, dim)];
			dst[RIDX(dim2-(j+1), i+2, dim)] = src[RIDX(i+2, j+1, dim)];
			dst[RIDX(dim2-(j+2), i+2, dim)] = src[RIDX(i+2, j+2, dim)];
			dst[RIDX(dim2-(j+3), i+2, dim)] = src[RIDX(i+2, j+3, dim)];

			dst[RIDX(dim2-j, i+3, dim)]		= src[RIDX(i+3, j, dim)];
			dst[RIDX(dim2-(j+1), i+3, dim)] = src[RIDX(i+3, j+1, dim)];
			dst[RIDX(dim2-(j+2), i+3, dim)] = src[RIDX(i+3, j+2, dim)];
			dst[RIDX(dim2-(j+3), i+3, dim)] = src[RIDX(i+3, j+3, dim)];
	
			}
	}  
}

char rotate_three_descr[] = "third attempt: Loop unrolling x16 on the second inner-most loop (ij sequence)";
void attempt_three(int dim, pixel *src, pixel *dst) 
{
    int i,j,x,y;

		
	for (i = 0; i < dim; i+=32)
	{
		for (j = 0; j < dim; j+=32)
		{
			for (x=i; x<min(i+32,dim); x+=16)
			{
				for (y=j; y<min(j+32,dim); y++)
				{
					dst[RIDX(dim-1-y, x, dim)] = src[RIDX(x, y, dim)];
					dst[RIDX(dim-1-y, x+1, dim)] = src[RIDX(x+1, y, dim)];
					dst[RIDX(dim-1-y, x+2, dim)] = src[RIDX(x+2, y, dim)];
					dst[RIDX(dim-1-y, x+3, dim)] = src[RIDX(x+3, y, dim)];
					dst[RIDX(dim-1-y, x+4, dim)] = src[RIDX(x+4, y, dim)];
					dst[RIDX(dim-1-y, x+5, dim)] = src[RIDX(x+5, y, dim)];
					dst[RIDX(dim-1-y, x+6, dim)] = src[RIDX(x+6, y, dim)];
					dst[RIDX(dim-1-y, x+7, dim)] = src[RIDX(x+7, y, dim)];
					dst[RIDX(dim-1-y, x+8, dim)] = src[RIDX(x+8, y, dim)];
					dst[RIDX(dim-1-y, x+9, dim)] = src[RIDX(x+9, y, dim)];
					dst[RIDX(dim-1-y, x+10, dim)] = src[RIDX(x+10, y, dim)];
					dst[RIDX(dim-1-y, x+11, dim)] = src[RIDX(x+11, y, dim)];
					dst[RIDX(dim-1-y, x+12, dim)] = src[RIDX(x+12, y, dim)];
					dst[RIDX(dim-1-y, x+13, dim)] = src[RIDX(x+13, y, dim)];
					dst[RIDX(dim-1-y, x+14, dim)] = src[RIDX(x+14, y, dim)];
					dst[RIDX(dim-1-y, x+15, dim)] = src[RIDX(x+15, y, dim)];

				}
			}
		}
	}
	  
}


char rotate_four_descr[] = "forth attempt: Code Motion and Loop unrolling the outer forloop (ij sequence) ";
void attempt_four(int dim, pixel *src, pixel *dst) 
{
    int i, j;
	int dim2=dim-1;
	int dim1=dim;
    for (i = 0; i < dim1-3; i+=4){
		for (j = 0; j < dim1; j++)
		{
			dst[(dim2-j)*(dim1)+i]=src[dim1*i+j];
			dst[(dim2-j)*(dim1)+(i+1)]=src[dim1*(i+1)+j];
			dst[(dim2-j)*(dim1)+(i+2)]=src[dim1*(i+2)+j];
			dst[(dim2-j)*(dim1)+(i+3)]=src[dim1*(i+3)+j];
	
		}
	}
}


char rotate_five_descr[] = "fifth attempt: Code motion only!! no loop un4ollint (ji sequence)";
void attempt_five(int dim, pixel *src, pixel *dst) 
{
	int i,j;
	int dim2 = dim-1;
	int dim1 = dim;


	for (j=0;j<dim1;j++)
	{
		int var_A = (dim2-j)*dim1;
		for(i=0; i<dim1; i++)
		{
			dst[var_A+i]=src[i*dim1+j];
			//dst[(dim2-j)*dim+i]=src[i*dim+j];
			//dst[RIDX(dim2 - j, i, dim)] = src[RIDX(i,j,dim)];
		}
	}

}

char rotate_six_descr[] = "sixth attempt: Looop unrolling 4 times each on inner&outer loop.  Loop interchange(ji sequence) ";
void attempt_six(int dim, pixel *src, pixel *dst) 
{


   	int i,j;
	int dim2 = dim-1;
	int dim1 = dim;


	for (j=0;j<(dim1-3);j+=4)
	{
		int varA = (dim2-j)*dim1;
		int varB = (dim2-(j+1))*dim1;
		int varC = (dim2-(j+2))*dim1;
		int varD = (dim2-(j+3))*dim1;
		
		for(i=0; i<(dim1-3); i+=4)
		{
			dst[varA+i]=src[i*dim1+j];
			dst[varA+(i+1)]=src[(i+1)*dim1+j];
			dst[varA+(i+2)]=src[(i+2)*dim1+j];
			dst[varA+(i+3)]=src[(i+3)*dim1+j];
			
			dst[varB+i]=src[i*dim1+(j+1)];
			dst[varB+(i+1)]=src[(i+1)*dim1+(j+1)];
			dst[varB+(i+2)]=src[(i+2)*dim1+(j+1)];
			dst[varB+(i+3)]=src[(i+3)*dim1+(j+1)];

			dst[varC+i]=src[i*dim1+(j+2)];
			dst[varC+(i+1)]=src[(i+1)*dim1+(j+2)];
			dst[varC+(i+2)]=src[(i+2)*dim1+(j+2)];
			dst[varC+(i+3)]=src[(i+3)*dim1+(j+2)];

			dst[varD+i]=src[i*dim1+(j+3)];
			dst[varD+(i+1)]=src[(i+1)*dim1+(j+3)];
			dst[varD+(i+2)]=src[(i+2)*dim1+(j+3)];
			dst[varD+(i+3)]=src[(i+3)*dim1+(j+3)];
		}
	}
}


char rotate_seven_descr[] = "seventh attempt: Loop tiling on first two loops TXT (32 by 32), then loop unrolling the second inner-most looop by 16 times. ji sequence";
void attempt_seven(int dim, pixel *src, pixel *dst) 
{


	int i,j,x,y;
	for (j = 0; j < dim; j+=32)
	{
		for (i = 0; i < dim; i+=32)
		{
			for (x=j; x<min(j+32,dim); x+=16)
			{
				for (y=i; y<min(i+32,dim); y++)
				{
					dst[RIDX(dim-1-y, x, dim)] = src[RIDX(x, y, dim)];
					dst[RIDX(dim-1-y, x+1, dim)] = src[RIDX(x+1, y, dim)];
					dst[RIDX(dim-1-y, x+2, dim)] = src[RIDX(x+2, y, dim)];
					dst[RIDX(dim-1-y, x+3, dim)] = src[RIDX(x+3, y, dim)];
					dst[RIDX(dim-1-y, x+4, dim)] = src[RIDX(x+4, y, dim)];
					dst[RIDX(dim-1-y, x+5, dim)] = src[RIDX(x+5, y, dim)];
					dst[RIDX(dim-1-y, x+6, dim)] = src[RIDX(x+6, y, dim)];
					dst[RIDX(dim-1-y, x+7, dim)] = src[RIDX(x+7, y, dim)];
					dst[RIDX(dim-1-y, x+8, dim)] = src[RIDX(x+8, y, dim)];
					dst[RIDX(dim-1-y, x+9, dim)] = src[RIDX(x+9, y, dim)];
					dst[RIDX(dim-1-y, x+10, dim)] = src[RIDX(x+10, y, dim)];
					dst[RIDX(dim-1-y, x+11, dim)] = src[RIDX(x+11, y, dim)];
					dst[RIDX(dim-1-y, x+12, dim)] = src[RIDX(x+12, y, dim)];
					dst[RIDX(dim-1-y, x+13, dim)] = src[RIDX(x+13, y, dim)];
					dst[RIDX(dim-1-y, x+14, dim)] = src[RIDX(x+14, y, dim)];
					dst[RIDX(dim-1-y, x+15, dim)] = src[RIDX(x+15, y, dim)];
				}
			}
		}
	}
}

char rotate_eight_descr[] = "eighth attempt: identical to the seventh one except ij sequence ";
void attempt_eight(int dim, pixel *src, pixel *dst) 
{
    int i,j,x,y;

		
	for (i = 0; i < dim; i+=32)
	{
		for (j = 0; j < dim; j+=32)
		{
			for (x=i; x<min(i+32,dim); x+=32)
			{
				for (y=j; y<min(j+32,dim); y++)
				{
					dst[RIDX(dim-1-y, x, dim)] = src[RIDX(x, y, dim)];
					dst[RIDX(dim-1-y, x+1, dim)] = src[RIDX(x+1, y, dim)];
					dst[RIDX(dim-1-y, x+2, dim)] = src[RIDX(x+2, y, dim)];
					dst[RIDX(dim-1-y, x+3, dim)] = src[RIDX(x+3, y, dim)];
					dst[RIDX(dim-1-y, x+4, dim)] = src[RIDX(x+4, y, dim)];
					dst[RIDX(dim-1-y, x+5, dim)] = src[RIDX(x+5, y, dim)];
					dst[RIDX(dim-1-y, x+6, dim)] = src[RIDX(x+6, y, dim)];
					dst[RIDX(dim-1-y, x+7, dim)] = src[RIDX(x+7, y, dim)];
					
					dst[RIDX(dim-1-y, x+8, dim)] = src[RIDX(x+8, y, dim)];
					dst[RIDX(dim-1-y, x+9, dim)] = src[RIDX(x+9, y, dim)];
					dst[RIDX(dim-1-y, x+10, dim)] = src[RIDX(x+10, y, dim)];
					dst[RIDX(dim-1-y, x+11, dim)] = src[RIDX(x+11, y, dim)];
					dst[RIDX(dim-1-y, x+12, dim)] = src[RIDX(x+12, y, dim)];
					dst[RIDX(dim-1-y, x+13, dim)] = src[RIDX(x+13, y, dim)];
					dst[RIDX(dim-1-y, x+14, dim)] = src[RIDX(x+14, y, dim)];
					dst[RIDX(dim-1-y, x+15, dim)] = src[RIDX(x+15, y, dim)];
					dst[RIDX(dim-1-y, x+16, dim)] = src[RIDX(x+16, y, dim)];
					dst[RIDX(dim-1-y, x+17, dim)] = src[RIDX(x+17, y, dim)];
					dst[RIDX(dim-1-y, x+18, dim)] = src[RIDX(x+18, y, dim)];
					dst[RIDX(dim-1-y, x+19, dim)] = src[RIDX(x+19, y, dim)];
					dst[RIDX(dim-1-y, x+20, dim)] = src[RIDX(x+20, y, dim)];
					dst[RIDX(dim-1-y, x+21, dim)] = src[RIDX(x+21, y, dim)];
					dst[RIDX(dim-1-y, x+22, dim)] = src[RIDX(x+22, y, dim)];
					dst[RIDX(dim-1-y, x+23, dim)] = src[RIDX(x+23, y, dim)];
					
					dst[RIDX(dim-1-y, x+24, dim)] = src[RIDX(x+24, y, dim)];
					dst[RIDX(dim-1-y, x+25, dim)] = src[RIDX(x+25, y, dim)];
					dst[RIDX(dim-1-y, x+26, dim)] = src[RIDX(x+26, y, dim)];
					dst[RIDX(dim-1-y, x+27, dim)] = src[RIDX(x+27, y, dim)];
					dst[RIDX(dim-1-y, x+28, dim)] = src[RIDX(x+28, y, dim)];
					dst[RIDX(dim-1-y, x+29, dim)] = src[RIDX(x+29, y, dim)];
					dst[RIDX(dim-1-y, x+30, dim)] = src[RIDX(x+30, y, dim)];
					dst[RIDX(dim-1-y, x+31, dim)] = src[RIDX(x+31, y, dim)];
					

				}
			}
		}
	}		
	
}
/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);   

    add_rotate_function(&attempt_two, rotate_two_descr);   
    add_rotate_function(&attempt_three, rotate_three_descr);   
    add_rotate_function(&attempt_four, rotate_four_descr);   
    add_rotate_function(&attempt_five, rotate_five_descr);   
    add_rotate_function(&attempt_six, rotate_six_descr);   
    add_rotate_function(&attempt_seven, rotate_seven_descr);   
    add_rotate_function(&attempt_eight, rotate_eight_descr);   
    //add_rotate_function(&attempt_nine, rotate_nine_descr);   
    //add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

    /* ... Register additional rotate functions here */
}

