# Test makefile

all: test

test: test.relative clean test.absolute test.analyze test.archive

test.relative: mremd.opts ../../CreateRemdDirs
	$(VALGRIND) ../../CreateRemdDirs -i mremd.opts -b 0 -e 0 -c ../CRD
	@echo "TEST DIFFS: " 
	@diff mremd.dim.save run.000/remd.dim | wc -l
	@diff relative.groupfile.save run.000/groupfile | wc -l
	@echo "----------------------------------------"
	@echo ""

test.absolute: mremd.opts ../../CreateRemdDirs
	$(VALGRIND) ../../CreateRemdDirs -i mremd.opts -b 0 -e 0 -c ~/bin/createRemdDirs/test/CRD
	@echo "TEST DIFFS: "
	@diff mremd.dim.save run.000/remd.dim | wc -l
	@diff absolute.groupfile.save run.000/groupfile | wc -l
	@echo "----------------------------------------"
	@echo ""

test.analyze: mremd.opts ../../CreateRemdDirs run.000/TRAJ
	touch run.000/TRAJ/rem.crd.001
	$(VALGRIND) ../../CreateRemdDirs -i mremd.opts -b 0 -e 0 --analyze
	@echo -n "TEST DIFF: "
	@diff batch.cpptraj.in.save Analyze.0.0/batch.cpptraj.in | wc -l
	@echo "----------------------------------------"
	@echo ""

test.archive: mremd.opts ../../CreateRemdDirs run.000/TRAJ
	touch run.000/TRAJ/rem.crd.001
	$(VALGRIND) ../../CreateRemdDirs -i mremd.opts -b 0 -e 0 --archive
	@echo "TEST DIFFS: "
	@diff ar1.0.cpptraj.in.save Archive.0.0/ar1.0.cpptraj.in | wc -l
	@diff ar2.0.cpptraj.in.save Archive.0.0/ar2.0.cpptraj.in | wc -l
	@echo "----------------------------------------"
	@echo ""

test.vg:
	$(MAKE) test VALGRIND="valgrind --tool=memcheck --leak-check=yes --show-reachable=yes"

clean:
	/bin/rm -rf run.000 run.001 Analyze.0.0 Archive.0.0
