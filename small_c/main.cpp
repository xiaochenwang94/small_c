//
//  main.cpp
//  small_c
//
//  Created by Wang Bill on 16/10/12.
//  Copyright © 2016年 Wang Bill. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <string.h>


using namespace std;


#define nrow 16       /* 保留字个数 */
#define txmax 100     /* 符号表容量 */
#define nmax 14       /* 数字的最大位数 */
#define al 10         /* 标识符的最大长度 */
#define maxerr 30     /* 允许的最多错误数  */
#define amax 2048     /* 地址上界*/
#define levmax 3      /* 最大允许过程嵌套声明层数*/
#define cxmax 200     /* 最多的虚拟机代码数 */
#define stacksize 500 /* 运行时数据栈元素最多为500个 */

/* 符号 */
enum symbol {
    nul,        ident,     number,      plus2,      minus2,
    times,      slash,     eql,         neq,        lss,
    leq,        gtr,       geq,         lparen,     mod,
    rparen,     comma,     semicolon,   period,     becomes,
    callsym,    constsym,  dosym,       elsesym,    endsym,
    forsym,     ifsym,     oddsym,      procsym,    procendsym,
    readsym,    repeatsym, thensym,     untilsym,   whilesym,
    writesym,   dollar,    incsym,      decsym
};
#define symnum 39

enum object {
    constant,
    variable,
    procedure,
};

/* 虚拟机代码指令 */
enum fct {
    lit,     opr,     lod,
    sto,     cal,     ini,
    jmp,     jpc,
};
#define fctnum 8

/* 虚拟机代码结构 */
struct instruction
{
    enum fct f; /* 虚拟机代码指令 */
    int l;      /* 引用层与声明层的层次差 */
    int a;      /* 根据f的不同而不同 */
};

/* 符号表结构 */
struct tablestruct
{
    char name[al];	    /* 名字 */
    enum object kind;	/* 类型：const，var或procedure */
    int val;            /* 数值，仅const使用 */
    int level;          /* 所处层，仅const不使用 */
    int adr;            /* 地址，仅const不使用 */
    int size;           /* 需要分配的数据区空间, 仅procedure使用 */
};

FILE* fin;      /* 输入源文件 */
FILE* ftable;	/* 输出符号表 */
FILE* fcode;    /* 输出虚拟机代码 */
FILE* foutput;  /* 输出文件及出错示意（如有错）、各行对应的生成代码首地址（如无错） */
FILE* fresult;  /* 输出执行结果 */
int err;        /* 错误计数器 */
char fname[al];

bool listswitch ;   /* 显示虚拟机代码与否 */
bool tableswitch ;  /* 显示符号表与否 */
char ch;            /* 存放当前读取的字符，getch 使用 */
enum symbol sym;    /* 当前的符号 */
char id[al+1];      /* 当前ident，多出的一个字节用于存放0 */
int num;            /* 当前number */
int cc, ll;         /* getch使用的计数器，cc表示当前字符(ch)的位置 */
int cx;             /* 虚拟机代码指针, 取值范围[0, cxmax-1]*/
char line[81];      /* 读取行缓冲区 */
char a[al+1];       /* 临时符号，多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
char word[nrow][al];        /* 保留字 */
enum symbol wsym[nrow];     /* 保留字对应的符号值 */
enum symbol ssym[256];      /* 单字符的符号值 */
char mnemonic[fctnum][5];   /* 虚拟机代码指令名称 */
bool declbegsys[symnum];    /* 表示声明开始的符号集合 */
bool statbegsys[symnum];    /* 表示语句开始的符号集合 */
bool facbegsys[symnum];     /* 表示因子开始的符号集合 */
int dx = 0;                 // dx代表要预留的空间
bool isEnd = false;
bool isPost = false;
enum symbol oldsym;

struct tablestruct table[txmax]; /* 符号表 */


