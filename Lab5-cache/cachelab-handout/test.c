#include <stdio.h>

int A[64][64],B[64][64];

void trans()
{
    int a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12;
    // 遍历矩阵A的每个8x8的分块
    for(int i=0; i<64; i+=8)
        for(int j=0; j<64; j+=8){
            // Step1：将A0 A1转给B0 B1
            for(int k=i; k<i+4; k++){
                a1 = A[k][j];
                a2 = A[k][j+1];
                a3 = A[k][j+2];
                a4 = A[k][j+3];
                a5 = A[k][j+4];
                a6 = A[k][j+5];
                a7 = A[k][j+6];
                a8 = A[k][j+7];

                B[j][k] = a1;
                B[j+1][k] = a2;
                B[j+2][k] = a3;
                B[j+3][k] = a4;
                B[j+0][k+4] = a5;
                B[j+1][k+4] = a6;
                B[j+2][k+4] = a7;
                B[j+3][k+4] = a8;
            }
            for(int k=j; k<j+4; k++){
                // 复制矩阵B右上角分块到local
                a1 = B[k][i+4];
                a2 = B[k][i+5];
                a3 = B[k][i+6];
                a4 = B[k][i+7];
                // 把矩阵A左下角分块A2复制到矩阵B右上角分块
                a5 = A[i+4][k];
                a6 = A[i+5][k];
                a7 = A[i+6][k];
                a8 = A[i+7][k];

                a9 = A[i+4][k+4];
                a10 = A[i+4][k+5];
                a11 = A[i+4][k+6];
                a12 = A[i+4][k+7];

                B[k][i+4] = a5;
                B[k][i+5] = a6;
                B[k][i+6] = a7;
                B[k][i+7] = a8;
                // 把local复制到矩阵B的右下角
                B[k+4][i] = a1;
                B[k+4][i+1] = a2;
                B[k+4][i+2] = a3;
                B[k+4][i+3] = a4;

                B[j+4][i+4+(k-j)] = a9;
                B[j+5][i+4+(k-j)] = a10;
                B[j+6][i+4+(k-j)] = a11;
                B[j+7][i+4+(k-j)] = a12;
            }
        }
}
int main()
{
    int cnt=0;
    for(int i=0; i<64; i++)
        for(int j=0; j<64; j++)
            A[i][j] = cnt++;

    for(int i=0; i<64; i++)
    {
        for(int j=0; j<64; j++)
        {
            printf("%d ", A[i][j]);
        }
        printf("\n");
    }
    trans();
    for(int i=0; i<64; i++)
    {
        for(int j=0; j<64; j++)
        {
            printf("%d ", B[i][j]);
        }
        printf("\n");
    }
    
    return 0;
}