@echo off
cd libdb
make %1
cd ..
cd libregex
make %1
cd ..
cd libutil
make %1
cd ..
cd btreeop
make %1
cd ..
cd global
make %1
cd ..
cd gtags
make %1
cd ..
cd gctags
make %1
cd ..
cd htags
make %1
cd ..