void init();
void getsym();
void getch();
void error(int n);
int addset(bool* sr, bool* s1, bool* s2, int n);
void listall();
void interpret();
void gen(enum fct x, int y, int z);
void program(int tx, bool* fsys);
void statment(int *tx, bool* fsys);
void stmt_sequence(int *tx, bool* fsys);
void if_stmt(int *tx, bool* fsys);
void repeat_stmt(int *tx, bool* fsys);
void assign_stmt(int *tx, bool* fsys, int pos);
void read_stmt(int *tx, bool* fsys);
void write_stmt(int *tx, bool* fsys);
void exp(int *tx, bool* fsys);
int position(char* idt, int tx);
void simple_exp(int *tx, bool *fsys);
void term(int *tx, bool *fsys);
void factor(int *tx, bool *fsys);
void test(bool* s1, bool* s2, int n);
int inset(int e, bool* s);
int base(int l, int* s, int b);
void enter(enum object k, int *ptx, int *pdx);
void incr(int pre, int post);
void decr(int pre, int post);



int main(int argc, const char * argv[]) {
    bool nxtlev[symnum];                    //跟随符号集合
    cout<<"Input small_c file?\t";
    scanf("%s", fname);		/* 输入文件名 */
    
    if ((fin = fopen(fname, "r")) == NULL)
    {
        printf("Can't open the input file!\n");
        exit(1);
    }

    ch = fgetc(fin);
    if (ch == EOF)
    {
        printf("The input file is empty!\n");
        fclose(fin);
        exit(1);
    }
    rewind(fin);
    
    if ((foutput = fopen("foutput.txt", "w")) == NULL)
    {
        printf("Can't open the output file!\n");
        exit(1);
    }
    
    if ((ftable = fopen("ftable.txt", "w")) == NULL)
    {
        printf("Can't open ftable.txt file!\n");
        exit(1);
    }
    
    printf("List object codes?(Y/N)");	/* 是否输出虚拟机代码 */
    scanf("%s", fname);
    listswitch = (fname[0]=='y' || fname[0]=='Y');
    
    printf("List symbol table?(Y/N)");	/* 是否输出符号表 */
    scanf("%s", fname);
    tableswitch = (fname[0]=='y' || fname[0]=='Y');
    
    init();
    err = 0;
    cc = ll = cx = 0;
    ch = ' ';
    
    addset(nxtlev, declbegsys, statbegsys, symnum);
    nxtlev[nul] = true;
    getsym();
    program(0, nxtlev); //处理分程序

    if (err == 0){
        printf("\n===Parsing success!===\n");
        fprintf(foutput,"\n===Parsing success!===\n");
        
        if ((fcode = fopen("fcode.txt", "w")) == NULL)
        {
            printf("Can't open fcode.txt file!\n");
            exit(1);
        }
        
        if ((fresult = fopen("fresult.txt", "w")) == NULL)
        {
            printf("Can't open fresult.txt file!\n");
            exit(1);
        }
        
        listall();	 /* 输出所有代码 */
        fclose(fcode);
        
        interpret();	/* 调用解释执行程序 */
        fclose(fresult);
    }
    else{
        printf("\n%d errors in small_c program!\n",err);
        fprintf(foutput,"\n%d errors in small_c program!\n",err);
    }
    
    fclose(ftable);
    fclose(foutput);
    fclose(fin);

    return 0;
}



/*
 * 编译程序主体
 *
 * tx:      符号表当前尾指针
 * fsys:    当前模块后继符号集合
 */

void program(int tx, bool* fsys){
    int tx0;            //保留初始tx
    int cx0;            //保留初始cx
    
    dx = 3;         //三个空间用于存放SL, DL, RAg
    tx0 = tx;           //记录本层标识符的初始位置
    table[tx].adr = cx; //记录代码开始位置
    cx0 = cx;
    gen(ini, 0, 0);     //程序开始预留的空间数量不能确定，因此先写为0，后面根据全局变量dx，预留相应空间。
    stmt_sequence(&tx, fsys);
    code[cx0].a = dx;
    gen(opr, 0, 0);
}

void stmt_sequence(int *tx, bool* fsys) {
    bool nxtlev[symnum];
    memcpy(nxtlev, fsys, symnum*sizeof(bool));
    nxtlev[semicolon] = true;
    while(1){
        statment(tx, nxtlev);
        if(sym == semicolon){
            getsym();
        } else {
            break;
        }
    }
}

