#include "lib/string.h"

int strncmp ( char * s1, char * s2, int n)
{ 
	if(!n)return(0); //nΪ�޷������α���;���nΪ0,�򷵻�0

	//�ڽ�������while������
	//��һ��ѭ��������--n,����Ƚϵ�ǰn���ַ����˳�ѭ��
	//�ڶ���ѭ��������*s1,���s1ָ����ַ���ĩβ�˳�ѭ��
	//�ڶ���ѭ��������*s1 == *s2,������ַ��Ƚϲ������˳�ѭ��
	while (--n && *s1 && *s1 == *s2)
	{ 
		s1++; //S1ָ���Լ�1,ָ����һ���ַ�
		s2++;//S2ָ���Լ�1,ָ����һ���ַ�
	}
	return( *s1 - *s2 );//���رȽϽ��
}

//������ת����ASICA��
//����ǰ��� 0 ������ʾ����, ���� 0000B800 ����ʾ�� B800
char * itoa(char * str, int num)
{
	char *	p = str;
	char	ch;
	int	i;
	int	flag = false;
	//��ʾ16����ǰ���0x
	*p++ = '0';
	*p++ = 'x';
	//Ϊ0
	if(num == 0){
		*p++ = '0';
	}
	else{	//4λ4λ�ķֽ����
		for(i=28;i>=0;i-=4){		//����ߵ�4λ��ʼ
			ch = (num >> i) & 0xF;	//ȡ��4λ
			//if((ch > 0)){
				//flag = true;
			ch += '0';			//����0��+'0'���ASICA������
			if(ch > '9'){		//����9�ͼ���7���ASICA����ĸ
				ch += 7;		
			}
			*p++ = ch;			//ָ���ַ�ϼ�¼������
			
		}
	}
	*p = 0;							//�����ָ���ַ��Ӹ�0�����ַ�������
	return str;
}

void *memset(void* src, uint8_t value, uint32_t size) {
   uint8_t* s = (uint8_t*)src;
   while (size-- > 0){
	   *s++ = value;
   }
}

/* ��src_��ʼ��size���ֽڸ��Ƶ�dst_ */
void memcpy(void* dst_, const void* src_, uint32_t size) {
  
   uint8_t* dst = dst_;
   const uint8_t* src = src_;
   while (size-- > 0)
      *dst++ = *src++;
}

/* ���ַ�����src_���Ƶ�dst_ */
char* strcpy(char* dst_, const char* src_) {
  
   char* r = dst_;		       // ��������Ŀ���ַ�����ʼ��ַ
   while((*dst_++ = *src_++));
   return r;
}

/* �����ַ������� */
uint32_t strlen(const char* str) {
  
   const char* p = str;
   while(*p++);
   return (p - str - 1);
}

/* �Ƚ������ַ���,��a_�е��ַ�����b_�е��ַ�����1,���ʱ����0,���򷵻�-1. */
int8_t strcmp (const char* a, const char* b) {
  
   while (*a != 0 && *a == *b) {
      a++;
      b++;
   }
/* ���*aС��*b�ͷ���-1,���������*a���ڵ���*b��������ں���Ĳ������ʽ"*a > *b"��,
 * ��*a����*b,���ʽ�͵���1,����ͱ��ʽ������,Ҳ���ǲ���ֵΪ0,ǡǡ��ʾ*a����*b */
   return *a < *b ? -1 : *a > *b;
}

int memcmp(const void * s1, const void *s2, int n)
{
	if ((s1 == 0) || (s2 == 0)) { /* for robustness */
		return (s1 - s2);
	}

	const char * p1 = (const char *)s1;
	const char * p2 = (const char *)s2;
	int i;
	for (i = 0; i < n; i++,p1++,p2++) {
		if (*p1 != *p2) {
			return (*p1 - *p2);
		}
	}
	return 0;
}
char* strrchr(char* str, int c)
{
    char* ret = NULL;
    while (*str)
    {
        if (*str == (char)c)//������ڣ�ret�����ַ
            ret = (char *)str;//ǿ��ת��ָ�����ͣ�����ret 
        str++;
    }
    if ((char)c == *str) //����str��һ���ַ�Ϊ'\0'�����  
        ret = (char *)str; //ǿ��ת��ָ�����ͣ�����ret  

    return ret;
}

char* strcat(char* strDest , const char* strSrc)
{
    char* address=strDest;
    while(*strDest)//��while(*strDest!=��\0��)�ļ���ʽ
    {
        strDest++;
    }
    while(*strDest++=*strSrc++);
    return address;//Ϊ��ʵ����ʽ��������Ŀ�ĵ�ַ����
}

