// 要求：阅读后面的文件管理代码，完成读、写文件的操作
//（最好是能读本地磁盘字符文件并显示，能将用户从键盘上输入的符号串写本地磁盘文件）。

//文件目录用单向链链接，链尾的最后一个目录总为空目录（文件名为空）。
//打开的文件创建对应的活动文件目录，关闭的文件删除对应的活动文件目录。
#pragma comment(linker, "/ENTRY:main")

#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#define TRUE 1
#define FALSE 0

//用户文件目录结构
struct dataEntry
{
    char filename[6];//文件名
    char pcode[3];//为3位代码,分别对应允许读、允许写、允许执行，值为'1'代表允许，为'0'代表不允许。
    int length;//文件长度
    int addr;//文件空间首地址
    dataEntry* next;
};
dataEntry* entryPtr1;
dataEntry* entryPtr2;

//主文件目录结构
struct dataMain
{
    char username[6];//用户名
    dataEntry* link;//用户文件目录链的链首指针
};
dataMain* userPtr1;
dataMain* userPtr2;
dataMain* mainUser[5];//对应5个用户

//活动文件目录结构
struct dataActive
{
    char filename[6];
    char pcode[3];//存取控制保护码
    int rpointer;//当前读指针
    int wpointer;//当前写指针
    int addr;//文件空间首地址
    dataActive* next;
};
dataActive* entryBeginPtr;//活动文件目录链的链首指针
dataActive* ActiveEntryPtr1;
dataActive* ActiveEntryPtr2;

static int openNumber=0;
char name[6];//当前操作的文件名

void main();
void init();
void pcreate();
void popen();
void close1(int s);
void pclose();
void pdelete();
void pread();
void pwrite();
void pdirectory();


//---------------------------------------------------------------------------------------
void main()
{ 
    int i,flag;
    
    init();
    flag=TRUE;
    while(flag)
    {
        printf("\n选择功能：1-创建文件  2-打开文件  3-关闭文件  4-删除文件  \n");
        printf("          5-读文件    6-写文件    7-目录      8-退出\n");
        scanf("%d",&i);
        switch (i)
        {
        case 1:
            {
                pcreate();
                break;
            }
        case 2:
            {
                popen();
                break;
            }
        case 3:
            {
                pclose();
                break;
            }
        case 4:
            {
                pdelete();
                break;
            }
        case 5:
            {
                pread();
                break;
            }
        case 6:
            {
                pwrite();
                break;
            }
        case 7:
            {
                pdirectory();
                break;
            }
        case 8:
            flag=FALSE;
        }//switch (i)
        
    }//while(flag)
}



//--------------------------------------------------------------------------
//为每个用户建立一个主目录及空的文件目录。整个系统一个空的活动文件目录。
void init()
{
    int usernum,i,j;
    printf("用户数量（<5）?");
    scanf("%d",&usernum);
    if (usernum>5)
        usernum=5;
    printf("开始用户的注册。\n");
    
    //为每个用户建立主目录
    for (i=0;i<usernum;i++)
    { //创建一个主目录
        mainUser[i]=new dataMain;
        for(j=0;j<6;j++)
            (mainUser[i]->username)[j]=0;
        printf("用户%d的名字？(<=5个字符)",i);
        scanf("%s",mainUser[i]->username);//指定用户名
        
        entryPtr1=new dataEntry;
        entryPtr1->next=NULL;
        for(j=0;j<6;j++)
            (entryPtr1->filename)[j]=0;//空的文件目录
        mainUser[i]->link=entryPtr1;//指定与该用户相关联的文件目录
        
    }
    //不足5个用户时，余下主目录项为空。
    for (i=usernum;i<5;i++)
        mainUser[i]=NULL;
    //建立空的活动文件目录项entryBeginPtr
    entryBeginPtr=new dataActive;
    entryBeginPtr->next=NULL;
    for(j=0;j<6;j++)
        (entryBeginPtr->filename)[j]=0; 
}