void statment(int *tx, bool* fsys) {
    int i = 0;
    bool nxtlev[symnum];
    memset(nxtlev, 0, sizeof(bool)*symnum);
    switch (sym) {
        case ifsym:
            getsym();
            if_stmt(tx, fsys);
            break;
        case repeatsym:
            getsym();
            repeat_stmt(tx, fsys);
            break;
        case ident:
            enter(variable, tx, &dx);
            i = position(id, *tx);
            getsym();
            assign_stmt(tx, fsys, i);
            break;
        case readsym:
            getsym();
            read_stmt(tx, fsys);
            break;
        case writesym:
            getsym();
            write_stmt(tx, fsys);
            break;
        default:
            break;
    }
}

void repeat_stmt(int *tx, bool* fsys){
    int cx0 = cx;
    stmt_sequence(tx, fsys);
    bool nxtlev[symnum];
//    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
    nxtlev[untilsym] = true;
    test(nxtlev, fsys, 7);
    if(sym == untilsym){
        getsym();
        exp(tx, fsys);
        gen(lit, 0, 1);
        gen(opr, 0, 8);
        gen(jpc, 0, cx0);
    }
}

void assign_stmt(int *tx, bool* fsys, int pos){
    bool nxtlev[symnum];
    memcpy(nxtlev, fsys, symnum*sizeof(bool));
    nxtlev[becomes] = true;
    test(nxtlev, fsys, 10);
    if(sym == becomes){
        getsym();
        exp(tx, fsys);
        if(table[pos].kind != variable) error(33);                     // error 33: only variable can be assigned
        gen(sto,0,table[pos].adr);
    }
}

void read_stmt(int *tx, bool* fsys){
    int i;
    bool nxtlev[symnum];
    memset(nxtlev, 0, symnum*sizeof(bool));
    nxtlev[ident] = true;
    test(nxtlev, fsys, 32);         //error sym follows read must be ident
    if(sym == ident){
        i = position(id,*tx);
    } else {
        i = 0;
    }
    if(i==0){
        enter(variable, tx, &dx);
        i=(*tx);
    }
    gen(opr, 0, 16);	/* 生成输入指令，读取值到栈顶 */
    gen(sto, 0, table[i].adr);	/* 将栈顶内容送入变量单元中 */
    
    getsym();
}

void write_stmt(int *tx, bool* fsys){
    bool nxtlev[symnum];
    memset(nxtlev, 0, symnum*sizeof(bool));
    nxtlev[ident] = true;
    exp(tx, fsys);
    gen(opr, 0, 14);
    gen(opr, 0, 15);
}

void if_stmt(int *tx, bool* fsys){
    bool nxtlev[symnum];
    nxtlev[thensym] = true;
    exp(tx,fsys);
    int cx0 = cx;
    int elsecx = -1;
    gen(jpc, 0, 0);
    test(nxtlev, fsys, 4);
    if(sym == thensym){
        getsym();
        stmt_sequence(tx, fsys);
        nxtlev[thensym] =false;
        addset(nxtlev, declbegsys, facbegsys, symnum);
        nxtlev[elsesym] = true;
        nxtlev[endsym] = true;
        test(nxtlev, fsys, 5);
        bool flag = false;
        int cx1 = cx;
        gen(jmp, 0, 0);
        while (sym == elsesym) {
            getsym();
            elsecx = cx;
            stmt_sequence(tx, fsys);
            nxtlev[elsesym] = false;
            nxtlev[endsym] = true;
            test(nxtlev, fsys, 6);
            if(sym == endsym) {
                getsym();
                flag = true;
                break;
            }
        }
        code[cx1].a = cx;
        if(sym == endsym && !flag){
            getsym();
        }
    }
    code[cx0].a = elsecx==-1?cx:elsecx;
}

