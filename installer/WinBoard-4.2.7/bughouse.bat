@echo off
REM  Usage: bughouse -icshost chessclub.com -icshelper timestamp
REM  or:    bughouse -icshost freechess.org -icshelper timeseal
REM
REM  This batch file starts up two copies of WinBoard for use in playing
REM  bughouse.  Each one has its own settings file and ICS logon file.
REM  Arrange the windows on the screen to your taste, then do Save
REM  Settings Now in both windows.  You can create the logon files
REM  bh1ics.ini and bh2ics.ini if you like; have one of them log in
REM  as you and the other as a guest.
REM
winboard -ics -x 0 -y 0 -icsX 30 -icsY 30 -settingsFile bh1stg.ini -icslogon bh1ics.ini %1 %2 %3 %4 %5 %6 %7 %8 %9
winboard -ics -x 120 -y 120 -icsX 150 -icsY 150 -settingsFile bh2stg.ini -icslogon bh2ics.ini %1 %2 %3 %4 %5 %6 %7 %8 %9

