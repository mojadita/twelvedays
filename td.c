#include<string.h>
char*a="On the first$\nA-\n#second$%\n#third$_\n#fourth$?\n#fifth$;\n#sixth$@\n#seventh$!#eighth$/#ninth$&#tenth$+#eleventh$=#twelfth$\n12 Drummers Drumm*=",b[]="@$%&#?!+-*/=_;",*t[]={"\n6 Geese a Lay*;"," day of Christmas\nmy true love sent to me:","\n2 Turtle Doves\nand a-","\n9 Ladies Danc*/","\nOn the ","\n4 Call* Birds_","\n7 Swans a Swimm*@\n","\n10 Lords a Leap*&"," Partridge in a Pear Tree","ing","\n8 Maids a Milk*!","\n11 Pipers Pip*+","\n3 French Hens%","\n5 Golden R*s?"};pr(char *s){for(;*s;s++){char*p=strchr(b,*s);if(p)pr(t[p-b]);else putchar(*s);}}main(){pr(a);}   
