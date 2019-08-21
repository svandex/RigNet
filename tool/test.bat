@echo off

if "%1"=="register" (curl -v -H "Connection: close" -d "{\"id\":\"G0001\",\"password\":\"asdf\",\"role\":\"te\",\"contact\":\"123456\",\"name\":\"hello\"}" localhost/register)

if "%1"=="data" (curl -v -H "Content-Type: application/json" -d "{\"database\":\"labwireless\",\"statement\":\"select * from `0用户信息`\",\"sessionId\":\"866c3c70-8c21-4cef-8fe2-97c81f0b9305\"}" localhost/sqlitedata)

if "%1"=="dataro" (curl -v -H "Content-Type: application/json" -H "Connection: close" -d "{\"database\":\"labwireless\",\"statement\":\"select * from `0用户信息`\",\"readonly\":0}" localhost/sqlitedata)