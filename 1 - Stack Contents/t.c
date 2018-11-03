#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


int *FP;
typedef unsigned int u32;
char *ctable = "0123456789ABCDEF";
int  BASE = 10; 

int myprintf(char *fmt, ...);

int main(int argc, char *argv[ ], char *env[ ])
{
	int a,b,c;
	printf("enter main\n");
	printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
	printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);

	printf("\n(Part 1.1). Write C code to print values of argc and argv[] entries\n");

	printf("   Argc value: %i", argc);
	printf("\n   Argv Values: ");
	for(int i = 1; i < argc; i++)
	{
		printf("%s, ", argv[i]);
	}

	printf("\n\n(Part 2 & 3). Use your myprintf() to print argc, argv and env values\n");

myprintf("\n testing: cha=%c string=%s      dec=%d hex=%x oct=%o neg=%d\n", 
	       'A', "this is a test", 100,    100,   100,  -100);

myprintf("\n   Argc Value: %d", argc);

printf("\n   Argv Values:\n");

for(int i = 1; i < argc; i++)
{
	myprintf("   Argv[%d]: %s\n", i, argv[i]);
}

for (int i = 0; env[i]; i++)
{
	myprintf("   Env[%d]: %s\n", i, env[i]);
}
	printf("\n");
	a=1; b=2; c=3;
	A(a,b);

	printf("exit main\n");
}




int rpu(u32 x)
{  
	char c;
	if (x)
	{
		BASE = 10; 
		c = ctable[x % BASE];
		rpu(x / BASE);
		putchar(c);
	}
}

int printu(u32 x)
{
	BASE = 10; 
	(x==0)? putchar('0') : rpu(x);
	putchar(' ');
}

int prints(char *s)
{
	char *c = s;
	while(*c)
	{
		putchar(*c++);;
	}
}

//2-2. Write YOUR ONW fucntions 

 //int  printd(int x) which prints an integer (x may be negative!!!)
 //int  printx(u32 x) which prints x in HEX   (start with 0x )
 //int  printo(u32 x) which prints x in Octal (start with 0  )

int printd(int x)
{
	if(x == 0)
	{
		putchar('0');
	}
	else if(x<0)
	{
		BASE = 10;
		putchar('-');
		rpu(-x);
	}
	else
	{
		BASE = 10;
		rpu(x);
	}
}

int printx(u32 x)
{
	putchar('0');
	putchar('x');
	if(x == 0)
	{
		putchar('0');
	}
	else
	{
		BASE = 16;
		rpu(x);
	}
}
int printo(u32 x)
{
	putchar('0');
	if(x!=0)
	{
		BASE = 8;
		rpu(x);
	}
}



int myprintf(char *fmt, ...)
{
	char *cp = fmt;
	int *ip = &fmt + 1;

	while(*cp)
	{
		if(*cp != '%')
		{
			putchar(*cp);
			if(*cp == '\n')
			{
				putchar('\r');
			}
			cp++; 
			continue;

		}
		cp++;

		switch (*cp)
		{
			case 'c':putchar(*ip); break;
			case 's':prints((char*)(*ip)); break;
			case 'u':printu(*ip); break;
			case 'd':printd(*ip); break;
			case 'o':printo(*ip); break;
			case 'x':printx(*ip); break;
		}
		ip++;
		cp++;		
	}
}


int A(int x, int y)
{
	int d,e,f;
	printf("  enter A\n");

//PRINT ADDRESS OF d, e, f
	printf("   Address of d: %x\n", &d);
	printf("   Address of e: %x\n", &e);
	printf("   Address of f: %x\n", &f);

	d=4; e=5; f=6;
	B(d,e);
	printf(" exit A\n");
}

int B(int x, int y)
{
	int g,h,i;
	printf("  enter B\n");

// PRINT ADDRESS OF g,h,i
	printf("   Address of g: %x\n", &g);
	printf("   Address of h: %x\n", &h);
	printf("   Address of i: %x\n", &i);

	g=7; h=8; i=9;
	C(g,h);
	printf(" exit B\n");
}

int C(int x, int y)
{
	int u, v, w, i, *p;

	printf("  enter C\n");
  // PRINT ADDRESS OF u,v,w,i,p;
	printf("   Address of u: %x\n", &u);
	printf("   Address of v: %x\n", &v);
	printf("   Address of w: %x\n", &w);
	printf("   Address of i: %x\n", &i);
	printf("   Address of p: %x\n", p);

	u=10; v=11; w=12; i=13;

	FP = (int *)getebp();

	printf("\n(Part 1.2). Write C code to print the stack frame link list\n   ");

	p = FP;

	while(p != 0)
	{
		printf("%x -> ", p);
		p = *p;
	}
	printf("%x\n", p);

	printf("\n(Part 1.3). Print the stack contents from p to the frame of main()\n");
	
	FP -= 22;
	p = FP;

	
	int count = 1;
	i = 0;

	while(i < 128)
	{
		printf("   Entry: %i |", count);
		count++;

		printf(" Stack Address: %p  |", FP);
		printf(" Stack Value: %x\n", *FP);
		FP++;
		i++;
	}

	
}