void exp(int *tx, bool* fsys){
    simple_exp(tx, fsys);
    switch (sym) {
        case eql:
            getsym();
            simple_exp(tx, fsys);
            gen(opr, 0, 8);
            break;
        case neq:
            getsym();
            simple_exp(tx, fsys);
            gen(opr, 0, 9);
            break;
        case lss:
            getsym();
            simple_exp(tx, fsys);
            gen(opr, 0, 10);
            break;
        case geq:
            getsym();
            simple_exp(tx, fsys);
            gen(opr, 0, 11);
            break;
        case gtr:
            getsym();
            simple_exp(tx, fsys);
            gen(opr, 0, 12);
            break;
        case leq:
            getsym();
            simple_exp(tx, fsys);
            gen(opr, 0, 13);
            break;
        default:
            return ;
    }
}

void simple_exp(int *tx, bool *fsys){
    enum symbol addop = sym;
    if(addop == minus2) getsym();
    term(tx, fsys);
    if(addop == minus2)
        gen(opr, 0, 1);

    switch (sym) {
        case plus2:
            getsym();
            term(tx, fsys);
            gen(opr, 0, 2);
            break;
        case minus2:
            getsym();
            term(tx, fsys);
            gen(opr, 0, 3);
            break;
        default:
            return;
    }
}

void term(int *tx, bool *fsys){
    factor(tx, fsys);
    switch (sym) {
        case times:
            getsym();
            factor(tx, fsys);
            gen(opr, 0, 4);
            break;
        case slash:
            getsym();
            factor(tx, fsys);
            gen(opr, 0, 5);
            break;
//        case incsym:
//            if(oldsym == ident)
//                isPost = true;
//            getsym();
//            prepos = position(id, *tx);
//            incr(prepos, postpos);
//            break;
//        case decsym:
//            if(oldsym == ident)
//                isPost = true;
//            getsym();
//            prepos = position(id, *tx);
//            decr(prepos, postpos);
//            break;

        default:
            return;
    }
}

void incr(int pre, int post){
    if(isPost){
        gen(lod, 0, table[post].adr);
        gen(lit, 0, 1);
        gen(opr, 0, 2);
        if(post == 0){
            error(101);             //error 101: no such variable
            return ;
        }
        gen(sto, 0, table[post].adr);
        
    } else {
        gen(lit, 0, 1);
        gen(opr, 0, 2);
        if(pre == 0){
            error(101);             //error 101: no such variable
            return ;
        }
        gen(sto, 0, table[pre].adr);
        gen(lod, 0, table[pre].adr);
    }
    isPost = false;
}

void decr(int pre, int post){
    if(isPost){
        gen(lod, 0, table[post].adr);
        gen(lit, 0, 1);
        gen(opr, 0, 3);
        if(post == 0){
            error(101);             //error 101: no such variable
            return ;
        }
        gen(sto, 0, table[post].adr);
        
    } else {
        gen(lit, 0, 1);
        gen(opr, 0, 2);
        if(pre == 0){
            error(101);             //error 101: no such variable
            return ;
        }
        gen(sto, 0, table[pre].adr);
        gen(lod, 0, table[pre].adr);
    }
    isPost = false;

}

void factor(int *tx, bool *fsys){
    int i;
    bool nxtlev[symnum];
    test(facbegsys, fsys, 24);	/* 检测因子的开始符号 */
    while(inset(sym, facbegsys)) 	/* 循环处理因子 */
    {
        if(sym == ident)	/* 因子为常量或变量 */
        {
            i = position(id, *tx);	/* 查找标识符在符号表中的位置 */
            if (i == 0) {
                error(11);	/* 标识符未声明 */
            }
            else {
                if(table[i].kind == constant)
                    gen(lit, 0, table[i].val);
                else
                    gen(lod, 0, table[i].adr);
            }
            getsym();
        }
        else if(sym == incsym || sym == decsym){
            if(oldsym == ident) isPost = true;
            int postpos = position(id, *tx);
            if(sym == incsym){
                getsym();
                if(!isPost && sym != ident) {error(102);break;}
                int prepos = position(id, *tx);
                incr(prepos, postpos);
            } else {
                getsym();
                if(!isPost && sym != ident) {error(102);break;}
                int prepos = position(id, *tx);
                incr(prepos, postpos);
            }
            isPost = false;
        }
        else {
            if(sym == number) {	/* 因子为数 */
                if (num > amax) {
                    error(31); /* 数越界 */
                    num = 0;
                }
                gen(lit, 0, num);
//                if(table[*tx].kind == variable)
//                    gen(sto, 0, table[*tx].adr);
                getsym();
            }
            else if (sym == oddsym) {
                getsym();
                factor(tx, fsys);
                gen(opr, 0, 6);
            }
            else {
                if (sym == lparen) {	/* 因子为表达式 */
                    getsym();
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    nxtlev[rparen] = true;
                    exp(tx, nxtlev);
                    if (sym == rparen) {
                        getsym();
                    }
                    else {
                        error(22);	/* 缺少右括号 */
                    }
                }
            }
        }
        memset(nxtlev, 0, sizeof(bool) * symnum);
        nxtlev[lparen] = true;
        //test(fsys, nxtlev, 23); /* 一个因子处理完毕，遇到的单词应在fsys集合中 */
        /* 如果不是，报错并找到下一个因子的开始，使语法分析可以继续运行下去 */
    }
}


