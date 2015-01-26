char *a[]={
	"\n6 Geese a Lay›gŒ",
	" day of Christmas\nmy˜rue l¡ s‘t˜o me:",
	"\n2 Turtle D¡s\nand aˆ",
	"\n9 Ladies Dancœ‰",
	"\n",
	"\n4 Call B ds",
	"\n10 LorLeapœƒ",
	"\n7 Swan”Swimm€",
	"—artridge in a—ear Tree\n",
	"\n8 MaiMilkœ‡",
	"\n11’š’†",
	"th",
	"\n5 Gold‘ Rs…",
	"\n3 Fr™ch H™s‚",
	"On˜he ",
	" Drumm",
	"d”",
	"™",
	"—ip",
	"ev‘‹",
	"s a ",
	"„s",
	"„t",
	" P",
	" t",
	"en",
	"ers",
	"in",
	"",
	"Ÿ",
	"Ÿ",
	"ing",
	"ir",
	"ove"
};
char*b="f st\nAˆ•econd‚–h "
	"d„four‹…„fif‹Œ•i"
	"x‹€•“‡„eigh‹‰„n›"
	"‹ƒ–‘‹†„el“Š–welf"
	"‹\n12š›gŠ";

pr(char*s)
{
	for(;*s;s++)
		if(*s&128)
			pr(a[*s&127]);
		else putchar(*s);
}
main()
{
	pr(b);
}	
