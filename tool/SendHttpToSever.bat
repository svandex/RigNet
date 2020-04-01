@echo off

if "%1"=="login" (curl -v -H "Content-Type: application/json" -d "{\"id\":\"G0000\",\"password\":\"saic\"}" localhost/login)

if "%1"=="exist" (curl -v -H "Content-Type: application/json" -d "{\"id\":\"G0003\",\"password\":\"asdf\"}" localhost/exist)

if "%1"=="regst" (curl -v -H "Connection: close" -d "{\"id\":\"G0015\",\"password\":\"asdf\",\"role\":\"te\",\"contact\":\"123456\",\"name\":\"hello\"}" localhost/regst)

if "%1"=="data" (curl -v -H "Content-Type: application/json" -d "{\"database\":\"management\",\"statement\":\"select * from Test\",\"sessionid\":\"9e2f6b7d-07d5-4c2c-b21e-0146f9c1d306\"}" localhost/sqlite)

if "%1"=="dataro" curl -v -H "Content-Type: application/json;charset=utf-8" -H "Connection: close" -d "{\"database\":\"labwireless\",\"readonly\":\"0\",\"statement\":\"select * from `0用户信息`\"}" localhost/sqlitedata

if "%1"=="img" (curl -v -H "Content-Type: image/jpeg" -H "Connection: close" -d "{\"database\":\"labwireless\",\"statement\":\"select * from `0用户信息`\",\"readonly\":0}" localhost/upload)

if "%1"=="imgx" (curl -v -H "Expect:" -H "Content-Type: image/jpeg" -H "Connection: close" -F "image=@C:\Users\svandex\Desktop\TVNET\img\windows.jpg" localhost/upload)
