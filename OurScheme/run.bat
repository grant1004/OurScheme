@echo off
:: TASKKILL /F /IM project3.exe
:: TASKKILL /F /IM cmd.exe
start cmd /k " cd /d E:\AllRepo\OurScheme\OurScheme && g++ -std=gnu++98 2023.5.2.cpp -o project3.exe && time /t && project3.exe < tt.txt > ME.txt && time /t "
:: /k not close cmd 
:: /c close cmd 