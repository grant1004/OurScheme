@echo off
start cmd /k "cd /d E:\AllRepo\OurScheme\OurScheme && g++ -std=c++11 2023.3.29.cpp -o project2.exe && project2.exe"
:: /k not close cmd 
:: /c close cmd 