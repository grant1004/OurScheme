@echo off
:: TASKKILL /F /IM project3.exe
:: TASKKILL /F /IM cmd.exe
start cmd /k " cd /d E:\AllRepo\OurScheme\OurScheme && g++ -std=gnu++98 2023.4.26.cpp -o project3.exe && project3.exe < tt.txt > ME.txt "
:: /k not close cmd 
:: /c close cmd 