//--------------------------------------------------------------------------
//最多同时打开5个文件。
void pcreate()
{
    char code[3];//保护码
    int i,j,useri;
    
    for (i=0;i<6;i++)
        name[i]=0;
    printf("用户名字?");
    scanf("%s",name);
    //检查是否合法用户
    for (useri=0;useri<5;useri++)
    {
        j=0;
        while (j<6 && ((mainUser[useri]->username)[j]==name[j]) ) 
            j++;
        if (j==6)
            break;
    }
    if (useri==5)
    {
        printf("你不是注册用户。\n");
        return;
    }
    //用户为第useri用户
    for (i=0;i<6;i++)
        name[i]=0;
    printf("新建的文件名？");
    scanf("%s",name);
    //寻找该用户的最后一个文件目录
    entryPtr1=mainUser[useri]->link;//取该用户文件目录链的链首指针
    while ((entryPtr1->next)!=NULL)
        entryPtr1=entryPtr1->next;
    //为新目录申请节点空间
    entryPtr2=new dataEntry;
    //把entryPtr2加入链尾.即entryPtr1的下一个节点是entryPtr2,entryPtr2是空目录。
    entryPtr2->next=NULL;
    for (i=0;i<6;i++)
        (entryPtr2->filename)[i]=' ';
    entryPtr1->next=entryPtr2;
    for (i=0;i<6;i++)
        (entryPtr1->filename)[i]=name[i];
    printf("请输入文件的保护码(如111，对应顺序：读写（执行），1允许，0不允许。)：");
    scanf("%s",code);
    for (i=0;i<3;i++)
        entryPtr1->pcode[i]=code[i];
    entryPtr1->length=0;//初始文件长度为0
    printf("文件创建成功!\n");
}


//--------------------------------------------------------------------------
//打开文件。寻找该文件，若寻找到，增加相应的活动文件目录。
void popen()
{
    int i,j,k,useri;
    bool flag;//是否找到文件
    
    for (i=0;i<6;i++)
        name[i]=0;
    printf("打开的文件名字?");
    scanf("%s",name);
    //在所有文件中查找该文件
    flag=false;
    for (useri=0;useri<5;useri++)
    {  //在第useri个用户处寻找
        entryPtr1=mainUser[useri]->link;
        while (entryPtr1!=NULL)
        {
            for (j=0;j<6;j++)
            {
                if ((entryPtr1->filename)[j]!=name[j])
                    break;
            }
            if (j==6)
            {
                flag=true;
                break;//找到该文件,是第useri用户的entryPtr1目录	
            }
            else
                entryPtr1=entryPtr1->next;//检查下一个文件目录
        } //while ((entryPtr1->next)!=NULL)
        if (flag)
            break;
    }//for (useri=0;useri<5;useri++)
    if (!flag)
    {
        printf("该文件尚未创建。\n");
        return;
    }
    //已经找到文件，是第useri用户的entryPtr1目录.为该文件生成一个活动文件目录。
    if (openNumber>5)
    {
        printf("打开的文件数已经超过最大限定(5个)。不能打开该文件.\n");
        return;
    }
    //打开文件
    openNumber++;
    //找到活动文件目录链的尾部
    ActiveEntryPtr2=entryBeginPtr;//取活动文件目录链首
    while ((ActiveEntryPtr2->next)!=NULL)
        ActiveEntryPtr2=ActiveEntryPtr2->next;
    ActiveEntryPtr1=new dataActive;//申请新的活动目录空间
    //ActiveEntryPtr1作为链尾加入。ActiveEntryPtr2的下一个节点是ActiveEntryPtr1。
    ActiveEntryPtr1->next=NULL;
    ActiveEntryPtr2->next=ActiveEntryPtr1;
    for (k=0;k<6;k++)
        (ActiveEntryPtr2->filename)[k]=name[k];
    for (k=0;k<3;k++)
        (ActiveEntryPtr2->pcode)[k]=(entryPtr1->pcode)[k];
    ActiveEntryPtr2->rpointer=0;
    ActiveEntryPtr2->wpointer=0;
    ActiveEntryPtr2->addr = 0;
    printf("文件打开成功。\n");
}

//--------------------------------------------------------------------------
//关闭文件。在活动目录中寻找文件，并删除相应节点。
//status为1，不提示关闭信息，用于删除文件之前的文件关闭。status为0，提示关闭信息，用于文件关闭操作。
void close1(int status)
{
    int i,j;
    bool flag;
    
    for (i=0;i<6;i++)
        name[i]=0;
    printf("文件名字？");
    scanf("%s",name);
    //在活动文件目录链中寻找该文件
    flag=false;
    ActiveEntryPtr1=NULL;//前一个节点
    ActiveEntryPtr2=entryBeginPtr;
    while (ActiveEntryPtr2!=NULL)
    {
        for (j=0;j<6;j++)
        {
            if ((ActiveEntryPtr2->filename)[j]!=name[j])
                break;
        }
        if (j==6)
        {
            flag=true;
            break;//找到该文件ActiveEntryPtr2	
        }
        else
        {
            ActiveEntryPtr1=ActiveEntryPtr2;//保存前一个节点
            ActiveEntryPtr2=ActiveEntryPtr2->next;//检查下一个文件目录
        }
    }//while (ActiveEntryPtr2!=NULL)
    if (!flag)
    {
        if (status==0)
            printf("该文件尚未打开。\n");
        return;
    }
    //关闭文件，删除节点ActiveEntryPtr2, ActiveEntryPtr2的前一节点是ActiveEntryPtr1.
    if (ActiveEntryPtr2==entryBeginPtr)   //链首
        entryBeginPtr=ActiveEntryPtr2->next;
    else
        ActiveEntryPtr1->next=ActiveEntryPtr2->next;
    openNumber--;
    delete ActiveEntryPtr2;//释放节点空间
    if (status==0)
        printf("文件成功关闭。\n");
}

