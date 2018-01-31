#ifndef _HIGH_LEVEL_H___
#define _HIGH_LEVEL_H___

/* ��λȫ0����Nλȫ1 */
#define Low_N_Bits_One(N)     ((1<<N) -1)  
/* ��λȫ1����Nλȫ0 */
#define Low_N_Bits_Zero(N)    (~((1<<N) -1))  
/* ��λ��Nλ��1 */
#define Set_Bit_N(NUM, N)     (NUM | (1 << (N - 1)))  
/* ��λ��Nλ��0 */
#define Clear_Bit_N(NUM, N)   (NUM & (~(1 << (N - 1))))  
/* ��λ��Nλ��ת */
#define Reverse_Bit_N(NUM, N) ((NUM) ^ (1 << (N - 1)))  
/* ��ȡ�� */
#define UpRoun8(Num)  (((Num) + 0x7)  & (~0x7))  
#define UpRoun16(Num) (((Num) + 0xf)  & (~0xf))  
#define UpRoun32(Num) (((Num) + 0x1f) & (~0x1f))  
/* ��ȡ�� */
#define LowRoun8(Num)  ((Num) & (~0x7))  
#define LowRoun16(Num) ((Num) & (~0xf))  
#define LowRoun32(Num) ((Num) & (~0x1f))  
/* ���� */
#define Div8(Num)         ((Num)>>3)  
#define Div16(Num)        ((Num)>>4)  
#define Div32(Num)        ((Num)>>5)  
/* ������λ���� */
#define UpDiv8(Num)        (((Num)>>3) + !!((Num) & 0x7))  
#define UpDiv16(Num)        (((Num)>>4) + !!((Num) & 0xf))  
#define UpDiv32(Num)        (((Num)>>5) + !!((Num) & 0x1f))  
/* ���� */
#define M8(Num)        ((Num) & 0x7)  
#define M16(Num)       ((Num) & 0xf)  
#define M32(Num)       ((Num) & 0x1f)  
/* ����
������С��x������(Num + x) % ģ�� == 0
���� RM16(Num) �ȼ��� (16 - Num%16) % 16 */
#define RM8(Num)        ((~(Num) + 1) & 0x7)  
#define RM16(Num)        ((~(Num) + 1) & 0xf)  
#define RM32(Num)        ((~(Num) + 1) & 0x1f)  

#endif _HIGH_LEVEL_H__