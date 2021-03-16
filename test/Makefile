# Test makefile

all: test

ALLTESTS=test.relative \
         test.absolute \
         test.analyze \
         test.archive \
         test.md.single \
         test.md.rst \
         test.md.multi.rst \
         test.umbrella \
         test.qsub.mremd \
         test.override \
         test.hremd \
         test.additionalnamelist

test.relative:
	@-cd Test_MREMD_Relative && ./RunTest.sh $(OPT)

test.absolute:
	@-cd Test_MREMD_Absolute && ./RunTest.sh $(OPT)

test.analyze:
	@-cd Test_Analyze && ./RunTest.sh $(OPT)

test.archive:
	@-cd Test_Archive && ./RunTest.sh $(OPT) 

test.md.single:
	@-cd Test_MD_Single && ./RunTest.sh $(OPT)

test.md.rst: 
	@-cd Test_MD_RST && ./RunTest.sh $(OPT)

test.md.multi.rst:
	@-cd Test_MD_Multi_RST && ./RunTest.sh $(OPT)

test.umbrella:
	@-cd Test_Umbrella && ./RunTest.sh $(OPT)

test.qsub.mremd:
	@-cd Test_Qsub_MREMD && ./RunTest.sh $(OPT)

test.override:
	@-cd Test_Override_Irest && ./RunTest.sh $(OPT)

test.hremd:
	@-cd Test_HREMD && ./RunTest.sh $(OPT)

test.additionalnamelist:
	@-cd Test_MD_AdditionalNamelist && ./RunTest.sh $(OPT)

test: $(ALLTESTS)

test.vg:
	$(MAKE) test OPT="$(OPT) vg"

clean:
	@echo "Cleaning all test directories:"
	(find . -name 'test.out' -o -name 'valgrind.out' -o -name 'Thread.*' \
        -o -name 'Test_Results.dat' -o -name 'Test_Error.dat' | \
        while read dif ;\
        do \
            rm -f $$dif ;\
        done ;\
        )
	$(MAKE) test OPT=clean
#	/bin/rm -rf run.000 run.001 Analyze.0.0 Archive.0.0 output.txt ConstF.rst*