//-------------------------------------------------------------------
void pclose()
{
    close1(0);//关闭文件
}


//--------------------------------------------------------------------------
//删除文件。首先关闭文件，然后在全部用户文件目录中寻找并删除相应文件目录。
void pdelete()
{
    int j,useri;
    bool flag;
    
    //关闭打开的文件name
    close1(1);
    //在全部的用户中寻找文件name并删除相应目录
    for (useri=0;useri<5;useri++)
    { //用户i
        flag=false;
        entryPtr2=NULL;//前一个节点
        entryPtr1=mainUser[useri]->link;//取文件目录链的首节点
        while (entryPtr1!=NULL)
        {
            for (j=0;j<6;j++)
            {
                if ((entryPtr1->filename)[j]!=name[j])
                    break;
            }
            if (j==6)
            {
                flag=true;
                break;//找到该文件entryPtr1,属于第useri个用户。	
            }
            else
            {
                entryPtr2=entryPtr1;//保留前一个节点
                entryPtr1=entryPtr1->next;//检查下一个文件目录
            }
        }//while (entryPtr1!=NULL)
        if (flag)       
            break;
    }//for (useri=0;useri<5;useri++)
    if (!flag)
    {
        printf("文件没有创建。\n");
        return;
    }
    //删除entryPtr1目录，其前一个目录是entryPtr2.
    if (entryPtr1==mainUser[useri]->link)   //entryPtr1是链首
        mainUser[useri]->link=entryPtr1->next;
    else
        entryPtr2->next=entryPtr1->next;
    delete entryPtr1;
    printf("文件%s被成功删除。\n",name);
}


//--------------------------------------------------------------------------
void pread()
{
    FILE *fp;
    dataActive *p;

    printf("确实要将test.txt读入到哪个文件: ");
    fgets(name, sizeof(name), stdin);
    
    p = entryBeginPtr;
    while (p->next)
    {
        if (strncmp(p->filename, name, sizeof(name)) == 0)
            break;
    }
    if (p->next == NULL)
    {
        printf("找不到 %s.\n", name);
        return;
    }

    fp = fopen("test.txt", "rb");
    p->
}


//--------------------------------------------------------------------------
void pwrite()
{
    printf("current opened file: %s\n", name);
}


//--------------------------------------------------------------------------
//查询主目录或者用户目录。
void pdirectory()
{  
    char sel,ppcode[4];
    int i,j,fileNumber,totalNumber,useri;
    
    printf("查询主目录（1）还是用户目录（2）？");
    scanf("%d",&sel);
    totalNumber=0;//统计总的文件数
    if (sel==1)
    {//查询主目录
        printf("主目录：\n");
        printf("用户名                文件数量\n");
        for(useri=0;useri<5;useri++)
        {
            if (mainUser[useri]!=NULL)
            {
                fileNumber=0;//用于统计该用户的文件数
                entryPtr1=mainUser[useri]->link;
                while (entryPtr1!=NULL)
                {
                    fileNumber++;//统计该用户的文件数
                    entryPtr1=entryPtr1->next;
                }
                fileNumber--;//最后一个为空目录
                printf("%s                %d\n",mainUser[useri]->username,fileNumber);
                totalNumber+=fileNumber;//统计总的文件数
            }
        }//for(useri=0;useri<5;useri++)
        printf("总的文件数=%d\n",totalNumber);
    }
    else
    {
        //查询用户目录
        for (i=0;i<6;i++)
            name[i]=0;
        printf("用户名字？");
        scanf("%s",name);
        for (useri=0;useri<5;useri++)
        {
            for (j=0;j<6;j++)
            {
                if ((mainUser[useri]->username)[j]!=name[j])
                    break;
            }
            if (j==6)
                break;//查到该用户，是第useri个用户
        } //for (useri=0;useri<5;useri++)
        if (useri==5)
        {
            printf("用户目录不存在。\n");
            return;
        }
        //显示第useri个用户的目录文件
        printf("用户%s的目录：\n",name);
        printf("文件名      保护码      长度\n");
        entryPtr1=mainUser[useri]->link;
        while (entryPtr1!=NULL)
        {
            for (i=0;i<3;i++)
                ppcode[i]=entryPtr1->pcode[i];
            ppcode[3]=0;
            if (entryPtr1->next!=NULL)//最后一个为空节点
                printf("%s          %s         %d\n",entryPtr1->filename,ppcode,entryPtr1->length);
            entryPtr1=entryPtr1->next;
        }
    }
}