/*
 * 输出所有目标代码
 */
void listall()
{
    int i;
    if (listswitch)
    {
        for (i = 0; i < cx; i++)
        {
            printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
            fprintf(fcode,"%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
        }
    }
}
void interpret(){
    int p = 0; /* 指令指针 */
    int b = 1; /* 指令基址 */
    int t = 0; /* 栈顶指针 */
    struct instruction i;	/* 存放当前指令 */
    int s[stacksize];	/* 栈 */
    
    printf("Start SmallC\n");
    fprintf(fresult,"Start SmallC\n");
    s[0] = 0; /* s[0]不用 */
    s[1] = 0; /* 主程序的三个联系单元均置为0 */
    s[2] = 0;
    s[3] = 0;
    do {
        i = code[p];	/* 读当前指令 */
        p = p + 1;
        switch (i.f)
        {
            case lit:	/* 将常量a的值取到栈顶 */
                t = t + 1;
                s[t] = i.a;
                break;
            case opr:	/* 数学、逻辑运算 */
                switch (i.a)
            {
                case 0:  /* 函数调用结束后返回 */
                    t = b - 1;
                    p = s[t + 3];
                    b = s[t + 2];
                    break;
                case 1: /* 栈顶元素取反 */
                    s[t] = - s[t];
                    break;
                case 2: /* 次栈顶项加上栈顶项，退两个栈元素，相加值进栈 */
                    t = t - 1;
                    s[t] = s[t] + s[t + 1];
                    break;
                case 3:/* 次栈顶项减去栈顶项 */
                    t = t - 1;
                    s[t] = s[t] - s[t + 1];
                    break;
                case 4:/* 次栈顶项乘以栈顶项 */
                    t = t - 1;
                    s[t] = s[t] * s[t + 1];
                    break;
                case 5:/* 次栈顶项除以栈顶项 */
                    t = t - 1;
                    s[t] = s[t] / s[t + 1];
                    break;
                case 6:/* 栈顶元素的奇偶判断 */
                    s[t] = s[t] % 2;
                    break;
                case 8:/* 次栈顶项与栈顶项是否相等 */
                    t = t - 1;
                    s[t] = (s[t] == s[t + 1]);
                    break;
                case 9:/* 次栈顶项与栈顶项是否不等 */
                    t = t - 1;
                    s[t] = (s[t] != s[t + 1]);
                    break;
                case 10:/* 次栈顶项是否小于栈顶项 */
                    t = t - 1;
                    s[t] = (s[t] < s[t + 1]);
                    break;
                case 11:/* 次栈顶项是否大于等于栈顶项 */
                    t = t - 1;
                    s[t] = (s[t] >= s[t + 1]);
                    break;
                case 12:/* 次栈顶项是否大于栈顶项 */
                    t = t - 1;
                    s[t] = (s[t] > s[t + 1]);
                    break;
                case 13: /* 次栈顶项是否小于等于栈顶项 */
                    t = t - 1;
                    s[t] = (s[t] <= s[t + 1]);
                    break;
                case 14:/* 栈顶值输出 */
                    printf("%d", s[t]);
                    fprintf(fresult, "%d", s[t]);
                    t = t - 1;
                    break;
                case 15:/* 输出换行符 */
                    printf("\n");
                    fprintf(fresult,"\n");
                    break;
                case 16:/* 读入一个输入置于栈顶 */
                    t = t + 1;
                    printf(">");
                    fprintf(fresult, ">");
                    scanf("%d", &(s[t]));
                    fprintf(fresult, "%d\n", s[t]);
                    break;
            }
                break;
            case lod:	/* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
                t = t + 1;
                s[t] = s[base(i.l,s,b) + i.a];
                break;
            case sto:	/* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
                s[base(i.l, s, b) + i.a] = s[t];
                t = t - 1;
                break;
            case cal:	/* 调用子过程 */
                s[t + 1] = base(i.l, s, b);	/* 将父过程基地址入栈，即建立静态链 */
                s[t + 2] = b;	/* 将本过程基地址入栈，即建立动态链 */
                s[t + 3] = p;	/* 将当前指令指针入栈，即保存返回地址 */
                b = t + 1;	/* 改变基地址指针值为新过程的基地址 */
                p = i.a;	/* 跳转 */
                break;
            case ini:	/* 在数据栈中为被调用的过程开辟a个单元的数据区 */
                t = t + i.a;
                break;
            case jmp:	/* 直接跳转 */
                p = i.a;
                break;
            case jpc:	/* 条件跳转 */
                if (s[t] == 0)
                    p = i.a;
                t = t - 1;
                break;
        }
    } while (p != 0);
    printf("End SmallC\n");
    fprintf(fresult,"End SmallC\n");
}
/*
 * 初始化变量
 */
void init(){
    for (int i; i<=255; ++i) {
        ssym[i]=nul;
    }
    ssym['+'] = plus2;
    ssym['-'] = minus2;
    ssym['*'] = times;
    ssym['/'] = slash;
    ssym['('] = lparen;
    ssym[')'] = rparen;
    ssym[','] = comma;
    ssym['.'] = period;
    ssym[';'] = semicolon;
    ssym['%'] = mod;
    ssym['$'] = dollar;
    
    /* 设置保留字名字,按照字母顺序，便于二分查找 */
    strcpy(&(word[0][0]), "call");
    strcpy(&(word[1][0]), "const");
    strcpy(&(word[2][0]), "do");
    strcpy(&(word[3][0]), "else");  //
    strcpy(&(word[4][0]), "end");
    strcpy(&(word[5][0]), "for");
    strcpy(&(word[6][0]), "if");
    strcpy(&(word[7][0]), "odd");
    strcpy(&(word[8][0]), "procedure");
    strcpy(&(word[9][0]), "procend");
    strcpy(&(word[10][0]), "read");  //
    strcpy(&(word[11][0]), "repeat");    //
    strcpy(&(word[12][0]), "then");  //
    strcpy(&(word[13][0]), "until");
    strcpy(&(word[14][0]), "while");
    strcpy(&(word[15][0]), "write");    //
    
    /* 设置保留字符号 */
    wsym[0] = callsym;
    wsym[1] = constsym;
    wsym[2] = dosym;
    wsym[3] = elsesym;
    wsym[4] = endsym;
    wsym[5] = forsym;
    wsym[6] = ifsym;
    wsym[7] = oddsym;
    wsym[8] = procsym;
    wsym[9] = procendsym;
    wsym[10] = readsym;
    wsym[11] = repeatsym;
    wsym[12] = thensym;
    wsym[13] = untilsym;
    wsym[14] = whilesym;
    wsym[15] = writesym;
    
    /* 设置指令名称 */
    strcpy(&(mnemonic[lit][0]), "lit");
    strcpy(&(mnemonic[opr][0]), "opr");
    strcpy(&(mnemonic[lod][0]), "lod");
    strcpy(&(mnemonic[sto][0]), "sto");
    strcpy(&(mnemonic[cal][0]), "cal");
    strcpy(&(mnemonic[ini][0]), "int");
    strcpy(&(mnemonic[jmp][0]), "jmp");
    strcpy(&(mnemonic[jpc][0]), "jpc");
    
    /* 设置符号集 */
    for (int i=0; i<symnum; i++)
    {
        declbegsys[i] = false;
        statbegsys[i] = false;
        facbegsys[i] = false;
    }
    
    /* 设置声明开始符号集 */
    declbegsys[constsym] = true;
    declbegsys[procsym] = true;
    
    /* 设置语句开始符号集 */
    statbegsys[callsym] = true;
    statbegsys[ifsym] = true;
    statbegsys[whilesym] = true;
    statbegsys[repeatsym] = true;
    statbegsys[dosym] = true;
    statbegsys[readsym] = true;
    statbegsys[writesym] = true;
    statbegsys[forsym] = true;
    
    /* 设置因子开始符号集 */
    facbegsys[ident] = true;
    facbegsys[number] = true;
    facbegsys[lparen] = true;
    facbegsys[oddsym] = true;
    facbegsys[incsym] = true;
    facbegsys[decsym] = true;
}

void getch() {
    if (cc == ll) { /* 判断缓冲区中是否有字符，若无字符，则读入下一行字符到缓冲区中 */
        if (feof(fin)) {
            if(isEnd){
                printf("Program is incomplete!\n");
                exit(1);
            } else {
                isEnd = true;
            }
        }
        ll = 0;
        cc = 0;
        printf("%d ", cx);
        fprintf(foutput,"%d ", cx);
        ch = ' ';
        while (ch != 10)
        {
            if (EOF == fscanf(fin,"%c", &ch))
            {
                line[ll] = 0;
                break;
            }
            
            printf("%c", ch);
            fprintf(foutput, "%c", ch);
            line[ll] = ch;
            ll++;
        }
    }
    ch = line[cc];
    cc++;
}

void getsym(){
    int i,j,k;
    oldsym = sym;
    while (ch == ' ' || ch == 10 || ch == 9 || ch == 13){	/* 过滤空格、换行和制表符 */
        getch();
    }
    
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) { /* 当前的单词是标识符或是保留字 */
        k = 0;
        do {
            if(k < al){
                a[k] = ch;
                k++;
            }
            getch();
        } while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'));
        a[k] = 0;
        strcpy(id, a);
        i = 0;
        j = nrow - 1;
        do {    /* 搜索当前单词是否为保留字，使用二分法查找 */
            k = (i + j) / 2;
            if (strcmp(id,word[k]) <= 0){
                j = k - 1;
            }
            if (strcmp(id,word[k]) >= 0){
                i = k + 1;
            }
        } while (i <= j);
        if (i-1 > j){ /* 当前的单词是保留字 */
            sym = wsym[k];
        }
        else { /* 当前的单词是标识符 */
            sym = ident;
        }
    }
    else {
        if (ch >= '0' && ch <= '9') { /* 当前的单词是数字 */
            k = 0;
            num = 0;
            sym = number;
            do {
                num = 10 * num + ch - '0';
                k++;
                getch();;
            } while (ch >= '0' && ch <= '9'); /* 获取数字的值 */
            k--;
            if (k > nmax) /* 数字位数太多 */
            {
                error(30);
            }
        }
        else {
            if(ch == '!'){
                getch();
                if(ch =='='){
                    sym = neq;
                    getch();
                } else {
                    sym = nul;
                    error(31);
                }
            }
            else if(ch == '='){
                getch();
                if(ch == '='){
                    sym = eql;
                    getch();
                } else {
                    sym = nul;
                    error(31);
                }
            }
            else if (ch == ':') {	/* 检测赋值符号 */
                getch();
                if (ch == '=') {
                    sym = becomes;
                    getch();
                }
                else {
                    sym = nul;	/* 不能识别的符号 */
                    error(31);
                }
            }
            else if (ch == '+') {
                getch();
                if(ch == '+') {
//                    if(sym != ident)
//                        isPost = false;
                    sym = incsym;
                    getch();
                }
                else {
                    sym = plus2;
                }
                
            }
            else if (ch == '-'){
                getch();
                if(ch == '-'){
                    getch();
                    sym = decsym;
                }
                else {
                    sym = minus2;
                }
            }
            else {
                if (ch == '<') {		/* 检测小于或小于等于符号 */
                    getch();
                    if (ch == '=') {
                        sym = leq;
                        getch();
                    }
                    else {
                        sym = lss;
                    }
                }
                else {
                    if (ch == '>') {		/* 检测大于或大于等于符号 */
                        getch();
                        if (ch == '=') {
                            sym = geq;
                            getch();
                        }
                        else {
                            sym = gtr;
                        }
                    }
                    else {
                        sym = ssym[ch];		/* 当符号不满足上述条件时，全部按照单字符符号处理 */
                        if(sym == semicolon && isEnd){  //程序最后一行不能是分号
                            printf("Program is incomplete!\n");
                            exit(1);
                        }
                        if (sym != period) {
                            getch();
                        }
                        
                    }
                }
            }
        }
    }
}



