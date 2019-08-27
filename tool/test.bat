@echo off

if "%1"=="login" (curl -v -H "Content-Type: application/json" -d "{\"id\":\"G0000\",\"password\":\"saic\"}" localhost/login)

if "%1"=="register" (curl -v -H "Connection: close" -d "{\"id\":\"G0001\",\"password\":\"asdf\",\"role\":\"te\",\"contact\":\"123456\",\"name\":\"hello\"}" localhost/register)

if "%1"=="data" (curl -v -H "Content-Type: application/json" -d "{\"database\":\"labwireless\",\"statement\":\"select * from `0用户信息`\",\"sessionId\":\"866c3c70-8c21-4cef-8fe2-97c81f0b9305\"}" localhost/sqlitedata)

if "%1"=="dataro" curl -v -H "Content-Type: application/json;charset=utf-8" -H "Connection: close" -d "{\"database\":\"labwireless\",\"readonly\":\"0\",\"statement\":\"select * from `0用户信息`\"}" localhost/sqlitedata

if "%1"=="img" (curl -v -H "Content-Type: image/jpeg" -H "Connection: close" -d "{\"database\":\"labwireless\",\"statement\":\"select * from `0用户信息`\",\"readonly\":0}" localhost/upload)

if "%1"=="imgx" (curl -v -H "Expect:" -H "Content-Type: image/jpeg" -H "Connection: close" -F "image=@C:\Users\svandex\Desktop\TVNET\img\windows.jpg" localhost/upload)
