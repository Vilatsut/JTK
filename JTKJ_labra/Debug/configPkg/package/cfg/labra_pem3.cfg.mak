# invoke SourceDir generated makefile for labra.pem3
labra.pem3: .libraries,labra.pem3
.libraries,labra.pem3: package/cfg/labra_pem3.xdl
	$(MAKE) -f C:\ti\JTKJ_labra_2020/src/makefile.libs

clean::
	$(MAKE) -f C:\ti\JTKJ_labra_2020/src/makefile.libs clean

