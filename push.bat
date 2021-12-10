@echo off
set project=sim

git add *.cpp *.h 
git add arduino\*

git add %project%.sln
git add %project%.vcxproj
git add %project%.vcxproj.filters

git add README.md
git add push.bat

git commit -m "See History.h for changes"
git push origin main

