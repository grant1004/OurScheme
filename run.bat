@echo off
start cmd /k "cd /d E:\AllRepo\OurScheme\OurScheme && g++ -std=c++11 2023.3.28_new.cpp -o project2.exe && project2.exe"
:: /k not close cmd 
:: /c close cmd 