void error(int n)
{
    char space[81];
    memset(space,32,81);
    
    space[cc-1]=0; /* 出错时当前符号已经读完，所以cc-1 */
    
    printf("**%s^%d\n", space, n);
    fprintf(foutput,"**%s^%d\n", space, n);
    
    err = err + 1;
    if (err > maxerr)
    {
        exit(1);
    }
}

int addset(bool* sr, bool* s1, bool* s2, int n) {
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]||s2[i];
    }
    return 0;
}

/*
 * 生成虚拟机代码
 *
 * x: instruction.f;
 * y: instruction.l;
 * z: instruction.a;
 */
void gen(enum fct x, int y, int z ){
    if (cx >= cxmax){
        printf("Program is too long!\n");	/* 生成的虚拟机代码程序过长 */
        exit(1);
    }
    if (z >= amax){
        printf("Displacement address is too big!\n");	/* 地址偏移越界 */
        exit(1);
    }
    code[cx].f = x;
    code[cx].l = y;
    code[cx].a = z;
    cx++;
}

/*
 * 查找标识符在符号表中的位置，从tx开始倒序查找标识符
 * 找到则返回在符号表中的位置，否则返回0
 *
 * id:    要查找的名字
 * tx:    当前符号表尾指针
 */
int position(char* id, int tx){
    int i;
    strcpy(table[0].name, id);
    i = tx;
    while (strcmp(table[i].name, id) != 0){
        i--;
    }
    return i;
}

