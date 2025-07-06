@echo off
echo AgonLight/Console8 build ....
copy %1.agd ".\Suite Agon\AGDsource"
cd "Suite Agon"
call build %~n1
cd ..
