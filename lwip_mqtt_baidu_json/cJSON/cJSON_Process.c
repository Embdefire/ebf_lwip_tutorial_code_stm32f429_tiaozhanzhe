#include "cJSON_Process.h"
#include "main.h"

/*******************************************************************
 *                          ��������                               
 *******************************************************************/



cJSON* cJSON_Data_Init(void)
{
  cJSON* cJSON_Root = NULL;    //json���ڵ�
  
  cJSON_Root = cJSON_CreateObject();   /*������Ŀ*/
  if(NULL == cJSON_Root)
  {
      return NULL;
  }
  cJSON_AddStringToObject(cJSON_Root, NAME, DEFAULT_NAME);  /*���Ԫ��  ��ֵ��*/
  cJSON_AddNumberToObject(cJSON_Root, NUMBER, DEFAULT_NUMBER);
  cJSON_AddBoolToObject(cJSON_Root, BOOL, DEFAULT_BOOL);
  
  char* p = cJSON_Print(cJSON_Root);  /*p ָ����ַ�����json��ʽ��*/
  
//  PRINT_DEBUG("%s\n",p);
  
  vPortFree(p);
  p = NULL;
  
  return cJSON_Root;
  
}

char* cJSON_Update(const cJSON * const object,const char * const string,void *d)
{
  cJSON* node = NULL;    //json���ڵ�
  node = cJSON_GetObjectItem(object,string);
  if(node == NULL)
    return NULL;
  if(cJSON_IsBool(node))
  {
    int *b = (int*)d;
//    printf ("d = %d",*b);
    cJSON_GetObjectItem(object,string)->type = *b ? cJSON_True : cJSON_False;
    char* p = cJSON_Print(object);    /*p ָ����ַ�����json��ʽ��*/
    return p;
  }
  else if(cJSON_IsString(node))
  {
    cJSON_GetObjectItem(object,string)->valuestring = (char*)d;
    char* p = cJSON_Print(object);    /*p ָ����ַ�����json��ʽ��*/
    return p;
  }
  else if(cJSON_IsNumber(node))
  {
    double *num = (double*)d;
    printf ("num = %f",*num);
    cJSON_GetObjectItem(object,string)->valueint = (double)*num;
    cJSON_GetObjectItem(object,string)->valuedouble = (double)*num;
    char* p = cJSON_Print(object);    /*p ָ����ַ�����json��ʽ��*/
    return p;
  }
  else
    return NULL;
}

//char *makeJson()
//{
//    char * p;
//    char * q;
//    cJSON * cJSON_Root = NULL;    //json���ڵ�
//    cJSON_Root = cJSON_CreateObject();   /*������Ŀ*/
//    if(NULL == cJSON_Root)
//    {
//        return NULL;
//    }
//    cJSON_AddStringToObject(cJSON_Root, "hello", "hello world"); /*���Ԫ��  ��ֵ��*/
//    cJSON_AddNumberToObject(cJSON_Root, "number", 10010);
//    cJSON_AddBoolToObject(cJSON_Root, "bool", 1);

//    cJSON * pSubJson = NULL;
//    pSubJson = cJSON_CreateObject();    /*������Ŀ*/
//    if(NULL == pSubJson)
//    {
//        cJSON_Delete(cJSON_Root);
//        return NULL;
//    }
//    cJSON_AddStringToObject(pSubJson, "subjsonobj", "a sub json string");  /*���Ԫ�� ��ֵ��*/
//    cJSON_AddItemToObject(cJSON_Root, "subobj", pSubJson);   /*���´�����pSubJson����cJSON_Root�б���*/

//    p = cJSON_Print(cJSON_Root);  /*p ָ����ַ�����json��ʽ��*/
//    
//    printf("1:%s\n", p);
//    
//    vPortFree(p);
//    p = NULL;
////    
//    cJSON_GetObjectItem(cJSON_Root,"number")->valueint = 10;
//    cJSON_GetObjectItem(cJSON_Root,"number")->valuedouble = 10;
//    q = cJSON_Print(cJSON_Root);  /*p ָ����ַ�����json��ʽ��*/
//    printf("2:%s\n", q);
//    
//    vPortFree(q);
//    q = NULL;
//    if(NULL == p)
//    {
//        //convert json list to string faild, exit
//        //because sub json pSubJson han been add to cJSON_Root, so just delete cJSON_Root, if you also delete pSubJson, it will coredump, and error is : double free
//        cJSON_Delete(cJSON_Root);   /*����ɾ��pSubJson*/
//        return NULL;
//    }

//    cJSON_Delete(cJSON_Root);    /*ɾ���б�   ����ڴ��cJSON_Print���ص��ڴ治һ��*/

//    
//    
//    return 0;
//}


//void test(void)
//{
//  cJSON_Item test;
//  
//  cJSON_Item test1;
//  
//  test.type = cJSON_Number;
//  test.name = "abc";
//  test.valuedouble = 123;
//  
//  test1.type = cJSON_String;
//  test1.name = "def";
//  test1.valuestring = "test1"; 
//  
//  test.child = &test1;
//}







