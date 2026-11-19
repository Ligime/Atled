all:
	clang -o app.exe -g ./src/*.c ./lib/libraylibdll.a -I./include -fsanitize=address && .\app.exe
