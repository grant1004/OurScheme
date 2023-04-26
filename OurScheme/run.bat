@echo off
start cmd /k "cd /d E:\AllRepo\OurScheme\OurScheme && g++ -std=gnu++98 2023.4.25.cpp -o project3.exe && project3.exe "
:: /k not close cmd 
:: /c close cmd 