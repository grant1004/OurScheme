@echo off
start cmd /k "cd /d E:\AllRepo\OurScheme\OurScheme && g++ -std=gnu++98 2023.3.30.cpp -o project2.exe && project2.exe < test.txt"
:: /k not close cmd 
:: /c close cmd 