/*
 * 测试当前符号是否合法
 *
 * 在语法分析程序的入口和出口处调用测试函数test，
 * 检查当前单词进入和退出该语法单位的合法性
 *
 * s1:	需要的单词集合
 * s2:	如果不是需要的单词，在某一出错状态时，
 *      可恢复语法分析继续正常工作的补充单词符号集合
 * n:  	错误号
 */
void test(bool* s1, bool* s2, int n)
{
    if (!inset(sym, s1))
    {
        error(n);
        /* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
        while ((!inset(sym,s1)) && (!inset(sym,s2)))
        {
            getsym();
        }
    }
}

/*
 * 用数组实现集合的集合运算
 */
int inset(int e, bool* s)
{
    return s[e];
}

/* 通过过程基址求上l层过程的基址 */
int base(int l, int* s, int b)
{
    int b1;
    b1 = b;
    while (l > 0)
    {
        b1 = s[b1];
        l--;
    }
    return b1;
}

/*
 * 在符号表中加入一项
 *
 * k:       标示符的种类
 * ptx:     符号表的尾指针
 * pdx:     dx为当前应分配变量的地址，分配后加1
 *
 */

void enter(enum object k, int *ptx, int *pdx){
    int i;
    i = position(id, *ptx);
    if(i == 0){
        (*ptx)++;
        switch(k){
            case constant:
                table[(*ptx)].val = num;
                break;
            case variable:
                table[(*ptx)].adr = (*pdx);
                (*pdx)++;
                break;
            case procedure:
                break;
        }
        strcpy(table[(*ptx)].name, id);
        table[(*ptx)].kind = k;
    } else {
        table[i].kind = k;
    }
}














