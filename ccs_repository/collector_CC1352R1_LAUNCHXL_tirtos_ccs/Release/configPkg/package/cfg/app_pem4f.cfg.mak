# invoke SourceDir generated makefile for app.pem4f
app.pem4f: .libraries,app.pem4f
.libraries,app.pem4f: package/cfg/app_pem4f.xdl
	$(MAKE) -f C:\Users\uesatj1\Documents\GitHub\lab_submissions\ccs_repository\collector_CC1352R1_LAUNCHXL_tirtos_ccs/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\uesatj1\Documents\GitHub\lab_submissions\ccs_repository\collector_CC1352R1_LAUNCHXL_tirtos_ccs/src/makefile.libs clean

