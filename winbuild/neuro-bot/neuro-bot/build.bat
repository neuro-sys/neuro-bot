cd ..\..\curl-7.29.0\winbuild
nmake /f Makefile.vc mode=dll VC=10
copy ..\builds\libcurl-vc-x86-release-dll-ipv6-sspi-spnego-winssl\lib\* ..
