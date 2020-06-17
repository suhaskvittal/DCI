@ECHO OFF &SETLOCAL

FOR %%F IN (src/*.cpp) DO (
	echo bin/%%~nF.o
	g++ -std=gnu++11 -w -c src/%%F -o bin/%%~nF.o -lws2_32
)

SET OBJ=
FOR %%F IN (bin/*.o) DO CALL SET "OBJ=%%OBJ%% bin/%%F"
g++ -o main.exe %OBJ% -lws2_32