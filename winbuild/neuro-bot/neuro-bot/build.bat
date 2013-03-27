cd ..\..\curl-7.29.0\winbuild
nmake /f Makefile.vc mode=dll
copy ..\builds\libcurl-vc-x86-release-dll-ipv6-sspi-spnego-winssl\lib\* ..
